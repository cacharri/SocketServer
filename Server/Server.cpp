#include "Server.hpp"

Server::Server(const ServerConfig& serverConfig)
    : MotherSocket(AF_INET, SOCK_STREAM, 0, serverConfig.port, serverConfig.host),
      buffer(new char[INITIAL_BUFFER_SIZE]),
      bufferSize(INITIAL_BUFFER_SIZE)
{
    std::cout << "<Server>:\n\t- " << serverConfig.host << "\n\t- " << serverConfig.port << std::endl;
    // Copiarse la configuracion en formato struct serverConfig (con una funcion statica de configparser)
    ConfigParser::copyServerConfig(serverConfig, Server::config);

    // Iteramos en el map de endpoints de struct serverConfig
    std::map<std::string, LocationConfig>::const_iterator it;
    for (it = config.locations.begin(); it != config.locations.end(); ++it) {
        const std::string& endpoint = it->first; // obtenemos ruta
        const LocationConfig& locConfig = it->second; // Obtenemos struct LocationConfig

        // Anadimos las reglas para un nuevo endpoint.
        // Construimos con el constructor por iterator de set() para formar un set de string de methods disponibles allowMethods
        std::set<std::string> allowedMethods(locConfig.limit_except.begin(), locConfig.limit_except.end());
        // Anadimos el enpoint path ('route' en) al mapa routes de router con los methodos allowedMethods y la funcion que gestiona ese endpoint
          // Verificar si GET está permitido y asociarlo a la función adecuada
        if (std::find(locConfig.limit_except.begin(), locConfig.limit_except.end(), "GET") != locConfig.limit_except.end()) {
            router.addRoute(endpoint, locConfig, setBodyWithFile);
        }

        // Verificar si POST está permitido y asociarlo a la función de manejar POST
        // Acceder como un método de instancia nos permite acceder a los datos de la instancia, organicacion,
        /*if (std::find(locConfig.limit_except.begin(), locConfig.limit_except.end(), "POST") != locConfig.limit_except.end()) {
            router.addRoute(endpoint, locConfig, handlePostRequest);
        }*/
    }
}

Server::~Server()
{
    delete[] buffer;
}

void    Server::init()
{
    toPassiveSocket(10);
    setNonBlocking();
}

void Server::launch()
{
    pollfd serverPollFd = {getSocketFd(), POLLIN, 0};
    fds.push_back(serverPollFd);

    while (42)
    {
        std::cout << "Waiting for events... Current clients: "<< (fds.size() - 1) << std::endl;
        int pollCount = poll(&fds[0], fds.size(), -1); // -1 se puede cambiar a un numero que seria el tiempo de espera en ms
        if (pollCount == -1)
        {
            ServerError error("Poll failed");
            LOG_EXCEPTION(error);
            continue;
        }

        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == getSocketFd())
                {
                    //LOG("New client !");
                    acceptClient();
                }
                else
                {
                    //LOG("Handling client");
                    handleClient(i);
                }
            }
            else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                removeClient(i);
        }
    }
}

// Función para parsear el cuerpo de la solicitud en pares clave-valor
std::map<std::string, std::string> parseFormData(const std::string& body) {
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

void Server::acceptClient() {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientFd = accept(getSocketFd(), (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ServerError error("Accept incoming socket failed !");
            LOG_EXCEPTION(error);
        }
        return;
    }

    // Set the new client socket to non-blocking
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(clientFd);
        ServerError error("Failed to set client socket to non-blocking !");
        LOG_EXCEPTION(error);
    }

    pollfd clientPollFd = {clientFd, POLLIN, 0};
    fds.push_back(clientPollFd);

    std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
}


