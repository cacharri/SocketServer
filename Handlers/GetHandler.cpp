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
    }
    // Agrega más tipos de contenido según sea necesario
    return "application/octet-stream"; // Tipo por defecto
} // esto es para que se pudieran vver las imagenes de los memes si te metes en el navegador en la pagina que esta el meme1 por ej. pero no la esta convirtiendo


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

        std::string fileContent = readFile(fullpath);
        if (fileContent.empty()) {
            struct stat buffer;
            if (stat(fullpath.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)) {
                fileContent = generateAutoIndex(fullpath);
                response.setStatus(200, "OK");
                response.setBody(fileContent);
                response.setHeader("Content-Type", "text/html");
            } else {
                std::cerr << "Error: No se pudo leer el archivo en " << fullpath << std::endl;
                response.setStatus(404, "Not Found");
                response.setBody("<html><body><h1>404 Not Found</h1></body></html>");
            }
        } else {
            // Determinar el tipo de contenido del archivo
            std::string mimeType = getMimeType(fullpath);
            response.setStatus(200, "OK");
            response.setBody(fileContent);
            response.setHeader("Content-Type", mimeType);
        }
    } else {
        std::cerr << "Error al obtener el directorio actual" << std::endl;
        response.setStatus(500, "Internal Server Error");
        response.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
    }
}
