/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 21:28:04 by smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 22:36:59 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handlers.hpp"


#include <sys/stat.h> // For file existence check
std::string readFile(const std::string &fullPath)
{

    // Check if the file exists
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
    std::cout << "File content: " << bufferStream.str() << std::endl;
    return bufferStream.str();
}


void    setBodyWithFile(const std::string& filestr, Response& response)
{
    std::cout << "Construyendo body a partir de " << filestr << std::endl;
    response.setStatus(200, "OK");
    
    std::cout << readFile(filestr) << std::endl;
    
    response.setBody(readFile(filestr));
}


// void handleHome(const Request& request, Response& response) {
//     response.setStatus(200);
//     response.setBody("<html><body><h1>Welcome to the Home Page</h1></body></html>");
// }

// void handleAbout(const Request& request, Response& response) {
//     response.setStatus(200);
//     response.setBody("<html><body><h1>About Us</h1></body></html>");
// }