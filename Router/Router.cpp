#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::~Router()
{
    // Iteramos en el mapa
    for (std::map<std::string, std::vector<RouteConfig *> >::iterator it = routes.begin(); it != routes.end(); ++it)
    {
        // Iteratamos el vector Routeconfig de la variable del mapa
        for (std::vector<RouteConfig *>::iterator vecIt = it->second.begin(); vecIt != it->second.end(); ++vecIt)
        {
            //Borramos el puntero al Handler que esta dentro de RouteConfig
            if ((*vecIt)->handler)
            {
                delete (*vecIt)->handler;
                (*vecIt)->handler = NULL; 
            }
            //y el puntero RouteConfig
            delete (*vecIt);
        }
    }
    LOG_INFO("Destructor of Router Called");
    //std::cout << "Destructor of router called "<< std::endl;
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
    config->endpointdata.redirect_type = locationconfig.redirect_type;
    config->handler = requesthandler;

    routes[path].push_back(config);
    //std::cout << "Ruta añadida: " << path << " con autoindex: " << (locationconfig.autoindex ? "Habilitado" : "Deshabilitado") << std::endl;
}

void    Router::loadEndpoints(const std::string& endpoint, const LocationConfig& locConfig)
{
     /* if ((!locConfig.cgi_pass.empty()) && 
        (std::find(locConfig.methods.begin(), locConfig.methods.end(), "GET") != locConfig.methods.end() ||
         std::find(locConfig.methods.begin(), locConfig.methods.end(), "POST") != locConfig.methods.end())) {
        CgiHandler *cgi_handler_instance = new CgiHandler();
        std::cout << "[ROUTER] Se ha añadido un CGI endpoint ! " << endpoint << std::endl;
        if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "GET") != locConfig.methods.end()){
            addRoute(endpoint, locConfig, cgi_handler_instance, "GET");  // Ajusta para GET o POST según sea necesario
        }
        else if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "POST") != locConfig.methods.end()){
            addRoute(endpoint, locConfig, cgi_handler_instance, "POST"); // Maneja ambos métodos si es necesario
        }
    }
    else{*/
        // Verificar si GET está permitido y asociarlo a la función adecuada
        if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "GET") != locConfig.methods.end()) {
            GetHandler  *get_handler_instance = new GetHandler();
            //std::cout << "[ROUTER] Se ha añadido un GET endpoint ! " << endpoint << std::endl;
            addRoute(endpoint, locConfig, get_handler_instance, "GET");
        }
        // Verificar si POST está permitido y asociarlo a la función de manejar POST
        if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "POST") != locConfig.methods.end()) {
            PostHandler  *post_handler_instance = new PostHandler();
            //std::cout << "[ROUTER] Se ha añadido un POST endpoint ! " << endpoint << std::endl;
            addRoute(endpoint, locConfig, post_handler_instance, "POST");
        }
    //}
  
}



Router::RouteConfig* Router::HasValidMethod(std::vector<RouteConfig *>& ConfigsAllowed, const std::string& input_method)
{   // iteramos en el vector de RouteConfig que nos pasan por parametros y comprobamos que input_method este disponible
    for (std::vector<RouteConfig *>::iterator it = ConfigsAllowed.begin(); it != ConfigsAllowed.end(); ++it) {
        if (std::find((*it)->endpointdata.methods.begin(), (*it)->endpointdata.methods.end(), input_method) != (*it)->endpointdata.methods.end()) {
            return (*it);  // Retornamos un puntero al RouteConfig con el methodo disponible
        }
    }
    return NULL; 
}

bool isDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // No se puede acceder a la ruta
    }
    return (info.st_mode & S_IFDIR) != 0; // Verifica si es un directorio
}

void Router::route(const Request* request, Response* response)
{
    if (routes.empty()) {
        response->setStatusCode(404);
        return;
    }

    std::string best_match_path = "";
    RouteConfig* best_match_config = NULL;
    size_t best_match_length = 0;
    std::string remaining_path = "";

    // Mapear el URI 
    for (std::map<std::string, std::vector<RouteConfig*> >::iterator it = routes.begin(); 
         it != routes.end(); ++it)
    {
        const std::string& location_path = it->first;
        LocationConfig& loc_config = it->second[0]->endpointdata;

        // Match exacto del endpoint
        if (request->getUri() == location_path)
        {
            best_match_config = it->second[0];
            best_match_path = location_path;
            remaining_path = loc_config.index;
            break;
        }
        
        // Match por prefijo de endpoint (relativo)
        if (request->getUri().substr(0, location_path.length()) == location_path)
        {
            if (location_path.length() > best_match_length)
            {
                best_match_length = location_path.length();
                best_match_config = it->second[0];
                best_match_path = location_path;
                remaining_path = request->getUri().substr(location_path.length());
            }
        }
    }
    //std::cout << "Match with " << best_match_config->endpointdata.root << std::endl;
    if (!best_match_config)
    {
        response->setStatusCode(404);
        response->setBody(readFile("/var/www/error-pages/404.html"));
        return;
    }

    // Construir el path relativo
    std::string full_path = best_match_config->endpointdata.root;
    if (!remaining_path.empty())
    {
        if (full_path[full_path.length()-1] != '/' && remaining_path[0] != '/')
            full_path += '/';
        full_path += remaining_path;
    }
    // pasar una copia de LocationConfig con el root relativo enrutado
    LocationConfig temp_config = best_match_config->endpointdata;
    temp_config.root = full_path;
    RouteConfig* route_config = HasValidMethod(routes[best_match_path], request->getMethod());
    if (request->getMethod() != "GET" && request->getMethod() != "POST" && request->getMethod() != "DELETE")
    {
        response->setStatusCode(501);
        response->setBody(readFile("/var/www/error-pages/501.html"));
        return ;
    }
    else if (route_config)
    {
        // Verificar si el metodo existe para el endpoint y llamar al handler al endpoint enrutado
        route_config->handler->handle(request, response, temp_config);
    }
    else
    {
        response->setStatusCode(405);
        response->setBody(readFile("/var/www/error-pages/405.html"));
    }

}


