/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 20:09:58 by smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 20:33:28 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <fstream>
#include <iostream>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"

std::string readFile(const std::string &filePath);
void        setBodyWithFile(const std::string& filestr, Response& response);

// void        handleHome(const Request& request, Response& response);
// void        handleAbout(const Request& request, Response& response);



#endif
