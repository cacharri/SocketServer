/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 14:08:00 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/17 17:40:16 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "ServerConfig.hpp"

ConfigReader::ConfigReader(const std::string &path_to_file)
{
	_input_file.open(path_to_file, std::ifstream::in);
}

//ejemplo de getter para la struct
std::string	ConfigReader::getHost() const
{
	return (this->_config.host);	
}
//ejemplo de setter para la struct
void	ConfigReader::setHost(std::string &hostdomain)
{
	this->_config.host = hostdomain;
}


ConfigReader::~ConfigReader()
{
}