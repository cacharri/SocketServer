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


void DeleteHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    std::string filepath = request->getPath();
std::cout << "DELETE HANDLER "<<std::endl;
request->print();
 //   std::string filepath = request->getQueryParam("filepath");
 std::string output = response->getBody();
 response->setHeader("Content-Type", "text/html");
    response->setBody(output);
    // Verificar si el archivo existe
    if (fileExists(filepath)) {
        // Intentar eliminar el archivo
        if (remove(filepath.c_str()) == 0) {
            response->setStatusCode(200);  // OK
            response->setBody("File deleted successfully.");
        } else {
            response->setStatusCode(500);  // Error al eliminar
            response->setBody("Failed to delete file.");
        }
    } else {
        response->setStatusCode(404);  // No encontrado
        response->setBody("File not found.");
    }
}

