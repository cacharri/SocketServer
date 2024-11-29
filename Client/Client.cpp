
#include "../Client/Client.hpp"
#include "../Http/Http.hpp"  // Si ServerError está definido aquí


Client::Client(ClientInfo* session_info)
    : clientFd(session_info->pfd.fd)
    , session_info(session_info)
    , request(NULL)
    , response(NULL)
{

    response = new Response();
}

Client::~Client() {
    // Liberar memoria de request y response si fueron inicializados
    if (request) {
        delete request;
        request = NULL;
    }
    if (response) {
        delete response;
        response = NULL;
    }
}

std::string Client::to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Request* Client::getRequest() {
    if (!request) {
        throw ClientError("Request object not initialized.");
    }
    return request;
}

Response* Client::getResponse() {
    if (!response) {
        throw ClientError("Response object not initialized.");
    }
    return response;
}

    
bool Client::shouldKeepAlive() const {
    return session_info ? session_info->keepAlive : false;
}

void Client::setLastActivity() const {
    if (session_info) {
        session_info->lastActivity = time(NULL);
    }
}

time_t Client::getLastActivity() const {
    return session_info ? session_info->lastActivity : 0;
}

bool Client::HandleConnection() {
    try {
        LOG_INFO("Starting to handle connection for FD: " + to_string(session_info->pfd.fd));


        if (!session_info || session_info->pfd.fd < 0) {
            throw ClientError("Invalid client session or file descriptor.");
        }

        LOG_INFO("Creating Request object...");
        if (!request) {
            request = new Request(*session_info);
        }
        LOG_INFO("Reading data from request...");
        if (!request->readData(session_info->pfd.fd, session_info->client_max_body_size)) {
            //LOG_INFO("Failed to read request data.");
            response->setStatusCode(400);
            return false; // Indicar fallo
        }
        LOG_INFO("Request data read successfully.");
        return true;
    } catch (const std::exception& e) {
        LOG("Exception caught during client handling: " + std::string(e.what()));

        // Crear y configurar la respuesta de error 500
        response->setStatusCode(500); // Internal Server Error
        return false; // Indicar fallo
    }
}

