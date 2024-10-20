/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderAnalyzer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 16:50:39 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/20 23:14:03 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderAnalyzer.hpp"

void     analyzeBasicHeaders(const Request& request, Response& response, const ServerConfig& config)
{   

    // HOST 
    std::cout << "\n\t\tHeader Basic Analyzer" << std::endl;
    std::string host(request.getHeader("Host"));

    if (host.empty() || isValidHostHeader(host) == false)
        return ;
    // else{ add logic to only accept to our own servers }
    std::cout << "Valid Request received. host: " << host << std::endl;


    // CONNECTION
    // Handled directly on HandleClient with an if condition in removeClient at end of transaction.
    
    // 
    
}
