/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:29:58 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/15 01:03:57 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sstream>
#include <map>

class Response {
private:
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    Response(int code = 200);
    void setStatus(int code);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& content);
    std::string toString() const;
};
#endif