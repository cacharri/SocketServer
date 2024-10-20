/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderAnalyzer.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 16:59:39 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/20 22:41:32 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../Config/ConfigParser.hpp"
#include <iostream>

void     analyzeBasicHeaders(const Request& request, Response& response, const ServerConfig& config);

// void analyzeGetHeaders(const Request& request, Response& response);
// void analyzePostHeaders(const Request& request, Response& response);
bool isValidHostHeader(const std::string &hostHeader);
