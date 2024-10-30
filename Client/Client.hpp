/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 23:02:59 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 03:10:33 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../Config/ConfigParser.hpp"
#include "../Response/Response.hpp"
#include "../Request/Request.hpp"
#include "../Handlers/Handlers.hpp"

#include <errno.h>
#include <poll.h>
#include <string>
#include <vector>
#include <exception>
#include <sys/time.h>


struct ClientInfo {
    pollfd          pfd;
    time_t          lastActivity;
    size_t          client_max_body_size;
    bool            keepAlive;
    unsigned int    timeout;
    unsigned int    max;
};

class Client {
private:
    size_t      clientFd;
    ClientInfo& session_info;
    Request     *request;
    Response    *response;
    std::vector<char> buffer;


public:
    Client(size_t fd, ClientInfo& session_info);
    ~Client();

    void        ReadFromConexion();
    bool        HandleConnection();

    Request*    getRequest() { return request; }
    Response*   getResponse() { return response; }
    
    bool        shouldKeepAlive() const { return session_info.keepAlive; }
    time_t      getLastActivity() const { return session_info.lastActivity; }

    class ClientError : public std::exception {
        private:
            std::string error_msg;
        public:
            ClientError(const std::string& msg) : error_msg(msg) {}
            virtual ~ClientError() throw() {}
            virtual const char* what() const throw() { return error_msg.c_str(); }
    };
};

#endif