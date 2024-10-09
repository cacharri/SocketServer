/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 20:09:58 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/09 13:11:45 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <fstream>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <limits.h>



#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../Config/ConfigParser.hpp"

std::string readFile(const std::string &filePath);

class RequestHandler {
    public:
        virtual void handle(const Request& request, Response& response, const LocationConfig& locationconfig) = 0;
        virtual ~RequestHandler() {}
        
};

class GetHandler : public RequestHandler
{
    public:
        GetHandler();
        void        handle(const Request& request, Response& response, const LocationConfig& locationconfig);
    private:
        std::string     printCurrentWorkingDirectory();
        std::string     readFile(const std::string &fullPath);
    
        
};

class PostHandler : public RequestHandler 
{
    public:
        PostHandler();
        void        handle(const Request& request, Response& response, const LocationConfig& locationconfig);
    private:
            std::string                         urlDecode(const std::string &str);
            std::string                         escapeHtml(const std::string& data);
            std::map<std::string, std::string>  parseFormData(const std::string& body);

};



// void        handleHome(const Request& request, Response& response);
// void        handleAbout(const Request& request, Response& response);



#endif
