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
    delete[] buffer;
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

    // lo anadimos a nuestro vectore para polear
    clients.push_back(&serverInfo);

    toPassiveSocket(10);

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
    //  socket to non-blocking
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(clientFd);
        ServerError error("Failed to set client socket to non-blocking !");
        LOG_EXCEPTION(error);
    }

    ClientInfo newClient;
    newClient.pfd.fd = clientFd;
    newClient.pfd.events = POLLIN;
    newClient.pfd.revents = 0;
    newClient.lastActivity = time(NULL);
    newClient.max = 10;
    newClient.timeout = 10;
    newClient.client_max_body_size = config.client_max_body_size;
    newClient.keepAlive = true; // Par defecto suponemos keep-alive
    clients.push_back(&newClient);

    std::cout << "New Client on Fd: " << newClient.pfd.fd << " " << newClient.lastActivity << std::endl;
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
        LOG("Error handling client: " + std::string(e.what()));
        removeClient(client);
    }
}


bool    Server::IsTimeout(ClientInfo* client)
{
    if (!client->keepAlive || 
        difftime(time(NULL), client->lastActivity) > CONNECTION_TIMEOUT)
        return true;
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
