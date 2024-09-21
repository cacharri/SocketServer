#include "Router.hpp"

Router::Router()
{
}

Router::~Router()
{
}

void Router::addRoute(const std::string& path, const std::set<std::string>& methods, void (*handler)(const std::string& path, Response& res)) {
    RouteConfig config;
    config.allowedMethods = methods;
    config.handler = handler;

    std::cout << "Added route: " << path << " with allowed methods: ";
    std::set<std::string>::const_iterator it;
    for (it = methods.begin(); it != methods.end(); ++it)
        std::cout << *it << " ";

    std::cout << std::endl;

    routes[path] = config;
}

void Router::route(const Request& request, Response& response) {
    std::map<std::string, RouteConfig>::iterator it = routes.find(request.getUri());
    if (it != routes.end()) {
        // {'GET','POST'}.find('GET') == True && iterator is not end
        if (it->second.allowedMethods.find(request.getMethod()) != it->second.allowedMethods.end()) 
        {
            if (request.getMethod() == "GET")
                it->second.handler(request.getUri(), response);// llamada al puntero de la funcion handler de this->routes[URI]{{'GET'}, (*func)}
            else if (request.getMethod() == "POST")
                it->second.handler(request.getUri(), response); 
        
        } else {
            response.setStatus(405, "Method Not Allowed");
            response.setBody("405 Method Not Allowed");
        }
    } else {
        response.setStatus(404, "Not found");
        response.setBody("404 Not Found");
    }
}

