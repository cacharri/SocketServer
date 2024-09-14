/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 00:30:32 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/15 01:09:43 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

class Response {
private:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    Response(int code = 200);
    void setStatus(int code, const std::string& message);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    std::string toString() const;

    // Helper methods
    void setContentType(const std::string& type);
    void setContentLength();
};

Response::Response(int code) : statusCode(code)
{
    setStatus(code, getDefaultStatusMessage(code));
}

void Response::setStatus(int code, const std::string& message)
{
    statusCode = code;
    statusMessage = message;
}

void Response::setHeader(const std::string& key, const std::string& value)
{
    headers[key] = value;
}

void Response::setBody(const std::string& content)
{
    body = content;
    setContentLength();
}

void Response::setContentType(const std::string& type)
{
    setHeader("Content-Type", type);
}

void Response::setContentLength()
{
    setHeader("Content-Length", std::to_string(body.length()));
}

std::string Response::toString() const
{
    std::ostringstream res;
    res << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        res << it->first << ": " << it->second << "\r\n";
    }
    
    res << "\r\n" << body;
    return res.str();
}


std::string getDefaultStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 404: return "Not Found";

        default: return "Unknown";
    }
}


