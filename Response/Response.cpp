/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 00:30:32 by Smagniny          #+#    #+#             */
/*   Updated: 2024/11/27 17:39:15 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

void Response::initStatusCode(void)
{
    _statusCodesMap[100] = "Continue";
    _statusCodesMap[101] = "Switching Protocols";
    _statusCodesMap[103] = "Processing request";
    _statusCodesMap[200] = "OK";
    _statusCodesMap[201] = "Created";
    _statusCodesMap[202] = "Accepted";
    _statusCodesMap[203] = "Non-Authoritative Information";
    _statusCodesMap[204] = "No Content";
    _statusCodesMap[205] = "Reset Content";
    _statusCodesMap[206] = "Partial Content";
    _statusCodesMap[300] = "Multiple Choices";
    _statusCodesMap[301] = "Moved Permanently";
    _statusCodesMap[302] = "Found";
    _statusCodesMap[303] = "See Other";
    _statusCodesMap[304] = "Use Proxy";
    _statusCodesMap[305] = "Not Modified";
    _statusCodesMap[307] = "Temporary Redirect";
    _statusCodesMap[400] = "Bad Request";
    _statusCodesMap[401] = "Unauthorized";
    _statusCodesMap[402] = "Payment Required";
    _statusCodesMap[403] = "Forbidden";
    _statusCodesMap[404] = "Not Found";
    _statusCodesMap[405] = "Method Not Allowed";
    _statusCodesMap[406] = "Not Acceptable";
    _statusCodesMap[407] = "Proxy Authentication Required";
    _statusCodesMap[408] = "Request Time-out";
    _statusCodesMap[409] = "Conflict";
    _statusCodesMap[410] = "Gone";
    _statusCodesMap[411] = "Length Required";
    _statusCodesMap[412] = "Precondition Failed";
    _statusCodesMap[413] = "Request Entity Too Large";
    _statusCodesMap[414] = "Request-URI Too Large";
    _statusCodesMap[415] = "Unsupported Media Types";
    _statusCodesMap[416] = "Requested range not satisfiable";
    _statusCodesMap[417] = "Expectation failed";
    _statusCodesMap[500] = "Internal Server Error";
    _statusCodesMap[501] = "Not Implemented";
    _statusCodesMap[502] = "Bad Gateway";
    _statusCodesMap[503] = "Service Unavailable";
    _statusCodesMap[504] = "Gateway Time-out";
    _statusCodesMap[505] = "HTTP Version not supported";
}

void    Response::initMimesTypes(void)
{
    _mimeTypesMap[""] = "text/plain";
    _mimeTypesMap[".aac"] = "audio/aac";
    _mimeTypesMap[".abw"] = "application/x-abiword";
    _mimeTypesMap[".arc"] = "application/x-freearc";
    _mimeTypesMap[".avif"] = "image/avif";
    _mimeTypesMap[".avi"] = "video/x-msvideo";
    _mimeTypesMap[".azw"] = "application/vnd.amazon.ebook";
    _mimeTypesMap[".bin"] = "application/octet-stream";
    _mimeTypesMap[".bmp"] = "image/bmp";
    _mimeTypesMap[".bz"] = "application/x-bzip";
    _mimeTypesMap[".bz2"] = "application/x-bzip2";
    _mimeTypesMap[".cda"] = "application/x-cdf";
    _mimeTypesMap[".csh"] = "application/x-csh";
    _mimeTypesMap[".css"] = "text/css";
    _mimeTypesMap[".csv"] = "text/csv";
    _mimeTypesMap[".doc"] = "application/msword";
    _mimeTypesMap[".docx"] =
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    _mimeTypesMap[".eot"] = "application/vnd.ms-fontobject";
    _mimeTypesMap[".epub"] = "application/epub+zip";
    _mimeTypesMap[".gz"] = "application/gzip";
    _mimeTypesMap[".gif"] = "image/gif";
    _mimeTypesMap[".htm"] = "text/html";
    _mimeTypesMap[".html"] = "text/html";
    _mimeTypesMap[".ico"] = "image/vnd.microsoft.icon";
    _mimeTypesMap[".ics"] = "text/calendar";
    _mimeTypesMap[".jar"] = "application/java-archive";
    _mimeTypesMap[".jpeg"] = "image/jpeg";
    _mimeTypesMap[".jpg"] = "image/jpeg";
    _mimeTypesMap[".js"] = "text/javascript";
    _mimeTypesMap[".json"] = "application/json";
    _mimeTypesMap[".jsonld"] = "application/ld+json";
    _mimeTypesMap[".mid"] = "audio/midi";
    _mimeTypesMap[".midi"] = "audio/midi";
    _mimeTypesMap[".mjs"] = "text/javascript";
    _mimeTypesMap[".mp3"] = "audio/mpeg";
    _mimeTypesMap[".mp4"] = "video/mp4";
    _mimeTypesMap[".mpeg"] = "video/mpeg";
    _mimeTypesMap[".mpkg"] = "application/vnd.apple.installer+xml";
    _mimeTypesMap[".odp"] = "application/vnd.oasis.opendocument.presentation";
    _mimeTypesMap[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    _mimeTypesMap[".odt"] = "application/vnd.oasis.opendocument.text";
    _mimeTypesMap[".oga"] = "audio/ogg";
    _mimeTypesMap[".ogv"] = "video/ogg";
    _mimeTypesMap[".ogx"] = "application/ogg";
    _mimeTypesMap[".opus"] = "audio/opus";
    _mimeTypesMap[".otf"] = "font/otf";
    _mimeTypesMap[".png"] = "image/png";
    _mimeTypesMap[".pdf"] = "application/pdf";
    _mimeTypesMap[".php"] = "application/x-httpd-php";
    _mimeTypesMap[".ppt"] = "application/vnd.ms-powerpoint";
    _mimeTypesMap[".pptx"] =
        "application/"
        "vnd.openxmlformats-officedocument.presentationml.presentation";
    _mimeTypesMap[".rar"] = "application/vnd.rar";
    _mimeTypesMap[".rtf"] = "application/rtf";
    _mimeTypesMap[".sh"] = "application/x-sh";
    _mimeTypesMap[".svg"] = "image/svg+xml";
    _mimeTypesMap[".tar"] = "application/x-tar";
    _mimeTypesMap[".tif"] = "image/tiff";
    _mimeTypesMap[".tiff"] = "image/tiff";
    _mimeTypesMap[".ts"] = "video/mp2t";
    _mimeTypesMap[".ttf"] = "font/ttf";
    _mimeTypesMap[".txt"] = "text/plain";
    _mimeTypesMap[".vsd"] = "application/vnd.visio";
    _mimeTypesMap[".wav"] = "audio/wav";
    _mimeTypesMap[".weba"] = "audio/webm";
    _mimeTypesMap[".webm"] = "video/webm";
    _mimeTypesMap[".webp"] = "image/webp";
    _mimeTypesMap[".woff"] = "font/woff";
    _mimeTypesMap[".woff2"] = "font/woff2";
    _mimeTypesMap[".xhtml"] = "application/xhtml+xml";
    _mimeTypesMap[".xls"] = "application/vnd.ms-excel";
    _mimeTypesMap[".xlsx"] =
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mimeTypesMap[".xml"] = "application/xml";
    _mimeTypesMap[".xul"] = "application/vnd.mozilla.xul+xml";
    _mimeTypesMap[".zip"] = "application/zip";
    _mimeTypesMap[".3gp"] = "video/3gpp";
    _mimeTypesMap[".3g2"] = "video/3gpp2";
    _mimeTypesMap[".7z"] = "application/x-7z-compressed";
}

std::string Response::getMimeType(const std::string& filename)
{
    // Comprobar la extension del archivo y meter en el contentType con el mapa de mimes types.
    size_t          position_extension = filename.rfind(".");

    std::string     extension = filename.substr(position_extension, filename.size());
    
    return _mimeTypesMap[extension];
}

void Response::setStatusCode(int code)
{
    std::map<int,std::string>::iterator it = _statusCodesMap.find(code);
    if (it != _statusCodesMap.end())
    {
        statusCode = code;
        statusCodeMessage = it->second;
    }
}
Response::Response()
{
    initStatusCode();
    initMimesTypes();
}


void Response::setHeader(const std::string& key, const std::string& value)
{
    headers[key] = value;
}

void Response::setBody(const std::string& content)
{
    body = content;
    setContentLength();
}

void Response::setContentType(const std::string& type)
{
    setHeader("Content-Type", type);
}

void Response::setContentLength()
{
    std::ostringstream oss;
    oss << body.length(); 
    std::string value = oss.str();
    setHeader("Content-Length", value);
}

std::string Response::toString() const
{
    std::ostringstream res;
    res << "HTTP/1.1 " << statusCode << " "<< statusCodeMessage << "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        res << it->first << ": " << it->second << "\r\n";
    }
    
    res << "\r\n" << body;
    return res.str();
}


// Getters implementation
int Response::getStatusCode() const {
    return statusCode;
}

std::string Response::getStatusMessage() const {
    return statusCodeMessage;
}

std::string Response::getBody() const {
    return body;
}

std::string Response::getHeaders(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()){
        return it->second; // Key found, return the value
    }
    return "";
}