std::string Server::receiveMessage(int clientSocket) {
    std::string message;
    ssize_t bytesRead;
    size_t totalBytesRead = 0;

    do {
        if (totalBytesRead >= bufferSize)
        {
            // Resize buffer if needed
            size_t newSize = bufferSize * 2;
            char* newBuffer = new char[newSize];
            std::copy(buffer, buffer + bufferSize, newBuffer);
            delete[] buffer;
            buffer = newBuffer;
            bufferSize = newSize;
        }

        bytesRead = recv(clientSocket, buffer + totalBytesRead, bufferSize - totalBytesRead, 0);
        if (bytesRead > 0)
            totalBytesRead += bytesRead;

        else if (bytesRead == 0)// Connection closed
            break;

        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            ServerError error("Error reading from socket !");
            LOG_EXCEPTION(error);
        }
    } while (bytesRead > 0);

    message.assign(buffer, totalBytesRead);
    return message;
}

std::string Server::receiveMessage(int clientSocket, size_t contentLength) {
    std::string message;
    ssize_t bytesRead;
    size_t totalBytesRead = 0;

    while (totalBytesRead < contentLength) {
        bytesRead = recv(clientSocket, buffer + totalBytesRead, contentLength - totalBytesRead, 0);
        if (bytesRead > 0) {
            totalBytesRead += bytesRead;
        } else if (bytesRead == 0) {
            break;  // Conexión cerrada
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ServerError error("Error reading from socket!");
            LOG_EXCEPTION(error);
            break;
        }
    }

    message.assign(buffer, totalBytesRead);
    return message;
}

void Server::sendResponse(int clientSocket, const std::string& response)
{
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent < 0)
    {
            ServerError error("Failed to send response !");
            LOG_EXCEPTION(error);
    }
}

void Server::removeClient(size_t index)
{
    close(fds[index].fd);
    fds.erase(fds.begin() + index);
}

void Server::handlePostRequest(Request& request, int clientFd, Response& response) {
    std::cout << "Received POST request" << std::endl;
    std::string contentType = request.getHeader("Content-Type");
    if (contentType == "application/x-www-form-urlencoded") {
        std::string requestBody = request.getBody();
        
        // Parsear el cuerpo del formulario
        std::map<std::string, std::string> formData = parseFormData(requestBody);
        
        // Extraer datos del formulario
        std::string name = formData["name"];
        std::string email = formData["email"];
        std::string age = formData["age"];
        std::string gender = formData["gender"];
        std::string comments = formData["comments"];

        // Generar respuesta
        std::string responseBody = "<html><body>";
        responseBody += "<h1>Formulario Recibido</h1>";
        responseBody += "<p>Nombre: " + name + "</p>";
        responseBody += "<p>Email: " + email + "</p>";
        responseBody += "<p>Edad: " + age + "</p>";
        responseBody += "<p>Género: " + gender + "</p>";
        responseBody += "<p>Comentarios: " + comments + "</p>";
        responseBody += "</body></html>";

        response.setStatus(200, "OK");
        response.setBody(responseBody);
    } else {
        response.setStatus(400, "Bad Request");
        response.setBody("Unsupported Content-Type");
    }
}

void Server::handleClient(size_t index) {
    int         clientFd = fds[index].fd;
    Response    response;
    try { 
        std::string request_str = receiveMessage(clientFd);
        if (request_str.empty())
        {
            removeClient(index);
            return;
        }

        Request request(request_str);
        if (request.getMethod() == "POST") {
            std::cout << "Handling POST request" << std::endl;
            handlePostRequest(request, clientFd, response);
        }
        else {
            router.route(request, response);
        }
        response.setStatus(200, "OK");
        response.setHeader("Content-Type", "text/html");
        response.setContentLength();
        
        sendResponse(clientFd, response.toString());
    }
    catch (const std::exception& e)
    {
        response.setStatus(500, "Internal Server Error"); // Internal Server Error
        response.setBody("500 Internal Server Error");
        sendResponse(clientFd, response.toString());
        removeClient(index);
        LOG("Error handling client: " + std::string(e.what()));
    }
}

//Exceptions handling
Server::ServerError::ServerError(const std::string& error): error_string(error)
{
}

Server::ServerError::~ServerError() throw()
{
}

const char * Server::ServerError::what() const throw()
{
	return error_string.c_str();
}