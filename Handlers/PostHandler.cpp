/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 19:15:28 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 12:03:04 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Handlers.hpp"

PostHandler::PostHandler()
{
    
}

PostHandler::~PostHandler()
{
    
}

void        PostHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig)
{   
    std::cout << "Received POST request" << std::endl;
    request->print();

    std::string contentType = request->getHeader("Content-Type");
    std::cout << "Content type is " << contentType << std::endl;
    if ((!locationconfig.cgi_pass.empty()))
        {
           CgiHandler cgi_handler_instance;
            cgi_handler_instance.handle(request, response, locationconfig);
          //  delete cgi_handler_instance;
        }

    else if (contentType.find("multipart/form-data") != std::string::npos) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            std::string boundary = contentType.substr(boundaryPos + 9); // 9 is the length of "boundary="
            std::cout << "Content type boundary is " << boundary << std::endl;

            // Now we process the multipart data
            //std::cout << "BODY POST >>> " << requestBody << std::endl;
            if (request->getBody().empty())
                LOG("EMPTY BODY POST REQUEST");
    
            std::map<std::string, std::string> formData = parseMultipartFormData(request->getBody(), boundary, locationconfig.upload_store);

            // Generar respuesta HTML
            response->setHeader("Content-Type", "text/html; charset=UTF-8");
            std::string responseBody = "<html><body><h1>Archivo Subido correctamente</h1>";
            for (std::map<std::string, std::string>::iterator it = formData.begin(); it != formData.end(); ++it) {
                    responseBody += "<p>" + escapeHtml(it->second) + "</p>";
            }
            responseBody += "</body></html>";


            response->setStatus(200, "OK");
            response->setBody(responseBody);
        } else {
            response->setStatus(400, "Bad Request");
            response->setBody("Boundary not found in Content-Type");
        }
    }
    else if (contentType == "application/x-www-form-urlencoded") {
        std::string requestBody = request->getBody();
        std::map<std::string, std::string> formData = parseUrlFormData(requestBody);

        // Process the form data as needed
        std::string name = urlDecode(formData["name"]);
        std::string email = urlDecode(formData["email"]);
        std::string age = urlDecode(formData["age"]);
        std::string gender = urlDecode(formData["gender"]);
        std::string comments = urlDecode(formData["comments"]);

        std::cout << "Decoded name: " << name << std::endl;

        std::string upload_path = ((locationconfig.upload_store[locationconfig.upload_store.size() - 1] == '/') 
                            ? locationconfig.upload_store + "form_inputs_database.csv" 
                            : locationconfig.upload_store + "/form_inputs_database.csv");
                            
        std::ofstream csvFile(upload_path.c_str(), std::ios::app);
        if (csvFile.is_open())
        {
            csvFile << "\"" << name << "\",\"" << email << "\",\"" << age << "\",\"" << gender << "\",\"" << comments << "\"" << std::endl;
            csvFile.close();
            std::cout << "Datos guardados en " << upload_path << std::endl;
        }
        else
        {
            std::cerr << "No se pudo abrir el archivo form_data.csv" << std::endl;
        }

        // Generate response HTML
        std::string responseBody = "<html><body>";
        responseBody += "<h1>Formulario Recibido</h1>";
        responseBody += "<p>Nombre: " + escapeHtml(name) + "</p>";
        responseBody += "<p>Email: " + escapeHtml(email) + "</p>";
        responseBody += "<p>Edad: " + escapeHtml(age) + "</p>";
        responseBody += "<p>G&eacute;nero: " + escapeHtml(gender) + "</p>";
        responseBody += "<p>Comentarios: " + escapeHtml(comments) + "</p>";
        responseBody += "</body></html>";

        response->setStatus(200, "OK");
        response->setBody(responseBody);
    }
    else {
        response->setStatus(400, "Bad Request");
        response->setBody("Unsupported Content-Type");
    }
}

// Nueva función para manejar multipart/form-data
void PostHandler::saveFile(const std::string& filename, const std::string& data) {
    std::ofstream outFile(filename.c_str(), std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(data.c_str(), data.size());
        outFile.close();
    } else {
        std::cerr << "Error: " << filename << std::endl;
    }
}

std::map<std::string, std::string> PostHandler::parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store) {
    std::map<std::string, std::string>	formData;
    std::string                         partDelimiter = "--" + boundary;
    std::string 						fileData;
    size_t 								start = 0;
    
    while ((start = data.find(partDelimiter, start)) != std::string::npos) {
        start += partDelimiter.length();
        size_t end = data.find(partDelimiter, start);
        
        if (end == std::string::npos) {
            break; // No hay más partes
        }
        
        std::string part = data.substr(start, end - start);
        size_t namePos = part.find("name=\"");
        size_t filenamePos = part.find("filename=\"");
        
        if (namePos != std::string::npos) {
            namePos += 6; // Longitud de "name=\""
            size_t nameEnd = part.find("\"", namePos);
            std::string name = part.substr(namePos, nameEnd - namePos);
            
            if (filenamePos != std::string::npos) {
                
                // Es un campo de archivo
                filenamePos += 10; // Longitud de "filename=\""
                size_t filenameEnd = part.find("\"", filenamePos);
                std::string filename(post_upload_store);
                if (filename[filename.size() - 1] == '/')
                    filename += part.substr(filenamePos, filenameEnd - filenamePos);
                else
                    filename += "/" + part.substr(filenamePos, filenameEnd - filenamePos);

                size_t valuePos = part.find("\r\n\r\n", filenameEnd) + 4; // Salta los encabezados
                fileData = part.substr(valuePos, part.size() - valuePos - 2); // Sin los últimos dos caracteres
                formData[name] = fileData;
				saveFile(filename, fileData);
                std::cout << "Archivo subido: " << filename << std::endl;
            } else {
                // Campo de formulario normal
                size_t valuePos = part.find("\r\n\r\n", nameEnd) + 4; // Salta los encabezados
                std::string value = part.substr(valuePos, part.size() - valuePos - 2); // Sin los últimos dos caracteres
                formData[name] = value;
            }
        }        
        start = end;
    }
    return formData;
}

// Función para parsear el cuerpo de la solicitud UrlEncoded en pares clave-valor
std::map<std::string, std::string> PostHandler::parseUrlFormData(const std::string& body)
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




