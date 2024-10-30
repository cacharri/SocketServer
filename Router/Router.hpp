/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 19:45:30 by smagniny          #+#    #+#             */
/*   Updated: 2024/10/30 03:13:56 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

// Forward declarations
class Request;
class Response;

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
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <dirent.h>

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

    void    route(const Request* request, Response* response);
private:
    // RouteConfig se refiere a {  {'methodos', 'disponibles'}, (*function) }
    // string es el endpoint.
    Router::RouteConfig*    HasValidMethod(std::vector<RouteConfig *>& ConfigsAllowed, const std::string& input_method);
    std::map<std::string, std::vector<RouteConfig *> >  routes;
};

bool isDirectory(const std::string& path);
std::string generateAutoIndex(const std::string& directory);

#endif