#include "Handlers.hpp"

PostHandler::PostHandler()
{
    
}

PostHandler::~PostHandler()
{
    
}

void PostHandler::handleMultipartFormData(const Request* request, Response* response, LocationConfig& locationconfig) {
    std::string contentType = request->getHeader("Content-Type");
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        response->setStatusCode(400);
        response->setBody("<html><body><h1>Bad Request - Missing Boundary</h1></body></html>");
        return;
    }

    std::string boundary = contentType.substr(boundaryPos + 9); // Longitud de "boundary="
    std::string filename;
    std::string fileData = parseMultipartFormData(request->getBody(), boundary, locationconfig.upload_store, filename);

    if (fileData.empty()) {
        response->setStatusCode(400);
        response->setBody("<html><body><h1>Bad Request - Invalid Multipart Data</h1></body></html>");
        return;
    }

    if (!saveFile(filename, fileData)) {
        response->setStatusCode(500);  // Internal Server Error
        response->setBody("<html><body><h1>Internal Server Error - Failed to Save File</h1></body></html>");
        return;
    }
    else
    {
        response->setStatusCode(201);
        response->setBody("<html><body><h1>File Uploaded Succesfully! </h1><p><a href='upload'>upload another file </a</p><p><a href='/'>GO back home</a></p></body></html>");
        return;
    }
    // Responder con el mismo cuerpo enviado por el cliente
    response->setStatusCode(201);
    response->setBody(request->getBody());
}

void PostHandler::handleUrlFormEncoded(const Request* request, Response* response, LocationConfig& locationconfig) {
    // Obtener el cuerpo de la solicitud
    std::string body = request->getBody();

    // Parsear el cuerpo como pares clave-valor
    std::map<std::string, std::string> formData = parseUrlFormData(body);

    // Si no hay datos, responde con el mismo cuerpo recibido
    if (formData.empty()) {
        response->setStatusCode(200);
        response->setBody(body);
        return;
    }

    // Respuesta con los datos procesados
    std::string responseBody = "<html><body>";
    responseBody += "<h1>Formulario Recibido</h1>";
    for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it) {
        responseBody += "<p>" + escapeHtml(it->first) + ": " + escapeHtml(it->second) + "</p>";
    }
    responseBody += "</body></html>";

    response->setStatusCode(200);
    response->setBody(responseBody);
}

void        PostHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig)
{   
    //std::cout << "Received POST request" << std::endl;
    //request->print();
    // Verificar si el cuerpo excede el tamaño máximo permitido
    size_t maxBodySize = locationconfig.client_max_body_size;
    if (request->getBody().size() > maxBodySize) {
        LOG("Request body exceeds maximum allowed size");
        response->setStatusCode(413);  // Payload Too Large

        return;
    }

    std::string contentType = request->getHeader("Content-Type");

    if (contentType.empty()) {
        LOG_INFO("Processing request without Content-Type");
        response->setStatusCode(201);  // Created
        response->setBody(request->getBody());
        return;
    }

    if (contentType.find("multipart/form-data") != std::string::npos) {
        handleMultipartFormData(request, response, locationconfig);
        return;
    }

    if (contentType == "application/x-www-form-urlencoded") {
        handleUrlFormEncoded(request, response, locationconfig);
        return;
    }

    if (!locationconfig.cgi_pass.empty()) {
        LOG_INFO("Handling CGI for POST request");
        CgiHandler cgi_handler_instance;
        cgi_handler_instance.handle(request, response, locationconfig, request->getClientFd());
        return;
    }

    response->setStatusCode(200);
    response->setBody(request->getBody());
    LOG_INFO("200 OK - Request processed successfully");
}

std::string     PostHandler::parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store, std::string& filename) {
    std::string fileData;
    std::string partDelimiter = "--" + boundary;
    size_t start = 0;

    while ((start = data.find(partDelimiter, start)) != std::string::npos) {
        start += partDelimiter.length();
        size_t end = data.find(partDelimiter, start);
        
        if (end == std::string::npos) {
            break; // No more parts
        }
        
        std::string part = data.substr(start, end - start);
        size_t namePos = part.find("name=\"");
        size_t filenamePos = part.find("filename=\"");
        
        if (namePos != std::string::npos) {
            namePos += 6; // Length of "name=\""
            size_t nameEnd = part.find("\"", namePos);
            std::string name = part.substr(namePos, nameEnd - namePos);
            
            if (filenamePos != std::string::npos) {
                // It's a file field
                filenamePos += 10; // Length of "filename=\""
                size_t filenameEnd = part.find("\"", filenamePos);
                filename = post_upload_store;
                if (filename[filename.size() - 1] == '/')
                    filename += part.substr(filenamePos, filenameEnd - filenamePos);
                else
                    filename += "/" + part.substr(filenamePos, filenameEnd - filenamePos);

                size_t valuePos = part.find("\r\n\r\n", filenameEnd) + 4; // Skip headers
                fileData = part.substr(valuePos, part.size() - valuePos - 2); // Without the last two characters
            } else {
                // Normal form field
                size_t valuePos = part.find("\r\n\r\n", nameEnd) + 4; // Skip headers
                std::string value = part.substr(valuePos, part.size() - valuePos - 2); // Without the last two characters
                // You can handle normal fields here if needed
            }
        }        
        start = end;
    }
    return fileData;
    
}

std::map<std::string, std::string> PostHandler::parseUrlFormData(const std::string& body)
{
    std::map<std::string, std::string> formData;
    std::stringstream ss(body);
    std::string pair;

    while (std::getline(ss, pair, '&')) {  // Divide las parejas clave=valor separadas por '&'
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            formData[key] = value;  // Guardamos el par clave-valor en el mapa
        }
    }

    return formData;
}

bool PostHandler::saveFile(const std::string& filename, const std::string& data)
{
    std::ofstream outFile(filename.c_str(), std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(data.c_str(), data.size());
        outFile.close();
        return true; 
    } else {
        std::cerr << "Error: " << filename << std::endl;
        return false; 
    }
} 





