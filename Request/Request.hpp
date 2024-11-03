/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:28:40 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/03 02:56:04 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../Client/Client.hpp"

#include <sys/socket.h>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>

class Request {
private:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    struct ClientInfo& info;
    
public:
    Request(ClientInfo& info_ref);

    bool readData(const size_t& ClientFd, size_t maxSize);
    size_t parseContentLength(const std::string& headers);
    void    parseRequest();


    // void readStatusLine(const size_t& ClientFd);
    // void readHeaders(const size_t& ClientFd);
    // bool readBody(const size_t& ClientFd);
    // bool readContentLengthBody(const size_t& ClientFd, size_t length);
    // bool readChunkedBody(const size_t& ClientFd);

    // bool HostHeader();
    // bool ConnectionHeader();
    // bool ContentLengthHeader();
    // void ContentTypeHeader();
    
    // void parse(const std::string& rawRequest);
    void print(void) const;
    
    // Getters1
    std::string getMethod() const;
    std::string getUri() const;
    std::string getHttpVersion() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
    std::string getPath() const;
    // Setter
    void setBody(const std::string& request_body);

    class RequestError : public std::exception {
        private:
            std::string error_msg;
        public:
            RequestError(const std::string& msg): error_msg(msg) {}
            virtual ~RequestError() throw() {}
            virtual const char* what() const throw() { return error_msg.c_str(); }
    };
};
#endif