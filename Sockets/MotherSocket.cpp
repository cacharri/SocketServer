/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MotherSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 13:57:44 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/09 21:29:43 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MotherSocket.hpp"
#include <stdexcept>
#include <cstring>
#include <unistd.h>

MotherSocket::MotherSocket(int domain, int service, int protocol, const std::vector<int>& ports, const std::string& interface)
{
    for (std::vector<int>::const_iterator it = ports.begin(); it != ports.end(); it++)
    {
        int newSocketFd = socket(domain, service, protocol);
        if (newSocketFd < 0) {
            SocketError error("Failed to create socket");
            LOG_EXCEPTION(error);
        }

        sockaddr_in newSocketStruct;
        std::memset(&newSocketStruct, 0, sizeof(newSocketStruct));
        newSocketStruct.sin_family = domain;
        newSocketStruct.sin_port = htons(*it);
        newSocketStruct.sin_addr.s_addr = inet_addr(interface.c_str());
        
        int flag = 1;
        if (setsockopt(newSocketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
            close(newSocketFd);
            SocketError error("Error setting socket options");
            LOG_EXCEPTION(error);
        }

        socketFds.push_back(newSocketFd);
        socketStructs.push_back(newSocketStruct);
    }
}

MotherSocket::~MotherSocket()
{
    for (std::vector<int>::iterator it = socketFds.begin(); it != socketFds.end(); it++)
    {
        if (*it >= 0)
            close(*it);
    }
}

std::vector<int> MotherSocket::getPassiveSocketFd() const
{
    return socketFds;
}

void MotherSocket::setNonBlocking()
{
    for (std::vector<int>::iterator it = socketFds.begin(); it != socketFds.end(); it++)
    {
        if (fcntl(*it, F_SETFL, O_NONBLOCK) == -1) {
            SocketError error("Failed to set socket to non-blocking");
            LOG_EXCEPTION(error);
        }
    }
}

void MotherSocket::toPassiveSocket(int queueLimit)
{
    for (size_t i = 0; i < socketFds.size(); i++)
    {
        if (bind(socketFds[i], reinterpret_cast<struct sockaddr*>(&socketStructs[i]), 
                sizeof(sockaddr_in)) == -1)
        {
            SocketError error("Failed to bind socket");
            LOG_EXCEPTION(error);
        }

        if (listen(socketFds[i], queueLimit) == -1)
        {
            SocketError error("Failed to set passive socket");
            LOG_EXCEPTION(error);
        }
    }
}

void MotherSocket::toActiveSocket()
{
    for (size_t i = 0; i < socketFds.size(); i++)
    {
        if (connect(socketFds[i], reinterpret_cast<struct sockaddr*>(&socketStructs[i]), 
                   sizeof(sockaddr_in)) == -1)
        {
            SocketError error("Failed to connect socket");
            LOG_EXCEPTION(error);
        }
    }
}

void MotherSocket::setSocketOption(int optionLevel, int option)
{
    int flag = 1;
    for (std::vector<int>::iterator it = socketFds.begin(); it != socketFds.end(); it++)
    {
        if (setsockopt(*it, optionLevel, option, &flag, sizeof(flag)) < 0) {
            SocketError error("Error setting socket options");
            LOG_EXCEPTION(error);
        }
    }
}

// Exception handling
MotherSocket::SocketError::SocketError(const std::string& error): error_string(error)
{
}

MotherSocket::SocketError::~SocketError() throw()
{
}

const char * MotherSocket::SocketError::what() const throw()
{
    return error_string.c_str();
}