/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/12 03:04:58 by smagniny         ###   ########.fr       */
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
        // Parsear la requeste
        parseRequest();
    }
    catch (const std::exception& e) {
        throw RequestError(e.what());
    }
}

bool Request::readData(const size_t& ClientFd, size_t maxSize)
{
    char buffer[2046];  // Buffer de talla razonable
    size_t totalRead = 0;
    bool headerComplete = false;
    std::string tempBody;

    // leer headers del fd
    while (totalRead < sizeof(buffer) - 1) {
        ssize_t bytesRead = recv(ClientFd, buffer + totalRead, sizeof(buffer) - totalRead - 1, 0);
        if (bytesRead <= 0)
            return false;
        totalRead += bytesRead;
        
        // comprobar si hemos llegado al final de los headers
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            headerComplete = true;            
            buffer[totalRead] = '\0';
            break;
        }
    }

    if (!headerComplete)
        return false;        

    // Separar headers y body
    std::string headers(buffer);
    size_t headerEnd = headers.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        throw RequestError("Invalid request format: no header delimiter found");

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
    if (contentLength > maxSize) {
        throw RequestError("Request body exceeds maximum allowed size");
    }
    body = buffer;
    // Leer el body con el tamano correcto. Content-lenght de la request ( inferior al client_max_body_size )
    tempBody.resize(contentLength);
    size_t totalBodyRead = 0;
    while (totalBodyRead < contentLength)
    {
        ssize_t bytesRead = recv(ClientFd, &tempBody[totalBodyRead], contentLength - totalBodyRead, 0);
        if (bytesRead <= 0)
            return false;
        totalBodyRead += bytesRead;
    }
    
    body += tempBody;
    return true;
}

void Request::parseRequest()
{
    size_t headerEnd = body.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        throw RequestError("Invalid request format: no header delimiter found");

    // seprar headers y body
    std::string headers = body.substr(0, headerEnd);
    if (headerEnd + 4 < body.length())
        body = body.substr(headerEnd + 4);
    else
        body.clear();

    // parseo status-line
    size_t firstLineEnd = headers.find("\r\n");
    if (firstLineEnd == std::string::npos)
        throw RequestError("Invalid request format: no status line");

    std::string statusLine = headers.substr(0, firstLineEnd);
    std::istringstream statusStream(statusLine);
    statusStream >> method >> uri >> httpVersion;

    if (method.empty() || uri.empty() || httpVersion.empty())
        throw RequestError("Invalid status line format");

    // Parseo de los headers
    std::string headerSection = headers.substr(firstLineEnd + 2);
    std::istringstream headerStream(headerSection);
    std::string line;

    while (std::getline(headerStream, line))
    {
        if (line == "\r" || line.empty())
            break;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // borar tabs espacios \n\r
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
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
