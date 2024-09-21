/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:29:58 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 18:59:57 by smagniny         ###   ########.fr       */
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
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    Response(int code = 200);
    void setStatus(int code, const std::string& message);
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