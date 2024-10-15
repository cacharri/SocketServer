/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 19:15:28 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/13 19:52:15 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Handlers.hpp"

PostHandler::PostHandler()
{
    
}

PostHandler::~PostHandler()
{
    
}

// Funcion para parsear el cuerpo de la solicitud MultiPart en pares clave valor
std::string PostHandler::parseMultipartData(const std::string& body, const std::string& boundary) {
    std::string delimiter = "--" + boundary;
    std::string Key_name;
    size_t start = 0;
    size_t  tmp = 0;
    size_t end = 0;

    while ((start = body.find(delimiter, end)) != std::string::npos) {
        end = body.find(delimiter, start + delimiter.length());
        std::string part = body.substr(start + delimiter.length(), end - start - delimiter.length() - 2); // Skip the trailing CRLF
        
        // Find the content disposition
        size_t contentDispositionPos = part.find("Content-Disposition:");
        if (contentDispositionPos != std::string::npos) {
            size_t namePos = part.find("name=\"", contentDispositionPos);
            if (namePos != std::string::npos)
            {
                size_t nameEnd = part.find("\"", namePos + 6);
                tmp += nameEnd;
                Key_name = part.substr(namePos + 6, nameEnd  - namePos - 6);
                std::cout << "found Content-Disposition header for " << (Key_name.empty() ? "\"\"":Key_name)<< std::endl;
                // Check if it's a file upload name == "fileUpload" ?
                if (Key_name == "fileUpload")
                {
                    size_t filenamePos = part.find("filename=\"", contentDispositionPos);
                    if (filenamePos != std::string::npos) {
                        // It's a file upload
                        size_t filenameEnd = part.find("\"", filenamePos + 10);
                        tmp += filenameEnd;
                        std::string filename = part.substr(filenamePos + 10, filenameEnd - filenamePos - 10);
                        // Find the content
                        size_t contentStart = part.find("\r\n\r\n", filenameEnd) + 10; // Skip the headers
                        std::string content = part.substr(contentStart, part.length() - contentStart - 2); // Skip the trailing CRLF
                        std::cout << filename << " has the following content; " << std::endl;
                        return content; // return the file content
                    }
                }
            }
        }
        else {
                // It's a regular form field
                size_t contentStart = part.find("\r\n\r\n", tmp); // Skip the headers
                std::string content = part.substr(contentStart, part.length() - contentStart - 2); // Skip the trailing CRLF
                return content; // return the form fields values
        }
    }

    return "No delimiter in body POST request";
}

// Función para parsear el cuerpo de la solicitud UrlEncoded en pares clave-valor
std::map<std::string, std::string> PostHandler::parseUrlEncodedData(const std::string& body)
{
    std::map<std::string, std::string> formData;
    std::stringstream ss(body);
    std::string pair;

    while (std::getline(ss, pair, '&')) {  // Divide las parejas clave=valor separadas por '&'
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            formData[key] = value;  // Guardamos el par clave-valor en el mapa
        }
    }

    return formData;
}

std::string                         PostHandler::urlDecode(const std::string &str) 
{
    std::string result;
    char ch;
    int i, ii;
    for (i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            result += ch;
            i = i + 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string                         PostHandler::escapeHtml(const std::string& data) {
    std::string result;
    for (size_t i = 0; i < data.length(); ++i) {
        unsigned char c = data[i];
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            // Acentos y caracteres especiales
            case 0xC3:
                if (i + 1 < data.length()) {
                    unsigned char nextChar = data[i + 1];
                    switch (nextChar) {
                        case 0xA1: result += "&aacute;"; break;  // á
                        case 0xA9: result += "&eacute;"; break;  // é
                        case 0xAD: result += "&iacute;"; break;  // í
                        case 0xB3: result += "&oacute;"; break;  // ó
                        case 0xBA: result += "&uacute;"; break;  // ú
                        case 0xB1: result += "&ntilde;"; break;  // ñ
                        case 0x91: result += "&Ntilde;"; break;  // Ñ
                        default: result += c; break;
                    }
                    i++;  // Salta el siguiente byte del carácter multibyte
                } else {
                    result += c;
                }
                break;
            default: result += c; break;
        }
    }
    return result;
}


void        PostHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig)
{   
    std::cout << "Received POST request" << std::endl;
    request.print();

    std::string contentType = request.getHeader("Content-Type");
    std::cout << "Content type is " << contentType << std::endl;

    if (contentType.find("multipart/form-data") != std::string::npos) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            std::string boundary = contentType.substr(boundaryPos + 9, ); // 9 is the length of "boundary="
            
            // Now we process the multipart data
            //std::cout << "BODY POST >>> " << requestBody << std::endl;
            if (request.getBody().empty())
                LOG("EMPTY BODY POST REQUEST");
            std::string formData = parseMultipartData(request.getBody(), boundary);

            response.setStatus(200, "OK");
            response.setBody(formData);
        } else {
            response.setStatus(400, "Bad Request");
            response.setBody("Boundary not found in Content-Type");
        }
    }
    else if (contentType == "application/x-www-form-urlencoded") {
        std::string requestBody = request.getBody();
        std::map<std::string, std::string> formData = parseUrlEncodedData(requestBody);

        // Process the form data as needed
        std::string name = urlDecode(formData["name"]);
        std::string email = urlDecode(formData["email"]);
        std::string age = urlDecode(formData["age"]);
        std::string gender = urlDecode(formData["gender"]);
        std::string comments = urlDecode(formData["comments"]);

        std::cout << "Decoded name: " << name << std::endl;

        // Generate response HTML
        std::string responseBody = "<html><body>";
        responseBody += "<h1>Formulario Recibido</h1>";
        responseBody += "<p>Nombre: " + escapeHtml(name) + "</p>";
        responseBody += "<p>Email: " + escapeHtml(email) + "</p>";
        responseBody += "<p>Edad: " + escapeHtml(age) + "</p>";
        responseBody += "<p>G&eacute;nero: " + escapeHtml(gender) + "</p>";
        responseBody += "<p>Comentarios: " + escapeHtml(comments) + "</p>";
        responseBody += "</body></html>";

        response.setStatus(200, "OK");
        response.setBody(responseBody);
    }
    else {
        response.setStatus(400, "Bad Request");
        response.setBody("Unsupported Content-Type");
    }
}


