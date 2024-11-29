#include "Server.hpp"

// ----------------------------------- Parametric Constructor --------------------------------------------
Server::Server(const ServerConfig& serverConfig)
    : MotherSocket(AF_INET, SOCK_STREAM, 0, serverConfig.ports, serverConfig.interface)
{
    ConfigParser::setDefaultErrorPages(config);
    ConfigParser::copyServerConfig(serverConfig, config);
}

// -------------------------------- Destructor -------------------------------------------------------
Server::~Server()
{
    //std::cout << "Number of clients"  << clients.size() << std::endl;
    LOG_INFO("Destructor of Server Called");
    int i = 0;
    while (i < clients.size())
    {
        LOG_INFO("Deleting one client");
        delete clients[i];
        i++;
    }
    clients.clear();
    i = 0;
    while (i < cgis.size())
    {
        LOG_INFO("Deleting one CGI");
        delete cgis[i];
        i++;
    }
    cgis.clear();
}

// --------------------------------- CORE FUNCTIONALITIES ------------------------------------------

void    Server::init()
{
    setNonBlocking();

    // Iteramos en el map de endpoints de struct serverConfig

    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = config.locations.begin(); it != config.locations.end(); ++it)
        router.loadEndpoints(it->first, it->second);

    // Creamos el primer cliente a polear (poll()) que seria nuestro socket passivo.
    toPassiveSocket(10);

    std::ostringstream info_message;
    info_message << "Server is listening on fd (each one corresponding to a port): ";
    for (std::vector<int>::const_iterator it = socketFds.begin(); it != socketFds.end(); it++)
    {
        info_message << *it << " ";
    }
    LOG_INFO(info_message.str());
}


void Server::acceptClient(int listenFd)
{
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ServerError error("Accept incoming socket failed !");
            LOG_EXCEPTION(error);
        }
        return;
    }

    ClientInfo* newClient = new ClientInfo();
    newClient->pfd.fd = clientFd;
    newClient->pfd.events = POLLIN;
    newClient->pfd.revents = 0;
    newClient->lastActivity = time(NULL);
    newClient->keepAlive = true;
    newClient->timeout = 60;
    newClient->client_max_body_size = config.client_max_body_size;
    clients.push_back(newClient);

    std::ostringstream info_message;
    info_message << "New Client on Fd: " << clientFd;
    LOG_INFO(info_message.str());
}


void Server::sendResponse(int clientSocket, const std::string& response)
{
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), MSG_CONFIRM);
    if (bytesSent < 0)
    {
        ServerError error("Failed to send response !");
        LOG_EXCEPTION(error);
    }
}

void Server::removeClient(ClientInfo* client)
{
    close(client->pfd.fd);
    std::vector<ClientInfo*>::iterator it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end())
        delete (*it);
        clients.erase(it);
}

void    Server::handleCGIresponse(CgiProcess* cgi)
{
    if (cgi->output_pipe_fd.fd < 0) {
        LOG_INFO("Invalid pipe file descriptor");
        return;
    }
    //std::cout << "Attempting to read from pipe fd: " << cgi->output_pipe_fd.fd << std::endl;
    std::string result;
    char buffer[4096];
    ssize_t bytesRead;
    bool hasData = false;

    // Comprobar validez del FD
    if (fcntl(cgi->output_pipe_fd.fd, F_GETFL) == -1) {
        LOG_INFO("Pipe fd is invalid or closed");
        return;
    }

    while (true) {
        bytesRead = read(cgi->output_pipe_fd.fd, buffer, sizeof(buffer));
        
        if (bytesRead > 0) {
            result.append(buffer, bytesRead);
            hasData = true;
            std::cout << "Read " << bytesRead << " bytes from CGI pipe" << std::endl;
            continue;
        }
        
        if (bytesRead == 0) {
            // End of file
            LOG_INFO("Reached end of CGI pipe");
            break;
        }
        
        if (bytesRead < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_INFO("No more data available right now");
                if (!hasData) {
                    return; // esperar hasta proximo loop
                }
                break;
            } else {
                // Real error
                LOG_INFO("Error reading from CGI pipe: " + std::string(strerror(errno)));
                break;
            }
        }
    }

    // If we got any data, send it to the client
    if (hasData) {
        LOG_INFO("CGI output received " + result);
        Response response;
        response.setStatusCode(200);
        response.setBody(result);
        response.setContentType("text/html");
        sendResponse(cgi->client_fd, response.toString());
    } else {
        LOG_INFO("No CGI output received");
        Response response;
        response.setStatusCode(500);
        response.setBody("CGI execution failed - No output received");
        response.setContentType("text/plain");
        sendResponse(cgi->client_fd, response.toString());
    }
}

void Server::handleClient(ClientInfo* client) {
    try {
        Client clientHandler(client);

        if (clientHandler.HandleConnection() == false) {
            setErrorPageFromStatusCode(clientHandler.getResponse());
            sendResponse(client->pfd.fd, clientHandler.getResponse()->toString());
            removeClient(client);
        }
        else
        {
            analyzeBasicHeaders(clientHandler.getRequest(), clientHandler.getResponse(), client);
            router.route(clientHandler.getRequest(), clientHandler.getResponse());
            if (IsCgiRequest(clientHandler.getResponse()) == false)
            {
                setErrorPageFromStatusCode(clientHandler.getResponse());
                clientHandler.getResponse()->setHeader("Server", this->config.server_name);
                //clientHandler.getResponse()->setHeader("Date", this->config.server_name); hacer funcion para el tiempo
                sendResponse(client->pfd.fd, clientHandler.getResponse()->toString());
            }

            if (!clientHandler.shouldKeepAlive())
                removeClient(client);
            else
                clientHandler.setLastActivity();
        }
    } catch (const std::exception& e) {
        LOG("Error handling client: " + std::string(e.what()));
        Response errorResponse;
        errorResponse.setStatusCode(500);
        setErrorPageFromStatusCode(&errorResponse);
        sendResponse(client->pfd.fd, errorResponse.toString());
        removeClient(client);
    }
}



