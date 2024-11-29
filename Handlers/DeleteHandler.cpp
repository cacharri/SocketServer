#include "Handlers.hpp"

DeleteHandler::DeleteHandler() {
    // Constructor
}

DeleteHandler::~DeleteHandler() {
    // Destructor
}

// std::string DeleteHandler::getQueryParam(const std::string& url, const std::string& param) {
//     std::string value;
//     size_t paramPos = url.find(param + "=");  // Buscar el parámetro
//     if (paramPos != std::string::npos) {
//         paramPos += param.length() + 1;  // Avanzar hasta el valor
//         size_t endPos = url.find("&", paramPos);  // Buscar el final del valor (si existe otro parámetro)

//         if (endPos == std::string::npos) {
//             value = url.substr(paramPos);  // Si no hay más parámetros, tomamos todo el resto
//         } else {
//             value = url.substr(paramPos, endPos - paramPos);  // Si hay otro parámetro, tomamos solo el valor
//         }
//     }
//     return value;
// }

// bool DeleteHandler::fileExists(const std::string& filepath) {
//     std::ifstream file(filepath.c_str());  // Abrir el archivo
//     return file.good();  // Verificar si el archivo se abrió correctamente
// }

// std::string urlDecode(const std::string &encoded) {
//     std::string decoded;
//     char ch;
//     int i, j;
//     for (i = 0; i < encoded.length(); i++) {
//         if (int(encoded[i]) == 37) {
//             sscanf(encoded.substr(i + 1, 2).c_str(), "%x", &j);
//             ch = static_cast<char>(j);
//             decoded += ch;
//             i = i + 2;
//         } else {
//             decoded += encoded[i];
//         }
//     }
//     return decoded;
// }


void DeleteHandler::remove_file_or_dir(Response* response, const LocationConfig& Locationconfig)
{
    // Verificar si el archivo o directorio existe
    char        cwd[PATH_MAX];
    std::string fullpath;
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        fullpath.append(cwd);

    fullpath += Locationconfig.root;
    LOG_INFO("Attempting to delete: " + fullpath);
    std::cout << fullpath << std::endl;
    struct stat fileStat;
    if (stat(fullpath.c_str(), &fileStat) != 0) {
        if (errno == ENOENT) {
            response->setStatusCode(404);
            LOG_INFO("404 Not Found - File or directory does not exist");
        } else {
            response->setStatusCode(403);
            LOG_INFO("403 Forbidden - Access denied to file or directory");
        }
        return;
    }
    if (S_ISDIR(fileStat.st_mode)) {
        if (rmdir(fullpath.c_str()) == 0)
        {
            response->setBody("<html><body><h1>Directory Removed Successfully!</h1><p><a href='/delete'>Delete another file</a></p><p><a href='/'>Go back to home</a></p></body></html>");
            response->setStatusCode(204);
            LOG_INFO("204 No Content - Recurso eliminado correctamente");
            return ;
        } 
        else if (errno == ENOTEMPTY) {
            response->setStatusCode(403);
            response->setBody("<html><body><h1>Directory is not empty</h1></body></html>");
            LOG_INFO("403 Forbidden - Directory not empty");
        } 
        else {
            response->setStatusCode(403);
            LOG_INFO("403 Forbidden - Failed to remove directory");
        }
    }
    else {
        if (remove(fullpath.c_str()) == 0)
        {
            response->setBody("<html><body><h1>File Removed Successfully!</h1><p><a href='/delete'>Delete another file</a></p><p><a href='/'>Go back to home</a></p></body></html>");
            response->setStatusCode(204);
            LOG_INFO("204 No Content - Recurso eliminado correctamente");
            return ;
        }
        else
            response->setStatusCode(500);
            response->setBody("<html><body><h1>Internal Server Error</h1></body></html>");
            LOG_INFO("500 Internal Server Error - Failed to remove file");
    }
}



void DeleteHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig)
{
    LOG_INFO("DELETE request on route;");
    LOG_INFO(request->getUri());
    remove_file_or_dir(response, locationconfig);
}


