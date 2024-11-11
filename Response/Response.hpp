/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:29:58 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/11 13:27:07 by smagniny         ###   ########.fr       */
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
};

#endif