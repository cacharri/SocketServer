/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 19:45:30 by smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 16:43:01 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>
#include "../Response/Response.hpp"
#include "../Request/Request.hpp"
#include "../Logger/Logger.hpp"
#include <set>

class Router {
public:
    Router();
    ~Router();
    struct RouteConfig {
        std::set<std::string> allowedMethods; // Set of allowed methods
        void (*handler)(const std::string&, Response&); // Handler function
    };
    
    void    addRoute(const std::string& path, const std::set<std::string>& methods, void (*handler)(const std::string& path, Response& res));
    void    route(const Request& request, Response& response);
private:
    // RouteConfig se refiere a {  {'methodos', 'disponibles'}, (*function) }
    // string es el endpoint.
    std::map<std::string, RouteConfig>  routes;
};


#endif