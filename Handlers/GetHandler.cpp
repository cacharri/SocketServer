/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 18:44:29 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/13 11:37:40 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handlers.hpp"

GetHandler::GetHandler()
{
    
}

GetHandler::~GetHandler()
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


void        GetHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig)
{
    std::cout << "Received GET request" << std::endl;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) //abs path from user
    {
        std::string fullpath(cwd);
        fullpath = fullpath + locationconfig.root + "/" + locationconfig.index; // Construct full path
        response.setBody(readFile(fullpath));
    }
    response.setStatus(200, "OK");
}
