/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:42:08 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/17 18:02:56 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <string>
#include <fstream>
#include <vector>



struct Location
{
	std::string	method_on_location;
	std::string	uri;
	std::string http_version;
	int			autoindex;
	//...
};


struct ServerConfig
{
    std::string				host;
	int						port;
	std::string				server_name;
	size_t					bodylimitforclient;
	std::vector<Location>	location;
	//...
};

class ConfigReader
{
	private:
		std::ifstream	_input_file;
		ServerConfig	_config;
	public:
		ConfigReader(const std::string &path_to_file);

		//getters
		std::string	getHost()const ;
		//setters
		void		setHost(std::string &hostdomain);
		
		~ConfigReader();
};

#endif