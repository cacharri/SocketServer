/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 20:09:58 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/22 14:35:41 by smagniny         ###   ########.fr       */
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
    virtual void handle(const Request* request, Response* response, LocationConfig& locationconfig) = 0;
};

class GetHandler : public RequestHandler
{
    public:
        GetHandler();
        ~GetHandler();
        
        virtual void        handle(const Request* request, Response* response, LocationConfig& locationconfig);        
};

class PostHandler : public RequestHandler 
{
    public:
        PostHandler();
        ~PostHandler();
        virtual void        handle(const Request* request, Response* response, LocationConfig& locationconfig);
    private:
            bool                                saveFile(const std::string& filename, const std::string& data);
            std::string                         urlDecode(const std::string &str);
            std::string                         escapeHtml(const std::string& data);
            std::map<std::string, std::string>  parseUrlFormData(const std::string& body);
            std::string                         parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store, std::string& filename);
            void                                appendUsertoDatabase(std::map<std::string, std::string>& formData, Response& response, const LocationConfig& locationconfig);
            void                                deleteFilefromDatabase(std::map<std::string, std::string>& formData, Response& response, const LocationConfig& locationconfig);
};

class CgiHandler : public RequestHandler 
{
public:
    CgiHandler();
    virtual ~CgiHandler();
    virtual void handle(const Request* request, Response* response, LocationConfig& locationconfig);
    
private:
    std::string executeCgi(const std::string& scriptPath, const std::map<std::string, std::string>& env, const std::string& inputData );
};


class DeleteHandler : public RequestHandler {
public:
    DeleteHandler();
    virtual ~DeleteHandler();
    virtual void    handle(const Request* request, Response* response, LocationConfig& locationconfig);
    static  void    remove_file_or_dir(Response* response, const LocationConfig& locationconfig);

private:
    // bool validatePath(const std::string& path, const std::string& root) const;
    // bool deleteResource(const std::string& path, Response* response);
    // std::string constructResponseHtml(const std::string& filename, bool success);
}; 

std::string                         urlDecode(const std::string &str);
std::string                         escapeHtml(const std::string& data);
std::string                         readFile(const std::string& path);

#endif
