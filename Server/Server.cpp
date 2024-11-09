#include "Server.hpp"

// ----------------------------------- Parametric Constructor --------------------------------------------
Server::Server(const ServerConfig& serverConfig)
    : MotherSocket(AF_INET, SOCK_STREAM, 0, serverConfig.ports, serverConfig.interface)
{
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


void Server::handleClient(ClientInfo* client)
{
    try {
        Client clientHandler(client);
        
        analyzeBasicHeaders(clientHandler.getRequest(), clientHandler.getResponse(), client);
        router.route(clientHandler.getRequest(), clientHandler.getResponse());
        sendResponse(client->pfd.fd, clientHandler.getResponse()->toString());
        clientHandler.setLastActivity();
        if (clientHandler.shouldKeepAlive() == false)
            removeClient(client);
    }
    catch (const std::exception& e) {
        removeClient(client);
        LOG(e.what());
    }
}


bool    Server::IsTimeout(ClientInfo* client)
{
    time_t currentTime = time(NULL);
    time_t diff = currentTime - client->lastActivity;
    
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
