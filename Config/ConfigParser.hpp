/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:42:08 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/29 16:56:05 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP


#include "../Logger/Logger.hpp"
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <sstream>

struct LocationConfig {
    std::string					root;  //directorio raiz 
    std::string					index; // archivo predeterminado si se solicita un directorio
    bool						autoindex;
    std::vector<std::string>	methods; //para los metodos https permitidos en la ubicacion
    bool						allow_upload; //booleano para indicar si se permite o no la carga de archivos
    std::string					upload_store; // directorio donde se almacenan los archivos cargados si se acepta la carga
    size_t					    client_max_body_size;
    std::string					cgi_pass; //ruta o comando que se va a usar para ejecutar el script
    std::string                 redirect;

    LocationConfig(): autoindex(false), allow_upload(false), client_max_body_size(8192) {};
};

struct ServerConfig {
    std::string								host; //direccion ip o del host donde el servidor escucha
    int										port; 
    std::string								server_name;
    std::map<int, std::string>				error_pages; //paginas personalizadas segun el codigo de error
    size_t									client_max_body_size; // 
    std::map<std::string, LocationConfig>	locations; // map de rutas a confi instead ofvector para asociar cada ruta 

    ServerConfig(): port(8080), client_max_body_size(8192) {};
};

class ConfigParser {
	private:
        ConfigParser();
        ~ConfigParser();
        
        static void printLocationConfig(const ServerConfig& serverConfig);

	public:
		static  std::vector<ServerConfig>   parseServerConfigFile(const std::string& filename);
        static  void                        copyServerConfig(const ServerConfig& source, ServerConfig& destination);
		static                   size_t     parseSize(const std::string& sizeStr);

        class ConfigError: public std::exception
        {
            private:
            std::string error_string;
            public:
                ConfigError(const std::string& error);
                virtual const char* what() const throw();
                virtual ~ConfigError() throw();
        };

};


#endif