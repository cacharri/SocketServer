/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 18:44:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/12 02:18:04 by smagniny         ###   ########.fr       */
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


void GetHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    //std::cout << "Received GET request" << std::endl;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::string fullpath(cwd);
        std::string requested_resource = request->getUri();
        //std::cout << requested_resource << std::endl;
        
        fullpath += locationconfig.root;
        
        //std::cout << "Full path:: " << fullpath << std::endl;
        LOG_INFO(fullpath);
        if ((!locationconfig.cgi_pass.empty()))
        {
            CgiHandler cgi_handler_instance;
            cgi_handler_instance.handle(request, response, locationconfig);
            response->setStatusCode(201);
            LOG_INFO("CGI Resource");
            return ;
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
            
            if (access(fullpath.c_str(), F_OK) != 0) {
                response->setStatusCode(404);
                response->setBody("<html><body><h1>404 Not Found</h1></body></html>");
            }
            
            std::cout << fullpath.c_str() << std::endl;
            std::ifstream file(fullpath.c_str());
            if (!file.is_open())
            {
                response->setStatusCode(404); 
                response->setBody("<html><body><h1>404 Not found</h1></body></html>");
            }
            
            std::stringstream bufferStream;
            bufferStream << file.rdbuf();

            if (!(bufferStream.str().empty()))
            {
                response->setStatusCode(200);
                response->setBody(bufferStream.str());
                response->setHeader("Content-Type", response->getMimeType(fullpath));
                LOG_INFO("Read Resource Succesfully");
            }
        }
    }
    else {
        LOG_INFO("Failed current Directory Read");
        response->setStatusCode(500);
        response->setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
    }
}


