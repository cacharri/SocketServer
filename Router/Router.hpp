/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 19:45:30 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/15 17:52:55 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>
#include "../Response/Response.hpp"
#include "../Request/Request.hpp"
#include "../Logger/Logger.hpp"
#include "../Config/ConfigParser.hpp"
#include "../Handlers/Handlers.hpp"

#include <algorithm>
#include <limits.h>
#include <unistd.h>
#include <iostream>
    // std::string					root;  //directorio raiz 
    // std::string					index; // archivo predeterminado si se solicita un directorio
    // bool						autoindex;
    // std::vector<std::string>	limit_except; //para los metodos https permitidos en la ubicacion
    // bool						allow_upload; //booleano para indicar si se permite o no la carga de archivos
    // std::string					upload_store; // directorio donde se almacenan los archivos cargados si se acepta la carga
    // std::string					cgi_pass; //ruta o comando que se va a usar para ejecutar el script
    // std::string                 redirect;

class Router {
public:
    Router();
    ~Router();
    struct RouteConfig {
        LocationConfig      endpointdata;
        RequestHandler      *handler; // Handler function
        
    };
    
    void    loadEndpoints(const std::string& endpoint, const LocationConfig& locConfig);
    //void    addRoute(const std::string& path, const LocationConfig& locationconfig, RequestHandler* handler);
    void    addRoute(const std::string& path, const LocationConfig& locationconfig, RequestHandler *requesthandler, std::string HandledMethod);

    void    route(const Request& request, Response& response);
private:
    // RouteConfig se refiere a {  {'methodos', 'disponibles'}, (*function) }
    // string es el endpoint.
    Router::RouteConfig*    HasValidMethod(std::vector<RouteConfig *>& ConfigsAllowed, const std::string& input_method);
    std::map<std::string, std::vector<RouteConfig *> >  routes;
};


#endif