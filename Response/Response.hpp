/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:29:58 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/28 15:34:05 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../Handlers/CgiProcess.hpp"
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>

class Response
{
private:
    std::string                         statusline;
    int                                 statusCode;
    std::string                         statusCodeMessage;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    std::string                         allData;
    std::string	                        resource_extension;
    std::map<int, std::string>          _statusCodesMap;
    std::map<int, std::string>          _errorPageFilesMap;
    std::map<std::string, std::string>  _mimeTypesMap;
    struct CgiProcess*                  cgi_process;
    

public:
    Response();
    ~Response();
    void    initStatusCode(void);
    void    initMimesTypes(void);

    std::string getMimeType(const std::string& filename);
    void setStatusCode(int code);
    void setContentLength();
    void setContentType(const std::string& type);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    void setCgiProcess(CgiProcess* process);
    std::string toString() const;

    // Getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::string getBody() const;
    std::string getHeaders(const std::string& key) const;
    CgiProcess* getCgiProcess() const;
    
    template <typename T>
    T getHeaderAs(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(key);
        if (it == headers.end()) {
            throw std::exception();
        }

        T value;
        std::istringstream iss(it->second);
        if (!(iss >> value)) {
            throw std::exception();
        }
        return value;
    }

};

#endif