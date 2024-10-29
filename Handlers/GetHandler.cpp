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
        response.setBody(readFile(fullpath));
    }
    response.setStatus(200, "OK");
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

void GetHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig) {
    std::cout << "Received GET request" << std::endl;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::string fullpath(cwd);
        fullpath += locationconfig.root;

        if (!fullpath.empty() && fullpath[fullpath.size() - 1] != '/') {
            fullpath += '/';
        }
        fullpath += locationconfig.index;

        std::cout << "Full path: " << fullpath << std::endl;
        if (fullpath[fullpath.size() - 1] == '/')
        {
            struct stat buffer;
            if (stat(fullpath.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)) {
                if (locationconfig.autoindex) { // Asegúrate de que esta verificación es correcta
                    std::string fileContent = generateAutoIndex(fullpath);
                    response.setStatus(200, "OK");
                    response.setBody(fileContent);
                    response.setHeader("Content-Type", "text/html");
                } else {
                    response.setStatus(403, "Forbidden");
                    response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
                }
                return ;
            }
        }
        std::string fileContent = readFile(fullpath);
        if (!(fileContent.empty())) {
            response.setStatus(200, "OK");
            response.setBody(fileContent);
            response.setHeader("Content-Type", getMimeType(fullpath));
        }
    }
    else {
        std::cerr << "Error al obtener el directorio actual" << std::endl;
        response.setStatus(500, "Internal Server Error");
        response.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
    }
}
