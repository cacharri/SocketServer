/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/06 14:07:44 by smagniny         ###   ########.fr       */
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

    while (totalRead < maxSize)
    {
        ssize_t bytesRead = recv(ClientFd, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            if (bytesRead == 0) // || (errno != EAGAIN && errno != EWOULDBLOCK))
                throw RequestError("Connection closed or error");
            break;
        }

        tempBody.append(buffer, bytesRead);
        totalRead += bytesRead;

        // BUscar el final de los headers (\r\n\r\n)
        if (!headerComplete)
        {
            size_t headerEnd = tempBody.find("\r\n\r\n");
            if (headerEnd != std::string::npos)
            {
                headerComplete = true;
                // si hay un content-lenght, podemos determinar el tamano total de la request (header + response)
                size_t contentLength = parseContentLength(tempBody.substr(0, headerEnd));
                if (contentLength > 0)
                {
                    if (contentLength > maxSize)
                        throw RequestError("Content length exceeds maximum allowed size");
                    // Continuar de leer hasta que tengamos todo el body
                    if (totalRead >= headerEnd + 4 + contentLength)
                        break;
                }
                else
                {
                    // NO hay body
                    break;
                }
            }
        }
    }
    body = tempBody;
    return true;
}

size_t Request::parseContentLength(const std::string& headers)
{
    std::string contentLengthHeader = "Content-Length: ";
    size_t pos = headers.find(contentLengthHeader);
    if (pos == std::string::npos)
        return 0;

    size_t start = pos + contentLengthHeader.length();
    size_t end = headers.find("\r\n", start);
    if (end == std::string::npos)
        return 0;

    std::string lengthStr = headers.substr(start, end - start);
    return std::atoi(lengthStr.c_str());
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
