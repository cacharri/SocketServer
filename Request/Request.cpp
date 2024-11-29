/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/29 19:01:14 by smagniny         ###   ########.fr       */
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

// std::string Request::getErrorPagePath(int errorCode) {
//     std::stringstream path;
//     path << "var/www/error-pages/" << errorCode << ".html";
//     return path.str();
// }

// void Request::sendErrorPage(const size_t& ClientFd, const std::string& errorPagePath) {
//     std::ifstream errorFile(errorPagePath.c_str());
//     if (!errorFile.is_open()) {
//         LOG_INFO("Error page not found: " + errorPagePath);
//         std::string response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
//         send(ClientFd, response.c_str(), response.size(), 0);
//         return;
//     }

//     std::stringstream buffer;
//     buffer << errorFile.rdbuf();
//     errorFile.close();

//     std::string response = buffer.str();
//     ssize_t bytesSent = send(ClientFd, response.c_str(), response.size(), 0);

//     if (bytesSent < 0) {
//         LOG_INFO("Failed to send error page to client.");
//     }
// }
bool Request::readData(const size_t& ClientFd, size_t maxSize)
{
    std::string buffer;
    buffer.reserve(HEADERS_SIZE);

    size_t totalRead = 0;
    size_t chunkSize = 1; // leo 2 bytes por 2 bytes

    // Primera lectura para leer stausline y headers
    while (true) {
        char tempBuffer[chunkSize]; // Buffer pequeno 4byes temporal
        ssize_t bytesRead = recv(ClientFd, tempBuffer, chunkSize, 0);
        if (bytesRead == 0) {
            LOG_INFO("Client closed connection.");
            return false;
        }
        if (bytesRead < 0) {
            LOG_INFO("No more data available right now.");
            return false;
        }

        // Anadir buffer pequeno al grande
        buffer.append(tempBuffer, bytesRead);
        totalRead += bytesRead;

        // std::string print;
        // print += tempBuffer[0];
        // print +=  tempBuffer[1];
        // std::cout << print << "|" << std::endl;
        // hay un \r\n\r\n ? hemos llegado al final de los headers
        if (buffer.find("\r\n\r\n") != std::string::npos) {
            //std::cout << "Found CLRF at " << totalRead << " bytes read" << std::endl;
            break;
        }
        

        // Comprobar talla headers
        if (totalRead >= HEADERS_SIZE) {
            LOG_INFO("Headers exceed maximum allowed size");
            return false;
        }
    }
    //std::cout <<"Buffer \n" << std::endl;
    //std::cout << buffer << std::endl;
    //std::cout << "--------------" << std::endl;
    // std::cout << "-------------------------\n\n--------------------\n" << std::endl;
    // // Ponemos bien los headers por si
    // size_t endHeadersPos = buffer.find("\r\n\r\n");
    // std::string headers = buffer.substr(0, endHeadersPos + 4); // Include the CRLF
    //std::cout << headers << std::endl;


    // Parse the headers
    parseRequest(buffer);

    // Check Transfer-Encoding for chunked mode
    std::string transferEncoding = getHeader("Transfer-Encoding");
    if (transferEncoding == "chunked") {
        return readChunkedBody(ClientFd);
    } else {
        size_t contentLength = parseContentLength();
        // Handle Content-Length
        if (contentLength > maxSize)
            return false;

        return readContentLengthBody(ClientFd, contentLength);
    }
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
            //std::cout << "request headers added " << key << std::endl;
        }
        // else 
        // {
        //     //std::cout << "found line in headers i dont know: " << line << std::endl;
        // }
    }
}

bool    Request::readContentLengthBody(const size_t& ClientFd, size_t contentLength)
{
    // Leer body teniendo en cuenta el COntent-legnht como limite
    size_t totalBodyRead = 0;
    std::string tempBody(contentLength, '\0'); // Body buffer
    //std::cout << contentLength << std::endl;
    while (totalBodyRead < contentLength)
    {
        //std::cout << "Voy a leer " << contentLength - totalBodyRead << std::endl;
        if (contentLength - totalBodyRead < 2)
            break;
        ssize_t bytesRead = recv(ClientFd, &tempBody[totalBodyRead], contentLength - totalBodyRead, 0);
        //std::cout << "He leido " << bytesRead << " bytes" << std::endl;
        totalBodyRead += bytesRead;
        //std::cout << "En total llevo : " << totalBodyRead << "\n" <<std::endl;
        if (bytesRead < 0) {
            LOG_INFO("Error reading body data");
            return false;
        } else if (bytesRead == 0) {
            LOG_INFO("Connection closed by client before reading complete body");
            return false; 
        }
    }
    body += tempBody;
    //std::cout << "tamaño body TOTAL: " << body.size() << std::endl;
    return true;
}

size_t  Request::parseContentLength()
{
    
    // Parsear Content-Length
    size_t contentLength = 0;
    std::map<std::string, std::string>::iterator header_iter = this->headers.find("Content-Length");
    
    if (header_iter != this->headers.end())
    {
        std::string  contentLength_Str(header_iter->second);
        //std::cout << "Content-length-value: " << header_iter->second << std::endl;
        contentLength = static_cast<size_t>(atoi(contentLength_Str.c_str()));
    }
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
    size_t totalSize = 0;
    std::string buffer;
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
                //std::cout << "readChar: " << c <<"|"<< std::endl;

                chunkSizeStr += c;
                if (chunkSizeStr.length() >= 2 && 
                    chunkSizeStr[chunkSizeStr.length() - 2] == '\r' && 
                    chunkSizeStr[chunkSizeStr.length() - 1] == '\n')
                {
                    chunkSizeStr = chunkSizeStr.substr(0, chunkSizeStr.length());
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
            chunkSize += 2; // Para el \r\n al final contarlo
            buffer.resize(chunkSize);
            size_t totalBodyRead = 0;
            while (totalBodyRead < chunkSize)
            {
                //std::cout << "Voy a leer " << chunkSize - totalBodyRead << std::endl;
                if (chunkSize - totalBodyRead <= 1)
                    break;
                size_t bytesRead = recv(ClientFd, &buffer[totalBodyRead], chunkSize - totalBodyRead, 0);
                if (bytesRead <= 0)
                    throw RequestError("Error leyendo datos del chunk");
                if (buffer.find_first_of("\r\n") != std::string::npos)
                    buffer.erase(buffer.find_first_of("\r\n"));                
                //std::cout << "read of chunk; " << buffer << "|" << std::endl;
                totalBodyRead += bytesRead;
            }
            
            // 7. Añadir chunk al body
            totalSize += chunkSize;
            body.append(buffer);
        }
        return true;
    }
    catch (const RequestError& e) {
        LOG("Error en lectura chunked: " + std::string(e.what()));
        return false;
    }
}


ClientInfo& Request::getClientInfo() const
{
    return info;
}

int Request::getClientFd() const
{ 
    return info.pfd.fd;
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
