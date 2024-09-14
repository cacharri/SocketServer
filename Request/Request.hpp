/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Smagniny <santi.mag777@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 22:28:40 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/14 22:29:38 by Smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
private:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    Request(const std::string& rawRequest);
    void parse(const std::string& rawRequest);
    
    // Getters
    std::string getMethod() const;
    std::string getUri() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
};
#endif