#include "Handlers.hpp"

DeleteHandler::DeleteHandler() {
    // Constructor
}

DeleteHandler::~DeleteHandler() {
    // Destructor
}

std::string DeleteHandler::getQueryParam(const std::string& url, const std::string& param) {
    std::string value;
    size_t paramPos = url.find(param + "=");  // Buscar el parámetro
    if (paramPos != std::string::npos) {
        paramPos += param.length() + 1;  // Avanzar hasta el valor
        size_t endPos = url.find("&", paramPos);  // Buscar el final del valor (si existe otro parámetro)

        if (endPos == std::string::npos) {
            value = url.substr(paramPos);  // Si no hay más parámetros, tomamos todo el resto
        } else {
            value = url.substr(paramPos, endPos - paramPos);  // Si hay otro parámetro, tomamos solo el valor
        }
    }
    return value;
}

bool DeleteHandler::fileExists(const std::string& filepath) {
    std::ifstream file(filepath.c_str());  // Abrir el archivo
    return file.good();  // Verificar si el archivo se abrió correctamente
}

std::string urlDecode(const std::string &encoded) {
    std::string decoded;
    char ch;
    int i, j;
    for (i = 0; i < encoded.length(); i++) {
        if (int(encoded[i]) == 37) {
            sscanf(encoded.substr(i + 1, 2).c_str(), "%x", &j);
            ch = static_cast<char>(j);
            decoded += ch;
            i = i + 2;
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

void DeleteHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    std::string resourcePath = locationconfig.root;
    if (!resourcePath.empty() && resourcePath[resourcePath.size() - 1] == '/') {
        resourcePath = resourcePath.substr(0, resourcePath.size() - 1);
    }
    resourcePath += request->getUri();
    std::cout << "esto es:" << resourcePath << std::endl;

    // Verificar si el archivo o directorio existe
    struct stat fileStat;
    if (stat(resourcePath.c_str(), &fileStat) != 0) {
        response->setStatusCode(404);
        response->setBody("<html><body><h1>404 Not Found</h1></body></html>");
        return;
    }

    if (S_ISDIR(fileStat.st_mode)) {
        if (rmdir(resourcePath.c_str()) == 0) {
            response->setStatusCode(204);
        } else {
            response->setStatusCode(403);
            response->setBody("<html><body><h1>403 Forbidden</h1></body></html>");
        }
    } else {
        if (remove(resourcePath.c_str()) == 0) {
            response->setStatusCode(204);
        } else {
            response->setStatusCode(403);
            response->setBody("<html><body><h1>403 Forbidden</h1></body></html>");
        }
    }
}

