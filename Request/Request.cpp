/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 02:41:55 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "../Server/Server.hpp"
#include "../Router/Router.hpp"
#include "../Handlers/Handlers.hpp"

Request::Request(const size_t& ClientFd, ClientInfo& info_ref): info(info_ref)
{
    readStatusLine(ClientFd);
    readHeaders(ClientFd);
    readBody(ClientFd);
}
void    Request::readStatusLine(const size_t& clientFd)
{
    char c;
    std::string statusLine;
    
    while (42) {
        ssize_t result = recv(clientFd, &c, 1, 0);
        if (result <= 0)
            throw RequestError("Invalid read on clientFd for status line");
            
        statusLine += c;
        if (c == '\n' && statusLine.length() >= 2 && 
            statusLine[statusLine.length()-2] == '\r')
            break;
        
        if (statusLine.length() > 8192) // Por si el status line es enorme
            throw RequestError("Status line too long");
    }
    std::istringstream lineStream(statusLine);
    lineStream >> method >> uri >> httpVersion;
    if (method.empty() || uri.empty() || httpVersion.empty())
        throw RequestError("Invalid Status line !");
}

void    Request::readHeaders(const size_t& clientFd)
{
    std::string headerLine;
    std::string allHeaders;
    bool endOfHeaders = false;
    
    while (!endOfHeaders) {
        char c;
        ssize_t result = recv(clientFd, &c, 1, 0);
        if (result <= 0)
            throw RequestError("Invalid read on ClientFd for headers");
            
        headerLine += c;
        allHeaders += c;
        
        if (c == '\n') {
            if (headerLine == "\r\n") {
                endOfHeaders = true;
            }
            headerLine.clear();
        }
        
        if (allHeaders.length() > info.client_max_body_size)
            throw RequestError("Headers too long");
    }

    std::istringstream HeadersStream(allHeaders);
    std::string line;

    while (std::getline(HeadersStream, line) && line != "\r")
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim whiteSpace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            headers[key] = value;
            
        }
    }
}

bool Request::readBody(const size_t& ClientFd)
{
    std::string contentLength = getHeader("Content-Length");
    std::string transferEncoding = getHeader("Transfer-Encoding");
    
    if (contentLength.empty() && transferEncoding.empty())
        return true; // Pas de body
    
    if (!contentLength.empty())
        return readContentLengthBody(ClientFd, std::atoi(contentLength.c_str()));
    
    if (transferEncoding == "chunked")
        return readChunkedBody(ClientFd);
        
    throw RequestError("Unsupported Transfer-Encoding");
}

bool Request::readContentLengthBody(const size_t& ClientFd, size_t length)
{
    if (length > info.client_max_body_size)
        throw RequestError("Body size exceeds maximum allowed size");
        
    char buffer[4096];
    size_t totalRead = 0;
    
    while (totalRead < length)
    {
        size_t toRead = std::min(sizeof(buffer), length - totalRead);
        ssize_t bytesRead = recv(ClientFd, buffer, toRead, 0);
        
        if (bytesRead <= 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                throw RequestError("Error reading body");
            continue;
        }
        
        body.append(buffer, bytesRead);
        totalRead += bytesRead;
    }
    
    return true;
}

bool Request::readChunkedBody(const size_t& ClientFd)
{
    std::string tempBody;
    
    while (true)
    {
        // 1. Lire la taille du chunk
        std::string chunkSizeStr;
        char c;
        
        while (true) {
            ssize_t result = recv(ClientFd, &c, 1, 0);
            if (result <= 0)
                throw RequestError("Error reading chunk size");
                
            if (c == '\r') {
                if (recv(ClientFd, &c, 1, 0) <= 0 || c != '\n')
                    throw RequestError("Invalid chunk format");
                break;
            }
            chunkSizeStr += c;
        }
        
        // 2. Convertir la taille hexadécimale en décimal
        unsigned long chunkSize;
        std::istringstream iss(chunkSizeStr);
        iss >> std::hex >> chunkSize;
        
        // 3. Si chunk size est 0, c'est la fin
        if (chunkSize == 0) {
            // Lire le CRLF final
            if (recv(ClientFd, &c, 1, 0) <= 0 || c != '\r' ||
                recv(ClientFd, &c, 1, 0) <= 0 || c != '\n')
                throw RequestError("Invalid end of chunked body");
            break;
        }
        
        // 4. Vérifier la taille maximale
        if (tempBody.length() + chunkSize > info.client_max_body_size)
            throw RequestError("Chunked body exceeds maximum size");
            
        // 5. Lire le chunk
        std::vector<char> chunk(chunkSize);
        size_t bytesRead = 0;
        
        while (bytesRead < chunkSize) {
            ssize_t result = recv(ClientFd, &chunk[bytesRead], 
                                chunkSize - bytesRead, 0);
            if (result <= 0)
                throw RequestError("Error reading chunk data");
            bytesRead += result;
        }
        
        // 6. Ajouter le chunk au body
        tempBody.append(chunk.begin(), chunk.end());
        
        // 7. Lire le CRLF après le chunk
        if (recv(ClientFd, &c, 1, 0) <= 0 || c != '\r' ||
            recv(ClientFd, &c, 1, 0) <= 0 || c != '\n')
            throw RequestError("Invalid chunk format");
    }
    
    body = tempBody;
    return true;
}


// bool    Request::HostHeader()
// {
//     std::string host(this->getHeader("Host"));

//     if (host.empty() || isValidHostHeader(host) == false)
//         return false;
//     return true;
// }



// void Request::parse(const std::string& rawRequest)
// {
//     std::istringstream requestStream(rawRequest);
//     std::string line;

//     // Parse request line
//     if (std::getline(requestStream, line))
//     {
//         std::istringstream lineStream(line);
//         lineStream >> method >> uri >> httpVersion;
//         if (method.empty() || uri.empty() || httpVersion.empty())
//             LOG("Error parsing request line: invalid format");
//     } else{
//         LOG("Error parsing request line");
//         return;
//     }

//     // Parse headers
//     while (std::getline(requestStream, line) && line != "\r")
//     {
//         size_t colonPos = line.find(':');
//         if (colonPos != std::string::npos) {
//             std::string key = line.substr(0, colonPos);
//             std::string value = line.substr(colonPos + 1);
            
//             // Trim whiteSpace
//             key.erase(0, key.find_first_not_of(" \t"));
//             key.erase(key.find_last_not_of(" \t") + 1);
//             value.erase(0, value.find_first_not_of(" \t"));
//             value.erase(value.find_last_not_of(" \t\r\n") + 1);

//             headers[key] = value;
//         }
//     }

//     // Parse body
//     std::string bodyLine;
//     while (std::getline(requestStream, bodyLine))
//         body += bodyLine + "\n";
//     //if (!body.empty())
//     //    body.pop_back(); // Remove last newline
// }

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
