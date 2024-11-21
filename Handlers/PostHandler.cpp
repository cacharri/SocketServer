/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 19:15:28 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/19 18:50:08 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Handlers.hpp"

PostHandler::PostHandler()
{
    
}

PostHandler::~PostHandler()
{
    
}

void PostHandler::handleMultipartFormData(const Request* request, Response* response, LocationConfig& locationconfig) {
    std::string contentType = request->getHeader("Content-Type");
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        response->setStatusCode(400);
        response->setBody(readFile("/var/www/error-pages/400.html"));
        return;
    }

    std::string boundary = contentType.substr(boundaryPos + 9); // Longitud de "boundary="
    std::string filename;
    std::string fileData = parseMultipartFormData(request->getBody(), boundary, locationconfig.upload_store, filename);

    if (fileData.empty()) {
        response->setStatusCode(400);
        response->setBody(readFile("/var/www/error-pages/400.html"));
        return;
    }

    if (!saveFile(filename, fileData)) {
        response->setStatusCode(500);  // Internal Server Error
        response->setBody(readFile("/var/www/error-pages/500.html"));
        return;
    }

    response->setStatusCode(201);  // Created
    response->setBody("<html><body><h1>File Uploaded Successfully!</h1></body></html>");
}

void PostHandler::handleUrlFormEncoded(const Request* request, Response* response, LocationConfig& locationconfig) {
    // Obtener el cuerpo de la solicitud
    std::string body = request->getBody();

    // Parsear el cuerpo como pares clave-valor
    std::map<std::string, std::string> formData = parseUrlFormData(body);

    // Si no hay datos, responde con un error
    if (formData.empty()) {
        response->setStatusCode(400); // Bad Request
        response->setBody(readFile("/var/www/error-pages/400.html"));
        return;
    }

    // Procesar datos específicos del formulario (ejemplo: almacenar en un archivo)
    if (formData.find("name") != formData.end() && formData.find("email") != formData.end()) {
        // Guardar los datos en una base de datos o archivo
        appendUsertoDatabase(formData, *response, locationconfig);
        return;
    }

    // Respuesta genérica para datos no procesados
    std::string responseBody = "<html><body>";
    responseBody += "<h1>Formulario Recibido</h1>";
    for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it) {
        responseBody += "<p>" + escapeHtml(it->first) + ": " + escapeHtml(it->second) + "</p>";
    }
    responseBody += "</body></html>";

    response->setStatusCode(200); // OK
    response->setBody(responseBody);
}


void PostHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig) {
    std::cout << "Received POST request" << std::endl;

    // Verificar si el cuerpo está vacío
    if (request->getBody().empty()) {
        LOG("EMPTY BODY POST REQUEST");
        response->setStatusCode(400);  // Bad Request
        response->setBody(readFile("/var/www/error-pages/400.html"));
        return;
    }

    // Verificar si el cuerpo excede el tamaño máximo permitido
    size_t maxBodySize = locationconfig.client_max_body_size;
    if (request->getBody().size() > maxBodySize) {
        LOG("Request body exceeds maximum allowed size");
        response->setStatusCode(413);  // Payload Too Large
        response->setBody(readFile("/var/www/error-pages/413.html"));
        return;
    }

    // Manejar multipart/form-data
    if (request->getHeader("Content-Type").find("multipart/form-data") != std::string::npos) {
        handleMultipartFormData(request, response, locationconfig);
        return;
    }

    // Manejar application/x-www-form-urlencoded
    if (request->getHeader("Content-Type") == "application/x-www-form-urlencoded") {
        handleUrlFormEncoded(request, response, locationconfig);
        return;
    }

    // Caso por defecto: Responder con el cuerpo recibido
    response->setStatusCode(200);
    response->setBody(request->getBody());
}


