#include "Server.hpp"

// ----------------------------------- Parametric Constructor --------------------------------------------
Server::Server(const ServerConfig& serverConfig)
    : MotherSocket(AF_INET, SOCK_STREAM, 0, serverConfig.port, serverConfig.host)
{
    std::cout << "<Server>:\n\t- " << serverConfig.host << "\n\t- " << serverConfig.port << std::endl;
    // Copiarse la configuracion en formato struct serverConfig (con una funcion statica de configparser)
    ConfigParser::copyServerConfig(serverConfig, Server::config);
    buffer = new char[config.client_max_body_size];
}

// -------------------------------- Destructor -------------------------------------------------------
Server::~Server()
{
    std::cout << "Destructor of Server Called" << std::endl;
    delete[] buffer;
    std::vector<ClientInfo*>::iterator it = clients.begin();
    while (it != clients.end())
    {
        delete (*it);
        it++;
    }
    clients.clear();
}

// --------------------------------- CORE FUNCTIONALITIES ------------------------------------------

int Server::getPassiveSocketFd() const
{
    return MotherSocket::getPassiveSocketFd();
}


void    Server::init()
{
    setNonBlocking();

    // Iteramos en el map de endpoints de struct serverConfig

    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = config.locations.begin(); it != config.locations.end(); ++it)
        router.loadEndpoints(it->first, it->second);

    // Creamos el primer cliente a polear (poll()) que seria nuestro socket passivo.
    ClientInfo serverInfo;
    serverInfo.pfd.fd = getPassiveSocketFd();
    serverInfo.pfd.events = POLLIN;
    serverInfo.pfd.revents = 0;
    serverInfo.lastActivity = 0;
    serverInfo.max = 0;
    serverInfo.timeout = 0;
    serverInfo.keepAlive = true;

    toPassiveSocket(10);

    std::string info_message("Server is listening on port ");
    std::ostringstream conversion_stream;
    conversion_stream << config.port;
    info_message += conversion_stream.str();
    info_message += std::string("  PassiveSocketBound: ");
    conversion_stream.clear();
    conversion_stream << getPassiveSocketFd();
    info_message += conversion_stream.str();
    info_message += " " + config.server_name;
    LOG_INFO(info_message);
}

// void Server::launch()
// {

//     std::vector<pollfd> pollFds(1, clients.back().pfd);

//     while (42)
//     {
//         // std::cout << "\nWaiting for events... Current clients: " << (clients.size() - 1) << "\n"<<std::endl;
        
//         int pollCount = poll(pollFds.data(), pollFds.size(), 1000);

//         if (pollCount == -1)
//         {
//             if (errno != EINTR) // Ignorar interrupciones del sistema
//             {
//                 ServerError error("Poll failed");
//                 LOG_EXCEPTION(error);
//             }
//             continue;
//         }

//         // Controlar los timeouts y gestionar los eventos em este bucle
//         for (size_t i = 0; i < clients.size(); )
//         {
//             // sessiones con tiempo de ausencia superior a CONNECTION_TIMEOUT miembro privado server.
//             if (IsTimeout(i))
//             {
//                 pollFds.erase(pollFds.begin() + i);
//                 continue;
//             }

//             // disponible para leer/escribir
//             if (pollFds[i].revents & POLLIN)
//             {
//                 if (pollFds[i].fd == getPassiveSocketFd())
//                 {
//                     clients[0].lastActivity = time(NULL);
//                     acceptClient();
//                     pollFds.push_back(clients.back().pfd);
//                 }
//                 else
//                 {
//                     std::cout << "\nHandling Request of resource demand on Fd: " << pollFds[i].fd << "\n"<<std::endl;

//                     handleClient(i);
//                 }
//             }
//             else if (pollFds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
//             {
//                 std::cout << "Removing Connection FD: " << clients[i].pfd.fd << std::endl;
//                 removeClient(i);
//                 pollFds.erase(pollFds.begin() + i);
//                 continue;
//             }

//             ++i;
//         }
//     }
// }

void Server::acceptClient() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientFd = accept(getPassiveSocketFd(), (struct sockaddr*)&clientAddr, &clientAddrLen);
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

    std::string info_message("New Client on Fd: ");
    std::ostringstream      int_output;
    int_output << clientFd;
    info_message += int_output.str();
    LOG_INFO(info_message.c_str());
}


void Server::sendResponse(int clientSocket, const std::string& response)
{
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
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
        clients.erase(it);
}


void Server::handleClient(ClientInfo* client)
{
    try {
        Client clientHandler(client);
        
        analyzeBasicHeaders(clientHandler.getRequest(), clientHandler.getResponse(), client);
        router.route(clientHandler.getRequest(), clientHandler.getResponse());
        sendResponse(client->pfd.fd, clientHandler.getResponse()->toString());
        
        client->keepAlive = clientHandler.shouldKeepAlive();
        client->lastActivity = clientHandler.getLastActivity();
    }
    catch (const std::exception& e) {
        std::string error_message("Error handling client: ");
        error_message += std::string(e.what());
        LOG(error_message.c_str());
        removeClient(client);
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
