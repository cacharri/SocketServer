/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:42:08 by smagniny          #+#    #+#             */
/*   Updated: 2024/11/28 15:05:34 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP


#include "../Logger/Logger.hpp"

#include <string>
#include <vector>
#include <map>
// #include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>

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
    int                         redirect_type;

    LocationConfig(): autoindex(false), allow_upload(false), client_max_body_size(8192), redirect_type(0) {};
    
    // Copy constructor for deep copy
    LocationConfig(const LocationConfig& other) {
        root = other.root;
        index = other.index;
        autoindex = other.autoindex;
        methods = other.methods;  // vector has its own copy constructor
        allow_upload = other.allow_upload;
        upload_store = other.upload_store;
        client_max_body_size = other.client_max_body_size;
        cgi_pass = other.cgi_pass;
        redirect = other.redirect;
        redirect_type = other.redirect_type;
    }

    // Assignment operator for deep copy
    LocationConfig& operator=(const LocationConfig& other) {
        if (this != &other) {
            root = other.root;
            index = other.index;
            autoindex = other.autoindex;
            methods = other.methods;  // vector handles its own deep copy
            allow_upload = other.allow_upload;
            upload_store = other.upload_store;
            client_max_body_size = other.client_max_body_size;
            cgi_pass = other.cgi_pass;
            redirect = other.redirect;
            redirect_type = other.redirect_type;
        }
        return *this;
    }
};

struct ServerConfig {
    std::string								interface; //direccion ip o del host donde el servidor escucha
    std::vector<int>						ports; 
    std::string								server_name;
    std::map<int, std::string>				error_pages; //paginas personalizadas segun el codigo de error
    size_t									client_max_body_size; // 
    std::map<std::string, LocationConfig>	locations; // map de rutas a confi instead ofvector para asociar cada ruta 
    ServerConfig(): client_max_body_size(8192) {};
};

class ConfigParser {
	private:
        ConfigParser();
        ~ConfigParser();
        ConfigParser(const ConfigParser& copy);
        ConfigParser& operator=(const ConfigParser& assign);

	public:
        
		static  std::vector<ServerConfig>   parseServerConfigFile(const std::string& filename);
        static  void                        copyServerConfig(const ServerConfig& source, ServerConfig& destination);
        
        static  void                        setDefaultErrorPages(ServerConfig& destination);
		static                   size_t     parseSize(const std::string& sizeStr);
        static  void                        printLocationConfig(const ServerConfig& serverConfig);
        static  bool                        validateSyntax(const std::string& configFilePath);
        static  bool                        validateServerParams(const ServerConfig& serverConfig);
        static  std::string                 intToString(int value);
        static  bool                        validateLocationParams(const LocationConfig& locationConfig);
        static  bool                        validateMethods(const std::vector<std::string>& methods);

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