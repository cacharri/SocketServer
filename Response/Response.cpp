/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 00:30:32 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 20:33:10 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::string getDefaultStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 404: return "Not Found";

        default: return "Unknown";
    }
}

void Response::setStatus(int code, const std::string& message)
{
    statusCode = code;
    statusMessage = message;
}
Response::Response(int code) : statusCode(code)
{
    setStatus(code, getDefaultStatusMessage(code));
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
    std::ostringstream oss;
    oss << body.length(); 
    std::string value = oss.str();
    setHeader("Content-Length", value); // Set the header
}

std::string Response::toString() const
{
    std::ostringstream res;
    res << "HTTP/1.1 " << statusCode << " "<< statusMessage << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        res << it->first << ": " << it->second << "\r\n";
    }
    
    res << "\r\n" << body;
    return res.str();
}


// Getters implementation
int Response::getStatusCode() const {
    return statusCode;
}

std::string Response::getStatusMessage() const {
    return statusMessage;
}

std::string Response::getBody() const {
    return body;
}


