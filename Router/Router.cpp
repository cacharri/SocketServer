#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::~Router()
{
    // Iterate through the map
    for (std::map<std::string, std::vector<RouteConfig> >::iterator it = routes.begin(); it != routes.end(); ++it) {
        // Iterate through the vector of RouteConfig
        for (std::vector<RouteConfig>::iterator vecIt = it->second.begin(); vecIt != it->second.end(); ++vecIt) {
            // Delete the handler if it's a pointer
            if (vecIt->handler)
            {
                delete vecIt->handler;
                vecIt->handler = NULL; // Avoid dangling pointer
            }

        }
        // Clear the vector after processing if needed
        //delete it->second;
        it->second.clear();
    }
}

void    Router::addRoute(const std::string& path, const LocationConfig& locationconfig, RequestHandler *requesthandler, std::string HandledMethod)
{
    RouteConfig *config = new RouteConfig;
    config->endpointdata.root = locationconfig.root;
    config->endpointdata.index = locationconfig.index;
    config->endpointdata.autoindex = locationconfig.autoindex;
    config->endpointdata.methods.push_back(HandledMethod); // shallow copy
    config->endpointdata.allow_upload = locationconfig.allow_upload;
    config->endpointdata.upload_store = locationconfig.upload_store;
    config->endpointdata.cgi_pass = locationconfig.cgi_pass;
    config->endpointdata.redirect = locationconfig.redirect;
    config->handler = requesthandler;

    // RouteConfig config;
    // config.endpointdata.root = locationconfig.root;
    // config.endpointdata.index = locationconfig.index;
    // config.endpointdata.autoindex = locationconfig.autoindex;
    // config.endpointdata.methods.push_back(HandledMethod); // shallow copy
    // config.endpointdata.allow_upload = locationconfig.allow_upload;
    // config.endpointdata.upload_store = locationconfig.upload_store;
    // config.endpointdata.cgi_pass = locationconfig.cgi_pass;
    // config.endpointdata.redirect = locationconfig.redirect;
    // config.handler = requesthandler;

    // std::cout << "Added route: " << path << " with allowed methods: ";
    // std::vector<std::string>::const_iterator it;
    // for (it = config.endpointdata.limit_except.begin(); it != config.endpointdata.limit_except.end(); ++it)
    //     std::cout << *it << " ";
    //std::cout << std::endl;
    routes[path].push_back(*config);
}

void    Router::loadEndpoints(const std::string& endpoint, const LocationConfig& locConfig)
{
    // Verificar si GET está permitido y asociarlo a la función adecuada
    if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "GET") != locConfig.methods.end()) {
        GetHandler  *get_handler_instance = new GetHandler();
        addRoute(endpoint, locConfig, get_handler_instance, "GET");
    }
    // Verificar si POST está permitido y asociarlo a la función de manejar POST
    if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "POST") != locConfig.methods.end()) {
        PostHandler  *post_handler_instance = new PostHandler();
        std::cout << "He añadido un POST endpoint ! " << endpoint << std::endl;
        addRoute(endpoint, locConfig, post_handler_instance, "POST");
    }
}

// Router::RouteConfig     *Router::HasValidMethod(std::vector<RouteConfig>	ConfigsAllowed, std::string input_method)
// {
//     std::vector<RouteConfig>::iterator vector_route_iterator = ConfigsAllowed.begin();

//     for (; vector_route_iterator != ConfigsAllowed.end(); vector_route_iterator++)
//     {
//         for (std::vector<std::string>::iterator vecIt = vector_route_iterator->endpointdata.methods.begin(); vecIt != vector_route_iterator->endpointdata.methods.end(); ++vecIt)
//         {
//             if (*vecIt == input_method)
//                 return &(*vector_route_iterator);
//         }
//     }
//     return NULL;
// }

Router::RouteConfig* Router::HasValidMethod(std::vector<RouteConfig>& ConfigsAllowed, const std::string& input_method) {
    // Iterate over the vector by reference (C++98 style)
    for (std::vector<RouteConfig>::iterator it = ConfigsAllowed.begin(); it != ConfigsAllowed.end(); ++it) {
        if (std::find(it->endpointdata.methods.begin(), it->endpointdata.methods.end(), input_method) != it->endpointdata.methods.end()) {
            return &(*it);  // Return pointer to the matching RouteConfig
        }
    }
    return NULL;  // Return NULL if no valid method is found
}

void Router::route(const Request& request, Response& response) {
    if (routes.empty())
    {
        std::cout << "empty router map" << std::endl;
        return ;
    }

    std::map<std::string, std::vector<RouteConfig> >::iterator it = routes.find(request.getUri()) ;
    if (it != routes.end())
    {
        RouteConfig *ptr = HasValidMethod(it->second, request.getMethod());
        if (ptr == NULL)
        {
            response.setStatus(405, "Method Not Allowed");
            response.setBody("405 Method Not Allowed");
        }
        //std::cout << "test::: "<< ptr->endpointdata.root << std::endl;
        std::cout << "Han solicitado " << ptr->endpointdata.index << std::endl;
        ptr->handler->handle(request, response, ptr->endpointdata);
        

    } else {
        response.setStatus(404, "Not found");
        response.setBody("404 Not Found");
    }
}

