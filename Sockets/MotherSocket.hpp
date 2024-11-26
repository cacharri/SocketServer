/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MotherSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 13:57:16 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/09 21:29:52 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MOTHERSOCKET_HPP
#define MOTHERSOCKET_HPP

#include "../Logger/Logger.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <vector>

class MotherSocket {
public:
    MotherSocket(int domain, int service, int protocol, const std::vector<int>& ports, const std::string& interface);
    virtual ~MotherSocket();

    std::vector<int> getPassiveSocketFd() const;

    void setNonBlocking();
    void toPassiveSocket(int queueLimit);
    void toActiveSocket();

    class SocketError: public std::exception {
        private:
            std::string error_string;
        public:
            SocketError(const std::string& error);
            virtual const char* what() const throw();
            virtual ~SocketError() throw();
    };
    
protected:
    std::vector<int>             socketFds;
    std::vector<sockaddr_in>     socketStructs;

private:
    void setSocketOption(int optionLevel, int option);
    void testConnection();
};

#endif