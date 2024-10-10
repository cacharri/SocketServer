/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 21:28:04 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/10 18:02:42 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handlers.hpp"




GetHandler::GetHandler()
{
    
}

std::string     GetHandler::readFile(const std::string &fullPath)
{
    
    struct stat buffer;

    if (stat(fullPath.c_str(), &buffer) != 0) {
        return "Error: File does not exist.";
    }

    std::ifstream file(fullPath.c_str());
    
    if (!file.is_open()) {
        return "Error: Unable to open file: " + fullPath;
    }
    
    std::stringstream bufferStream;
    bufferStream << file.rdbuf();
    //std::cout << "File content: " << bufferStream.str() << std::endl;
    return bufferStream.str();
}


std::string     GetHandler::printCurrentWorkingDirectory()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        return (cwd);
    else
        return "";
}


void        GetHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig)
{
    std::cout << "Received GET request" << std::endl;

    std::string fullpath =  printCurrentWorkingDirectory();
    //std::cout << "abs path: " << fullpath << std::endl;
    fullpath = fullpath + locationconfig.root + "/" + locationconfig.index; // Construct full path
    //std::cout << "join with file: " << fullpath << std::endl;

    response.setBody(readFile(fullpath));
    response.setStatus(200, "OK");
}

PostHandler::PostHandler()
{
    
}

// Función para parsear el cuerpo de la solicitud en pares clave-valor
std::map<std::string, std::string> PostHandler::parseFormData(const std::string& body)
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
    std::string contentType_multipart;
    std::string boundary;
    
    if (contentType.find(";") != std::string::npos)
    {
        contentType_multipart = contentType.find("multipart/form-data");
        boundary = contentType.find("-");
    }
    if (contentType == "application/x-www-form-urlencoded")
    {
        std::string requestBody = request.getBody();

        std::map<std::string, std::string> formData = parseFormData(requestBody);

        // Decodificar campos del formulario
        std::string name = urlDecode(formData["name"]);
        std::string email = urlDecode(formData["email"]);
        std::string age = urlDecode(formData["age"]);
        std::string gender = urlDecode(formData["gender"]);
        std::string comments = urlDecode(formData["comments"]);
        
        std::cout << "Received request body: " << requestBody << std::endl;

        // Establecer el Content-Type en UTF-8 para la respuesta
        response.setHeader("Content-Type", "text/html; charset=UTF-8");

        std::cout << "Decoded name: " << name << std::endl;

        // Generar respuesta HTML
        std::string responseBody = "<html><body>";
        responseBody += "<h1>Formulario Recibido</h1>";
        responseBody += "<p>Nombre: " + escapeHtml(name) + "</p>";
        responseBody += "<p>Email: " + escapeHtml(email) + "</p>";
        responseBody += "<p>Edad: " + escapeHtml(age) + "</p>";
        responseBody += "<p>G&eacute;nero:: " + escapeHtml(gender) + "</p>";
        responseBody += "<p>Comentarios: " + escapeHtml(comments) + "</p>";

        responseBody += "</body></html>";

        response.setStatus(200, "OK");
        response.setBody(responseBody);
    } 
    else if ( contentType_multipart  == "multipart/form-data"){
        std::cout << "multipart/form-data encoding received" << std::endl;
    }
    else {
        response.setStatus(400, "Bad Request");
        response.setBody("Unsupported Content-Type");
    }
}