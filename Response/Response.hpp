/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:29:58 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/26 18:15:10 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sstream>
#include <map>

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

public:
    Response();
    void    initStatusCode(void);
    void    initMimesTypes(void);

    std::string getMimeType(const std::string& filename);
    void setStatusCode(int code);
    void setContentLength();
    void setContentType(const std::string& type);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    std::string toString() const;

    // Getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::string getBody() const;
    std::string getHeaders(const std::string& key) const;
    
    template <typename T>
    T getHeaderAs(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(key);
        if (it == headers.end()) {
            throw std::runtime_error("Key not found: " + key);
        }

        T value;
        std::istringstream iss(it->second);
        if (!(iss >> value)) {
            return ;
        }
        return value;
    }
};

#endif