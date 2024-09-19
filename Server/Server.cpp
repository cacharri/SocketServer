#include "Server.hpp"
#include "../Request/Request.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>

Server::Server(const ServerConfig& serverConfig)
    : MotherSocket(AF_INET, SOCK_STREAM, 0, serverConfig.port, "127.0.0.1"),
      buffer(new char[INITIAL_BUFFER_SIZE]),
      bufferSize(INITIAL_BUFFER_SIZE)
{

}

Server::~Server()
{
    delete[] buffer;
}

void    Server::init()
{
    toPassiveSocket(10);
    setNonBlocking();
}

void Server::launch()
{
    pollfd serverPollFd = {getSocketFd(), POLLIN, 0};
    fds.push_back(serverPollFd);

    while (42)
    {
        int pollCount = poll(&fds[0], fds.size(), -1);
        if (pollCount == -1)
        {
            ServerError error("Poll failed");
            LOG_EXCEPTION(error);
        }

        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == getSocketFd())
                    acceptClient();
                else
                    handleClient(i);
            }
            else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                removeClient(i);
        }
    }
}

void Server::acceptClient() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientFd = accept(getSocketFd(), (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ServerError error("Accept incoming socket failed !");
            LOG_EXCEPTION(error);
        }
        return;
    }

    // Set the new client socket to non-blocking
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(clientFd);
        ServerError error("Failed to set client socket to non-blocking !");
        LOG_EXCEPTION(error);
    }

    pollfd clientPollFd = {clientFd, POLLIN, 0};
    fds.push_back(clientPollFd);

    std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
}

void Server::handleClient(size_t index) {
    int         clientFd = fds[index].fd;
    try {
        
        std::string request_str = receiveMessage(clientFd);
        if (request_str.empty())
        {
            removeClient(index);
            return;
        }
        Request request(request_str);

        // Process the request and generate a response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        sendResponse(clientFd, response);
    }
    catch (const std::exception& e)
    {
        std::string error("Error handling client: ");
        error += e.what();
        LOG(error);
        removeClient(index);
    }
}

std::string Server::receiveMessage(int clientSocket) {
    std::string message;
    ssize_t bytesRead;
    size_t totalBytesRead = 0;

    do {
        if (totalBytesRead >= bufferSize)
        {
            // Resize buffer if needed
            size_t newSize = bufferSize * 2;
            char* newBuffer = new char[newSize];
            std::copy(buffer, buffer + bufferSize, newBuffer);
            delete[] buffer;
            buffer = newBuffer;
            bufferSize = newSize;
        }

        bytesRead = recv(clientSocket, buffer + totalBytesRead, bufferSize - totalBytesRead, 0);
        if (bytesRead > 0)
            totalBytesRead += bytesRead;

        else if (bytesRead == 0)// Connection closed
            break;

        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ServerError error("Error reading from socket !");
            LOG_EXCEPTION(error);
        }
    } while (bytesRead > 0);

    message.assign(buffer, totalBytesRead);
    return message;
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

void Server::removeClient(size_t index)
{
    close(fds[index].fd);
    fds.erase(fds.begin() + index);
}

//Exceptions handling
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