/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MotherSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 13:57:44 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/06 15:48:50 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MotherSocket.hpp"
#include <stdexcept>
#include <cstring>
#include <unistd.h>

MotherSocket::MotherSocket(int domain, int service, int protocol, int port, const std::string& interface){
    std::memset(&socketStruct, 0, sizeof(socketStruct));
    socketStruct.sin_family = domain;
    socketStruct.sin_addr.s_addr = inet_addr(interface.c_str()); //INADDR_ANY
    socketStruct.sin_port = htons(port);

    socketFd = socket(domain, service, protocol);
    setSocketOption(SOL_SOCKET, SO_REUSEADDR);
    testConnection();
}

MotherSocket::~MotherSocket() {
    if (socketFd != -1) {
        close(socketFd);
    }
}

int MotherSocket::getPassiveSocketFd() const {
    return socketFd;
}

const sockaddr_in& MotherSocket::getAddress() const {
    return socketStruct;
}

void MotherSocket::setNonBlocking() {
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) {
        SocketError error("Failed to set socket to non-blocking");
        LOG_EXCEPTION(error);
        throw error;
    }
}

void MotherSocket::toPassiveSocket(int queueLimit) {
    if (bind(socketFd, reinterpret_cast<struct sockaddr*>(&socketStruct), sizeof(socketStruct)) == -1) {
        SocketError error("Failed to bind socket");
        LOG_EXCEPTION(error);
    } //else
        //std::cout << "PassiveSocket: " << socketFd << " bound successfully to " << inet_ntoa(socketStruct.sin_addr) << ":" << ntohs(socketStruct.sin_port) << std::endl;

    if (listen(socketFd, queueLimit) == -1) {
        SocketError error("Failed to set passive socket");
        LOG_EXCEPTION(error);
    } //else
        //std::cout << "Server is now listening." << std::endl;
}

void MotherSocket::toActiveSocket() {
    if (connect(socketFd, reinterpret_cast<struct sockaddr*>(&socketStruct), sizeof(socketStruct)) == -1) {
        SocketError error("Failed to connect socket");
        LOG_EXCEPTION(error);
    }
}

void MotherSocket::setSocketOption(int optionLevel, int option) {
    int flag = 1;
    if (setsockopt(socketFd, optionLevel, option, &flag, sizeof(flag)) < 0) {
        SocketError error("Error setting socket options");
        LOG_EXCEPTION(error);
    }
}

void MotherSocket::testConnection() {
    if (socketFd < 0) {
        SocketError error("Invalid socket");
        LOG_EXCEPTION(error);
    }
}

//Exceptions handling
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