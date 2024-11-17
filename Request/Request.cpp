/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/17 14:00:33 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "../Server/Server.hpp"
#include "../Router/Router.hpp"
#include "../Handlers/Handlers.hpp"

Request::Request(ClientInfo& info_ref)
    : info(info_ref)
{
    try {
        // leer toda la request de 'golpe' hasta client_max_body_size como limite
        if (!readData(info_ref.pfd.fd, info.client_max_body_size))
            throw RequestError("Failed to read request data");
    }
    catch (const std::exception& e) {
        throw RequestError(e.what());
    }
}

bool Request::readData(const size_t& ClientFd, size_t maxSize)
{
    char buffer[1046];  // Buffer de talla razonable
    size_t totalRead = 0;
    bool headerComplete = false;
    std::string tempBody;
    std::string headers;

    // leer headers del fd
    while (totalRead < sizeof(buffer) - 1) {
        ssize_t bytesRead = recv(ClientFd, buffer + totalRead, sizeof(buffer) - totalRead - 1, 0);
        if (bytesRead <= 0)
        {
            LOG_INFO("ADIOS invalid headers read");
            return false;        
        }
        totalRead += bytesRead;
        
        // comprobar si hemos llegado al final de los headers
        char *endHeaders = strstr(buffer, "\r\n\r\n");
        if (endHeaders != NULL)
        {
            headerComplete = true;
            headers = std::string(buffer);
            headers[( (headers.rfind("\r\n\r\n")) + 4 )] = '\0';
            tempBody += std::string(endHeaders + 4);
            break;
        }
        else
            throw RequestError("Invalid request format: no header delimiter found");

    }

    //std::cout << "Headers : " << headers << std::endl;
    if (!headerComplete)
    {
        LOG_INFO("ADIOS headers incomplete");
        return false;        
    }

    // Parsear Content-Length
    size_t contentLength = 0;
    size_t contentLengthPos = headers.find("Content-Length: ");
    if (contentLengthPos != std::string::npos) {
        size_t lengthStart = contentLengthPos + 16; // valor de "Content-Length: "
        size_t lengthEnd = headers.find("\r\n", lengthStart);
        std::string lengthStr = headers.substr(lengthStart, lengthEnd - lengthStart);
        contentLength = static_cast<size_t>(atoi(lengthStr.c_str()));
    }
    // comprobar que el tamano sea menor que el de la config 
    if (contentLength > maxSize)
        throw RequestError("Request body exceeds maximum allowed size");

    // Leer el body con el tamano correcto. Content-lenght de la request ( inferior al client_max_body_size )
    //std::cout << "Size of body read meanwhie reading headers: " << tempBody.size() << std::endl;
    //std::cout << "Body size: " << contentLength << std::endl;

    if (tempBody.size() >= contentLength - 1)
    {
        body += tempBody;
        parseRequest(headers);
        return true;
    }

    tempBody.resize(contentLength);
    size_t totalBodyRead = 0;
    while (totalBodyRead < contentLength)
    {
        std::cout << "loop read body" << std::endl;
        ssize_t bytesRead = recv(ClientFd, &tempBody[totalBodyRead], contentLength - totalBodyRead, 0);
        std::cout << "I read " << bytesRead << "From body" << std::endl;
        if (bytesRead <= 0)
        {
            LOG_INFO("ADIOS invalid body read");
            break ;        
        }
        totalBodyRead += bytesRead;
    }
    body += tempBody;
    parseRequest(headers);
    return true;
}

void Request::parseRequest(std::string headers) {
    // Parsing status-line
    std::string statusLine;
    std::istringstream LineStatusStream(headers);
    std::getline(LineStatusStream, statusLine);  // Read the first line

    // Parse status line (method, uri, http version)

    //LOG_INFO(statusLine);
    std::istringstream statusLineStream(statusLine);
    statusLineStream >> method >> uri >> httpVersion;

    if (method.empty() || uri.empty() || httpVersion.empty())
        throw RequestError("Invalid status line format");

    // Parsing headers
    std::string line;
    while (std::getline(LineStatusStream, line)) {
        // Ignore empty lines and lines with just carriage return
        if (line.empty() || line == "\r") {
            break;
        }

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // Remove leading/trailing spaces and tabs
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);

            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            // Store in the map
            this->headers[key] = value;
        }
    }
}

std::string Request::getMethod() const
{
    return method;
}

std::string Request::getUri() const
{
    return uri;
}

std::string Request::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

std::string Request::getBody() const {
    return body;
}

std::string Request::getHttpVersion() const {
    return httpVersion;
}

void Request::setBody(const std::string& requestBody) {
        body = requestBody;
}

//https://stackoverflow.com/questions/31950470/what-is-the-upgrade-insecure-requests-http-header/32003517#32003517
void    Request::print(void)const
{
    std::cout << "-----------STATUS LINE-----------" << std::endl;
    std::cout << getMethod() << " " << getUri() << " " << getHttpVersion() << std::endl;
    std::map<std::string, std::string>::const_iterator it = headers.begin();
    std::cout << "-----------HEADERS-----------" << std::endl;
    for (it = headers.begin(); it != headers.end(); it++)
        std::cout << it->first << " " << it->second << std::endl; // key
    std::cout << "-----------BODY-----------" << std::endl;
    std::cout << getBody() << std::endl; 
}


std::string Request::getPath() const {
    size_t questionMarkPos = uri.find('?');
    if (questionMarkPos != std::string::npos) {
        return uri.substr(0, questionMarkPos); // Devuelve el path antes del '?'
    }
    return uri; // Si no hay query string, devuelve la URI completa
}
