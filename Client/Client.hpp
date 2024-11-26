/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 23:02:59 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/06 14:01:15 by smagniny         ###   ########.fr       */
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
#include <exception>


struct ClientInfo {
    pollfd          pfd;
    time_t          lastActivity;
    size_t          client_max_body_size;
    bool            keepAlive;
    unsigned int    timeout;
    unsigned int    max;
    
    ClientInfo() 
        : lastActivity(time(NULL))
        , client_max_body_size(8192)
        , keepAlive(true)
        , timeout(60)
        , max(100)
    {
        pfd.fd = -1;
        pfd.events = 0;
        pfd.revents = 0;
    }
};

class Client {
private:
    size_t      clientFd;
    ClientInfo* session_info;
    Request     *request;
    Response    *response;

public:
    Client(ClientInfo* session_info);
    ~Client();

    bool        HandleConnection();
    std::string to_string(int value);

    Request*    getRequest();
    Response*   getResponse();
    
    bool        shouldKeepAlive()const;
    time_t      getLastActivity() const;
    
    void        setLastActivity() const;
    void        sendRResponse(int clientSocket, const std::string& response); 

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