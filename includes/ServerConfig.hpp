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
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>


struct LocationConfig {
    std::string					root;  //directorio raiz 
    std::string					index; // archivo predeterminado si se solicita un directorio
    bool						autoindex;		
    std::vector<std::string>	limit_except; //para los metodos https permitidos en la ubicacion
    bool						allow_upload; //booleano para indicar si se permite o no la carga de archivos
    std::string					upload_store; // directorio donde se almacenan los archivos cargados si se acepta la carga
    std::string					cgi_pass; //ruta o comando que se va a usar para ejecutar el script
};

struct ServerConfig {
    std::string								host; //direccion ip o del host donde el servidor escucha
    int										port; 
    std::string								server_name;
    std::map<int, std::string>				error_pages; //paginas personalizadas segun el codigo de error
    size_t									client_max_body_size; // 
    std::map<std::string, LocationConfig>	locations; // map de rutas a confi instead ofvector para asociar cada ruta 
};

class ConfigParser {
	public:
		ConfigParser();
		~ConfigParser();

		ConfigParser(const ConfigParser& other);

		ConfigParser& operator=(const ConfigParser& other);

		std::vector<ServerConfig> parseConfigFile(const std::string& filename) const;

	private:
		size_t parseSize(const std::string& sizeStr) const;
};

#endif