/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 23:59:59 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/15 01:09:23 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

#include <sstream>
#include <algorithm>
#include "../Logger/Logger.hpp"

Request::Request(const std::string& rawRequest)
{
    parse(rawRequest);
}

void Request::parse(const std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse request line
    if (std::getline(requestStream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> method >> uri >> httpVersion;
        if (!(lineStream >> method >> uri >> httpVersion))
            LOG("Error parsing request line: invalid format");
    } else
    {
        LOG("Error parsing request line");
        return;
    }

    // Parse headers
    while (std::getline(requestStream, line) && line != "\r")
    {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            headers[key] = value;
        }
    }

    // Parse body
    std::string bodyLine;
    while (std::getline(requestStream, bodyLine))
        body += bodyLine + "\n";
    if (!body.empty())
        body.pop_back(); // Remove last newline
}

std::string Request::getMethod() const
{
    return method;
}

std::string Request::getUri() const
{
    return uri;
}

std::string Request::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

std::string Request::getBody() const {
    return body;
}