bool    Server::IsTimeout(ClientInfo* client)
{
    time_t currentTime = time(NULL);
    time_t diff = currentTime - client->lastActivity;
    
    // Don't timeout clients with active CGI processes
    for (std::vector<CgiProcess*>::const_iterator it = cgis.begin(); it != cgis.end(); ++it) {
        if ((*it)->client_fd == client->pfd.fd) {
            return false;  // Client has an active CGI process
        }
    }
    
    if (!client->keepAlive || diff > CONNECTION_TIMEOUT)
    {
        std::string info_message("Client Timeouted: ");
        std::ostringstream      time_output;
        time_output << diff;
        info_message += time_output.str();
        info_message += " seconds elapsed\n";
        LOG_INFO(info_message.c_str());
        return true;
    }
    return false;
}

bool    Server::IsTimeoutCGI(CgiProcess* cgi)
{
    time_t currentTime = time(NULL);
    time_t diff = currentTime - cgi->start_time;
    
    if (diff > CGI_TIMEOUT)
    {
        std::ostringstream info_message;
        info_message << "CGI Timeouted after " << diff << " seconds for client " << cgi->client_fd;
        LOG_INFO(info_message.str());
        
        // Send timeout response to client
        Response timeoutResponse;
        timeoutResponse.setStatusCode(504);  // Gateway Timeout
        timeoutResponse.setBody("CGI process timed out");
        timeoutResponse.setContentType("text/plain");
        sendResponse(cgi->client_fd, timeoutResponse.toString());
        
        return true;
    }
    return false;
}

bool        Server::IsCgiRequest(Response* res)
{
    if (res->getStatusCode() == 103 && res->getCgiProcess() != NULL)
    {
        cgis.push_back(res->getCgiProcess()); 
        std::cout << "New Cgi appended to server" << std::endl;
        return true;
    }
    return false;
}


void        Server::setErrorPageFromStatusCode(Response*    response)
{
    int errorCode = response->getStatusCode();
    std::string filepath = this->config.error_pages.at(errorCode);
    //std::cout << "From errorCode page is " << filepath << std::endl;
  
    LOG_INFO(filepath);
    //std::cout << "filepath << std::endl;
    if (filepath.empty())
        return ;
    std::ifstream file(filepath.c_str());
    if (!file.is_open())
        return ;
    std::stringstream buffer;
    buffer << file.rdbuf();
    //std::cout << buffer.str() << std::endl;
    response->setBody(buffer.str());

}



//-------------------------------- EXCEPTION HANDLING ------------------------------------


Server::ServerError::ServerError(const std::string& error): error_string(error)
{
}

Server::ServerError::~ServerError() throw()
{
}

const char * Server::ServerError::what() const throw()
{
	return error_string.c_str();
}


//-------------------------------- HEADERS HANDLING ------------------------------------

// HOST, KEEP-ALIVE, CONTENT-LENGHT
void     Server::analyzeBasicHeaders(const Request* request, Response* response, ClientInfo*    client)
{   

    // HOST 
    //std::cout << "\n\t\tHeader Basic Analyzer" << std::endl;
    std::string host(request->getHeader("Host"));

    if (host.empty() || isValidHostHeader(host) == false)
        return ;
    // else{ add logic to only accept to our own servers }
    //std::cout << "Valid Request received. host: " << host << std::endl;


    // CONNECTION
    std::string connectionHeader = request->getHeader("Connection");
    if (connectionHeader == "keep-alive")
    {
        client->keepAlive = true;
        response->setHeader("Connection", "Keep-alive");
        // KEEP-ALIVE
        std::string isthere_keepalive_spec(request->getHeader("Keep-alive"));

        if (!isthere_keepalive_spec.empty())
        {
            // search for timeout 
            size_t timeoutPos = isthere_keepalive_spec.find("timeout=");
            std::string header_response_value;

            if (timeoutPos != std::string::npos)
            {
                size_t valueStart = timeoutPos + strlen("timeout=");
                size_t valueEnd = isthere_keepalive_spec.find_first_of(" ,", valueStart);
                std::string timeoutValue = isthere_keepalive_spec.substr(valueStart, valueEnd - valueStart);
                client->timeout = std::atoi(timeoutValue.c_str());
                header_response_value += std::string(" timeout=");
                header_response_value += timeoutValue;
                response->setHeader("Keep-alive", header_response_value);
            }
            // search for max
            size_t maxPos = isthere_keepalive_spec.find("max=");
            if (maxPos != std::string::npos)
            {
                size_t valueStart = maxPos + strlen("max=");
                size_t valueEnd = isthere_keepalive_spec.find_first_of(" ,", valueStart);
                std::string maxValue = isthere_keepalive_spec.substr(valueStart, valueEnd - valueStart);
                client->max = std::atoi(maxValue.c_str());
                header_response_value += std::string(" max=");
                header_response_value += maxValue;
                response->setHeader("Keep-alive", header_response_value);
            }
        }
    }
    std::string contentLength = request->getHeader("Content-length");
    if (!contentLength.empty())
        config.locations[request->getUri()].client_max_body_size = ConfigParser::parseSize(contentLength);
}