/*void        PostHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig)
{   
    std::cout << "Received POST request" << std::endl;
    //request->print();

    std::string contentType = request->getHeader("Content-Type");
    //std::cout << "Content type is " << contentType << std::endl;
    if (contentType.empty())
    {
        //std::cout << "yooo2" << std::endl;
        response->setStatusCode(200);
        std::cout << request->getBody() << std::endl;
        response->setBody(request->getBody());
    }
    if ((!locationconfig.cgi_pass.empty()))
    {
            CgiHandler cgi_handler_instance;
            cgi_handler_instance.handle(request, response, locationconfig);
            response->setStatusCode(201);
            return ;
    }
    else if (contentType.find("multipart/form-data") != std::string::npos) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            std::string boundary = contentType.substr(boundaryPos + 9); // 9 is the length of "boundary="
            //std::cout << "Content type boundary is " << boundary << std::endl;

            if (request->getBody().empty())
            {
                LOG("EMPTY BODY POST REQUEST");
                response->setStatusCode(204);
                response->setBody(readFile("/var/www/error-pages/204.html"));
                return;
            }
            std::string fileData;
            std::string filename;
            fileData = parseMultipartFormData(request->getBody(), boundary, locationconfig.upload_store, filename);
            std::string responseBody;

            // Check if fileData is empty
            if (request->getBody().empty())
            {
                response->setStatusCode(400);
                response->setBody(readFile("/var/www/error-pages/400.html"));
                return;
            }
            struct stat fileStat;
            if (stat((filename).c_str(), &fileStat) == 0) {
                LOG_INFO("FILE ALLREADY EXIST");
                std::string body = "<html><head><title>File Upload</title></head><body>";
                body += "<h2>File already exists.</h2>";
                body += "<p><a href='/upload'>Upload another file</a></p>";
                body += "<p><a href='/'>Go back to home</a></p>";
                body += "</body></html>";

                // Establecemos la respuesta
                response->setBody(body);
                response->setStatusCode(201);  // Respuesta OK
                return;
            }


            // Save the file and check for errors
            if (!saveFile(filename, fileData))
            {
                response->setStatusCode(500);
                response->setBody(readFile("/var/www/error-pages/500.html"));
                return;
            }
                response->setHeader("Content-Type", "text/html; charset=UTF-8");
                responseBody = "<html><body>";
                responseBody += "<h1>File Uploaded Successfully!</h1>";
                responseBody += "<p>Your file has been uploaded as: " + filename + "</p>";
                responseBody += "<script>localStorage.setItem('uploadedFileName', '" + filename + "');</script>";
                responseBody += "<p><a href='/upload'>Upload another file</a></p>";
                responseBody += "<p><a href='/'>Go back to home</a></p>";
                responseBody += "</body></html>";

                response->setStatusCode(201);
                response->setBody(responseBody);
        } else {
            response->setStatusCode(400);
        }
    }
    else if (contentType == "application/x-www-form-urlencoded") {
        std::map<std::string, std::string> formData = parseUrlFormData(request->getBody());
        
        // Si c'est une demande de suppression via formulaire
        if (request->getUri() == "/delete" && !(urlDecode(formData["archivo"]).empty()) && !(urlDecode(formData["boton"]).empty()) ) {
            DeleteHandler   deleteHandler;
            std::string     filepath(urlDecode(formData["archivo"]));
            
            if (locationconfig.root[locationconfig.root.length()-1] != '/' && filepath[0] != '/')
                locationconfig.root += std::string("/");
            else if (locationconfig.root[locationconfig.root.length()-1] == '/' && filepath[0] == '/')
                locationconfig.root.resize(locationconfig.root.size() - 1);
                
            locationconfig.root += filepath;
            deleteHandler.remove_file_or_dir(response, locationconfig);
            return;
            
        }
        else
            appendUsertoDatabase(formData, *response, locationconfig);
    }
    else
    {
        response->setStatusCode(200);
        std::cout << request->getBody() << std::endl;
        response->setBody(request->getBody());
    }

   
}*/



std::string     PostHandler::parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store, std::string& filename) {
    std::string fileData;
    std::string partDelimiter = "--" + boundary;
    size_t start = 0;

    while ((start = data.find(partDelimiter, start)) != std::string::npos) {
        start += partDelimiter.length();
        size_t end = data.find(partDelimiter, start);
        
        if (end == std::string::npos) {
            break; // No more parts
        }
        
        std::string part = data.substr(start, end - start);
        size_t namePos = part.find("name=\"");
        size_t filenamePos = part.find("filename=\"");
        
        if (namePos != std::string::npos) {
            namePos += 6; // Length of "name=\""
            size_t nameEnd = part.find("\"", namePos);
            std::string name = part.substr(namePos, nameEnd - namePos);
            
            if (filenamePos != std::string::npos) {
                // It's a file field
                filenamePos += 10; // Length of "filename=\""
                size_t filenameEnd = part.find("\"", filenamePos);
                filename = post_upload_store;
                if (filename[filename.size() - 1] == '/')
                    filename += part.substr(filenamePos, filenameEnd - filenamePos);
                else
                    filename += "/" + part.substr(filenamePos, filenameEnd - filenamePos);

                size_t valuePos = part.find("\r\n\r\n", filenameEnd) + 4; // Skip headers
                fileData = part.substr(valuePos, part.size() - valuePos - 2); // Without the last two characters
            } else {
                // Normal form field
                size_t valuePos = part.find("\r\n\r\n", nameEnd) + 4; // Skip headers
                std::string value = part.substr(valuePos, part.size() - valuePos - 2); // Without the last two characters
                // You can handle normal fields here if needed
            }
        }        
        start = end;
    }
    return fileData;
    // Return fileData through the reference parameter if needed
}

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


void    PostHandler::appendUsertoDatabase(std::map<std::string, std::string>& formData, Response& response, const LocationConfig& locationconfig)
{
    //Process the form data as needed
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
            response.setStatusCode(500);
            response.setBody("Failed to open form_data.csv");
            return;
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
}

bool PostHandler::saveFile(const std::string& filename, const std::string& data)
{
    std::ofstream outFile(filename.c_str(), std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(data.c_str(), data.size());
        outFile.close();
        return true; 
    } else {
        std::cerr << "Error: " << filename << std::endl;
        return false; 
    }
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




