/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 21:28:04 by smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 20:34:04 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handlers.hpp"

std::string readFile(const std::string &filePath) {
    std::ifstream file(filePath.c_str());
    
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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