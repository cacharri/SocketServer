/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/25 12:00:00 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/25 12:42:12 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

std::vector<std::string> split(std::string text, char delim);


std::vector<std::string> split(std::string text, char delim)
{
    std::string line;
    std::vector<std::string> vec;
    std::stringstream ss(text);
    while(std::getline(ss, line, delim) )
        vec.push_back(line);
    
    // En el caso que no se haya encontrado index end, start sigue siendo mas pequeno que el tamano final
    // if (headers_start_char_iterator < data.size())
	// {
	// 		headers.push_back(data.substr(headers_start_char_iterator));
	// 		headerlines++;
    // }
    return vec;
}