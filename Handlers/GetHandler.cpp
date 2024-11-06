/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 18:44:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 11:47:39 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handlers.hpp"
#include "../Router/Router.hpp"

GetHandler::GetHandler()
{
    
}

GetHandler::~GetHandler()
{
    
    
}

std::string GetHandler::readFile(const std::string &fullPath) {
    struct stat buffer;

    // Verificar si el archivo existe
    if (stat(fullPath.c_str(), &buffer) != 0) {
        std::cerr << "Error: El archivo no existe: " << fullPath << std::endl; // Log de error
        return ""; // Devuelve una cadena vacía si no existe
    }

    std::ifstream file(fullPath.c_str());

    // Verificar si se pudo abrir el archivo
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo: " << fullPath << std::endl; // Log de error
        return ""; // Devuelve una cadena vacía si no se puede abrir
    }

    std::stringstream bufferStream;
    bufferStream << file.rdbuf(); // Lee el contenido del archivo
    return bufferStream.str(); // Devuelve el contenido del archivo
}


/*void        GetHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig)
{
    std::cout << "Received GET request" << std::endl;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) //abs path from user
    {
        std::string fullpath(cwd);
        fullpath = fullpath + locationconfig.root + "/" + locationconfig.index; // Construct full path
        response->setBody(readFile(fullpath));
    }
    response->setStatus(200, "OK");
}*/

std::string getMimeType(const std::string& filename) {
    // Comprobar si el nombre del archivo termina en ".jpg", ".jpeg", ".png", ".gif"
    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0) {
        return "image/jpeg";
    } else if (filename.size() >= 5 && filename.compare(filename.size() - 5, 5, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0) {
        return "image/png";
    } else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".gif") == 0) {
        return "image/gif";
    } else if (filename.size() >= 5 && filename.compare(filename.size() - 5, 5, ".html") == 0) {
        return ("text/html");
    }
    //"application/octet-stream"
    return "text/html"; // Tipo por defecto
}

void GetHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    std::cout << "Received GET request" << std::endl;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::string fullpath(cwd);
        fullpath += locationconfig.root;
        
        if (!fullpath.empty() && fullpath[fullpath.size() - 1] != '/') {
            fullpath += '/';
        }
        fullpath += locationconfig.index;

        std::cout << "Full path:: " << fullpath << std::endl;
        if ((!locationconfig.cgi_pass.empty()))
        {
            CgiHandler cgi_handler_instance;
            cgi_handler_instance.handle(request, response, locationconfig);
         //   delete cgi_handler_instance;
            
        }/*
        if (!locationconfig.redirect.empty()) {
            response->setStatus(301, "Moved Permanently");
            response->setHeader("Location", locationconfig.redirect);
            response->setBody("<html><body><h1>301 Moved Permanently</h1></body></html>");
            return;
        }*/
        if (!locationconfig.redirect.empty()) {
            if (locationconfig.redirect_type == 301) {
                response->setStatus(301, "Moved Permanently");
            } else if (locationconfig.redirect_type == 302) {
                response->setStatus(302, "Found");
            } else {
                response->setStatus(400, "Bad Request");
                response->setBody("<html><body><h1>400 Bad Request</h1></body></html>");
                return;
            }
            response->setHeader("Location", locationconfig.redirect);
            std::ostringstream oss;
            oss << locationconfig.redirect_type;
            std::string body = "<html><body><h1>" + oss.str() + " " +
                            (locationconfig.redirect_type == 301 ? "Moved Permanently" : "Found") +
                            "</h1></body></html>";
            response->setBody(body);
            return;
        }
        else if (fullpath[fullpath.size() - 1] == '/')
        {
            struct stat buffer;
            if (stat(fullpath.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)) {
                if (locationconfig.autoindex) {
                    std::string fileContent = generateAutoIndex(fullpath);
                    response->setStatus(200, "OK");
                    response->setBody(fileContent);
                    response->setHeader("Content-Type", "text/html");
                } else {
                    response->setStatus(403, "Forbidden");
                    response->setBody("<html><body><h1>403 Forbidden</h1></body></html>");
                }
                return ;
            }
        }
        else{

            std::string fileContent = readFile(fullpath);
            if (!(fileContent.empty())) {
                response->setStatus(200, "OK");
                response->setBody(fileContent);
                response->setHeader("Content-Type", getMimeType(fullpath));
            }
        }
    }
    else {
        std::cerr << "Error al obtener el directorio actual" << std::endl;
        response->setStatus(500, "Internal Server Error");
        response->setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
    }
}


