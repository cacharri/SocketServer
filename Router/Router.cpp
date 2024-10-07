#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::~Router()
{
}

std::string printCurrentWorkingDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        return (cwd);
    else
        return "";
}

void    Router::addRoute(const std::string& path, const LocationConfig& locationconfig, void (*handler)(const std::string& path, Response& res))
{
    RouteConfig config;
    config.endpointdata.root = locationconfig.root;
    config.endpointdata.index = locationconfig.index;
    config.endpointdata.autoindex = locationconfig.autoindex;
    config.endpointdata.limit_except = locationconfig.limit_except; // shallow copy
    config.endpointdata.allow_upload = locationconfig.allow_upload;
    config.endpointdata.upload_store = locationconfig.upload_store;
    config.endpointdata.cgi_pass = locationconfig.cgi_pass;
    config.endpointdata.redirect = locationconfig.redirect;
    config.handler = handler;

    std::cout << "Added route: " << path << " with allowed methods: ";
    std::vector<std::string>::const_iterator it;
    for (it = config.endpointdata.limit_except.begin(); it != config.endpointdata.limit_except.end(); ++it)
        std::cout << *it << " ";

    std::cout << std::endl;

    routes[path] = config;
}


void Router::route(const Request& request, Response& response) {
    std::map<std::string, RouteConfig>::iterator it = routes.find(request.getUri());
    if (it != routes.end()) {

        bool methodAllowed = false;
        for (std::vector<std::string>::iterator vecIt = it->second.endpointdata.limit_except.begin(); 
             vecIt != it->second.endpointdata.limit_except.end(); ++vecIt) {
            if (*vecIt == request.getMethod()) {
                methodAllowed = true;
                break;
            }
        }

        if (methodAllowed) {
            std::string fullPath =  printCurrentWorkingDirectory();
            fullPath = fullPath + it->second.endpointdata.root + "/" + it->second.endpointdata.index; // Construct full path
            std::cout << "[Endpoint] --> " << request.getUri() << "  -->  " << fullPath << std::endl;
            if (request.getMethod() == "GET") {
                it->second.handler(fullPath, response); 
            } else if (request.getMethod() == "POST") {
                it->second.handler(fullPath, response);
                std::cout << "hola\n" << std::endl; 
            }
        } else {
            response.setStatus(405, "Method Not Allowed");
            response.setBody("405 Method Not Allowed");
        }
    } else {
        response.setStatus(404, "Not found");
        response.setBody("404 Not Found");
    }
}

