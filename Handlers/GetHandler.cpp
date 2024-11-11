/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 18:44:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/11 14:43:32 by smagniny         ###   ########.fr       */
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


std::string GetHandler::readFile(const std::string &fullPath)
{
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
    std::cout << bufferStream.str();
    return bufferStream.str(); // Devuelve el contenido del archivo
}


void GetHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    //std::cout << "Received GET request" << std::endl;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::string fullpath(cwd);
        fullpath += locationconfig.root;
        
        //std::cout << "Full path:: " << fullpath << std::endl;
        LOG_INFO(fullpath);
        if ((!locationconfig.cgi_pass.empty()))
        {
            CgiHandler cgi_handler_instance;
            cgi_handler_instance.handle(request, response, locationconfig);
            LOG_INFO("CGI Resource");
        }
        if (!locationconfig.redirect.empty()) {
            if (locationconfig.redirect_type == 301) {
                LOG_INFO("Redirected Permanently");
                response->setStatusCode(301);
            } else if (locationconfig.redirect_type == 302) {
                LOG_INFO("Redirected Temporaly");
                response->setStatusCode(302);
            } else {
                response->setStatusCode(400);
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
                    response->setStatusCode(200);
                    response->setBody(fileContent);
                    response->setHeader("Content-Type", "text/html");
                    LOG_INFO("AUTOINDEX resource");
                } else {
                    LOG_INFO("Forbidden resource");
                    response->setStatusCode(403);
                    response->setBody("<html><body><h1>403 Forbidden</h1></body></html>");
                }
                return ;
            }
        }
        else{
            std::string fileContent = readFile(fullpath);
            if (!(fileContent.empty()))
            {
                response->setStatusCode(200);
                response->setBody(fileContent);
                response->setHeader("Content-Type", response->getMimeType(fullpath));
            }
            LOG_INFO("Read Resource Succesfully");
        }
    }
    else {
        LOG_INFO("Failed current Directory Read");
        response->setStatusCode(500);
        response->setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
    }
}


