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
    std::cout << "Destructor of router called "<< std::endl;
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

    routes[path].push_back(config);
     std::cout << "Ruta añadida: " << path << " con autoindex: " << (locationconfig.autoindex ? "Habilitado" : "Deshabilitado") << std::endl;
}

void    Router::loadEndpoints(const std::string& endpoint, const LocationConfig& locConfig)
{
    // Verificar si GET está permitido y asociarlo a la función adecuada
    if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "GET") != locConfig.methods.end()) {
        GetHandler  *get_handler_instance = new GetHandler();
        std::cout << "[ROUTER] Se ha añadido un GET endpoint ! " << endpoint << std::endl;
        addRoute(endpoint, locConfig, get_handler_instance, "GET");
    }
    // Verificar si POST está permitido y asociarlo a la función de manejar POST
    if (std::find(locConfig.methods.begin(), locConfig.methods.end(), "POST") != locConfig.methods.end()) {
        PostHandler  *post_handler_instance = new PostHandler();
        std::cout << "[ROUTER] Se ha añadido un POST endpoint ! " << endpoint << std::endl;
        addRoute(endpoint, locConfig, post_handler_instance, "POST");
    }
    if ((!locConfig.cgi_pass.empty()) && 
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

std::string generateAutoIndex(const std::string& directory) {
    std::cout << "Generando autoindex para el directorio: " << directory << std::endl; // Añadir depuración
    std::string autoindexHtml = "<html><body><h1>Index of " + directory + "</h1><ul>";
    
    DIR* dir = opendir(directory.c_str());
    if (dir != NULL) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string filename = entry->d_name;
            if (filename != "." && filename != "..") {
                autoindexHtml += "<li><a href=\"" + filename + "\">" + filename + "</a></li>";
            }
        }
        closedir(dir);
    } else {
        // Error al abrir el directorio
        autoindexHtml = "<html><body><h1>Error: Cannot open directory</h1></body></html>";
        std::cerr << "Error al abrir el directorio: " << strerror(errno) << std::endl; // Mostrar error
    }
    
    autoindexHtml += "</ul></body></html>";
    return autoindexHtml;
}


void Router::route(const Request& request, Response& response) {
    if (routes.empty()) {
        std::cout << "empty router map" << std::endl;
        return;
    }

    std::map<std::string, std::vector<RouteConfig *> >::iterator it = routes.find(request.getUri());
    
    if (it != routes.end()) {
        RouteConfig *ptr = HasValidMethod((*it).second, request.getMethod());
        
        if (ptr == NULL) {
            response.setStatus(405, "Method Not Allowed");
            response.setBody("405 Method Not Allowed");
            return;
        }
        //
        
    // Lógica existente para manejar otras solicitudes
    //

            // Verificar si el autoindex está habilitado y si la ruta es un directorio
            std::string fullPath = ptr->endpointdata.root + request.getUri();
            std::cout << "Ruta completa: " << fullPath << std::endl;
            //Verifica si es una solicitud CGI
            if (!((ptr->endpointdata.cgi_pass).empty())) {
                CgiHandler cgiHandler;              // Crea una instancia del CgiHandler
                cgiHandler.handle(request, response, ptr->endpointdata);
                }  // Maneja la solicitud CGI
            else {
                // Código existente para servir archivos como index.html
                std::cout << "Han solicitado " << ptr->endpointdata.index << std::endl;
                ptr->handler->handle(request, response, ptr->endpointdata);
            }

    }
    else {
        response.setStatus(404, "Not found");
        response.setBody("404 Not Found");
    }
}

