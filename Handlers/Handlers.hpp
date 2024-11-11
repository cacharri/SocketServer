/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 20:09:58 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/11 13:17:56 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLERS_HPP
#define HANDLERS_HPP

// Forward declarations
class Request;
class Response;
class LocationConfig;

#include <fstream>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>


#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../Config/ConfigParser.hpp"

class RequestHandler {
public:
    virtual ~RequestHandler() {}
    virtual void handle(const Request* request, Response* response, const LocationConfig& locationconfig) = 0;
};

class GetHandler : public RequestHandler
{
    public:
        GetHandler();
        ~GetHandler();
        
        virtual void        handle(const Request* request, Response* response, const LocationConfig& locationconfig);
    private:
        std::string     readFile(const std::string &fullPath);
        
};

class PostHandler : public RequestHandler 
{
    public:
        PostHandler();
        ~PostHandler();
        virtual void        handle(const Request* request, Response* response, const LocationConfig& locationconfig);
    private:
            void                                saveFile(const std::string& filename, const std::string& data);
            std::string                         urlDecode(const std::string &str);
            std::string                         escapeHtml(const std::string& data);
            std::map<std::string, std::string>  parseUrlFormData(const std::string& body);
            std::map<std::string, std::string>  parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store);
};

class CgiHandler : public RequestHandler 
{
public:
    CgiHandler();
    virtual ~CgiHandler();
    virtual void handle(const Request* request, Response* response, const LocationConfig& locationconfig);
    
private:
    std::string executeCgi(const std::string& scriptPath, const std::map<std::string, std::string>& env, const std::string& inputData );
};


// void        handleHome(const Request& request, Response& response);
// void        handleAbout(const Request& request, Response& response);



#endif
