/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/21 18:46:23 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "../Server/Server.hpp"
#include "../Router/Router.hpp"
#include "../Handlers/Handlers.hpp"

Request::Request(ClientInfo& info_ref)
    : info(info_ref)
{
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
            headers = std::string(buffer, endHeaders - buffer + 4);
            tempBody += std::string(endHeaders + 4);
            break;
        }
        else
            throw RequestError("Invalid request format: no header delimiter found");
    }

    std::cout << headers << std::endl;
    //std::cout << "Headers : " << headers << std::endl;
    if (!headerComplete)
    {
        LOG_INFO("ADIOS headers incomplete");
        return false;        
    }
    parseRequest(buffer);

    // Comprobar Transfer-encoding para chunked mode
    std::string transferEncoding = getHeader("Transfer-Encoding");
    if (transferEncoding == "chunked")
        return readChunkedBody(ClientFd);
    else {
        size_t contentLength = parseContentLength(headers, maxSize);
        if (tempBody.size() >= contentLength - 1)
        {
            body += tempBody;
            return true;
        }
        tempBody.resize(contentLength);
        return readContentLengthBody(ClientFd, contentLength, tempBody);
    }
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
        // ignorar lineas vacias
        if (line.empty() || line == "\r") {
            break;
        }
        // Si hay una asignacion de header con ':' anadirlo al miembro mapa
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            // Borrar espacios y tabs 
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);

            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            // Almacenar en headers, mapa de strings miembro del objeto Request
            this->headers[key] = value;
        }
    }
}

bool    Request::readContentLengthBody(const size_t& ClientFd, size_t contentLength, std::string& tempBody)
{
    // Leer body teniendo en cuenta el COntent-legnht como limite
    size_t totalBodyRead = 0;
    std::cout << contentLength << std::endl;
    while (totalBodyRead < contentLength)
    {
        //std::cout << "loop read body" << std::endl;
        ssize_t bytesRead = recv(ClientFd, &tempBody[totalBodyRead], contentLength - totalBodyRead, 0);
        //std::cout << "I read " << bytesRead << "From body" << std::endl;
        if (bytesRead < 0)
        {
            LOG_INFO("ADIOS invalid body read");
            return false;
        }
        else if (bytesRead == 0)
        {
            LOG_INFO("finished");
            break ;
        }
        totalBodyRead += bytesRead;
        std::cout << "Leido: " << totalBodyRead << std::endl;
    }
    body += tempBody;
    std::cout << "tamaño body: " << body.size() << std::endl;

    return true;
}

size_t  Request::parseContentLength(std::string&  headers, size_t max_size)
{
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
    if (contentLength > max_size)
        throw RequestError("Request body exceeds maximum allowed size");
    return contentLength;
}


//  Lee datos en formato chunked de un socket
//  ClientFd Socket descriptor del cliente
//  return true si la lectura fue exitosa, false en caso contrario
 
//  Formato de datos chunked:
//  [tamaño en hex]\r\n
//  [datos]\r\n
//  [datos]\r\n
//  [siguiente tamaño en hex]\r\n
//  [datos]\r\n
//  0\r\n\r\n
 
bool Request::readChunkedBody(const size_t& ClientFd)
{
    LOG_INFO("Chunked request");
    std::string tempBody;
    char buffer[4096];
    size_t totalSize = 0;

    try {
        while (true)
        {
            // 1. Leer tamaño del chunk
            std::string chunkSizeStr;
            bool foundCRLF = false;
            
            while (!foundCRLF && chunkSizeStr.length() < 10)
            {
                char c;
                ssize_t bytesRead = recv(ClientFd, &c, 1, 0);
                if (bytesRead <= 0)
                    throw RequestError("Error leyendo tamaño del chunk");
                std::cout << "readChar: " << c << std::endl;

                chunkSizeStr += c;
                if (chunkSizeStr.length() >= 2 && 
                    chunkSizeStr[chunkSizeStr.length() - 2] == '\r' && 
                    chunkSizeStr[chunkSizeStr.length() - 1] == '\n')
                {
                    chunkSizeStr = chunkSizeStr.substr(0, chunkSizeStr.length() - 2);
                    foundCRLF = true;
                }
                //std::cout << "Got chunk of raw Size: " << chunkSizeStr << std::endl;
            }
            // 2. Convertir hex a decimal para el tamano del chunk
            unsigned long chunkSize = 0;
            std::istringstream iss(chunkSizeStr);
            iss >> std::hex >> chunkSize;


            std::cout << "Got chunk size of: " << chunkSize << std::endl;

            // 3. Si es chunk final (size = 0)
            if (chunkSize == 0)
            {
                // Leer CRLF final
                char crlf[2];
                ssize_t bytesRead = recv(ClientFd, crlf, 2, 0);
                if (bytesRead != 2 || crlf[0] != '\r' || crlf[1] != '\n')
                    throw RequestError("CRLF final inválido");
                break;
            }
            // 4. Verificar tamaño máximo
            if (totalSize + chunkSize > info.client_max_body_size)
                throw RequestError("Tamaño del body excede el límite");

            // 5. Leer datos del chunk
            size_t bytesRemaining = chunkSize;
            std::string chunkData;

            while (bytesRemaining > 0)
            {
                size_t toRead = (bytesRemaining < sizeof(buffer)) ? bytesRemaining : sizeof(buffer);
                ssize_t bytesRead = recv(ClientFd, buffer, toRead, 0);
                if (bytesRead <= 0)
                    throw RequestError("Error leyendo datos del chunk");
                

                chunkData.append(buffer, bytesRead);
                bytesRemaining -= bytesRead;
            }

            std::cout << "read of chunk; " << chunkData << std::endl;

            // 6. Leer CRLF después del chunk
            char crlf[2];
            ssize_t bytesRead = recv(ClientFd, crlf, 2, 0);
            if (bytesRead != 2 || crlf[0] != '\r' || crlf[1] != '\n')
                throw RequestError("CRLF después del chunk inválido");

            // 7. Añadir chunk al body
            tempBody += chunkData;
            totalSize += chunkSize;
        }
        std::cout << "\nAll chunked Data: " << tempBody << std::endl;
        body = tempBody;
        
        return true;
    }
    catch (const RequestError& e) {
        LOG("Error en lectura chunked: " + std::string(e.what()));
        return false;
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
