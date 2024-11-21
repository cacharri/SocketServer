
#include "../Client/Client.hpp"
#include "../Http/Http.hpp"  // Si ServerError está definido aquí


Client::Client(ClientInfo* session_info)
    : clientFd(session_info->pfd.fd)
    , session_info(session_info)
    , request(NULL)
    , response(NULL) {}

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

void Client::sendRResponse(int clientSocket, const std::string& response)
{
    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), MSG_CONFIRM);
    if (bytesSent < 0)
    {
        try {
            throw std::runtime_error("Failed to send response!");
        } catch (const std::exception& e) {
            LOG_EXCEPTION(e); // Aquí se usa 'e' en lugar de 'error'
}

    }
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
            LOG_INFO("Failed to read request data.");

            // Crear y configurar la respuesta de error 413
            Response errorResponse;
            errorResponse.setStatusCode(413); // Payload Too Large
            try {
                errorResponse.setBody(readFile(request->getErrorPagePath(413))); // Cargar página de error
            } catch (const std::exception& e) {
                LOG("Failed to read error page: " + std::string(e.what()));
                errorResponse.setBody("<html><body><h1>413 Payload Too Large</h1></body></html>");
            }

            LOG_INFO("Sending 413 response...");
            sendRResponse(session_info->pfd.fd, errorResponse.toString());
            return false; // Indicar fallo
        }

        LOG_INFO("Request data read successfully.");

        if (!response) {
            LOG_INFO("Creating Response object...");
            response = new Response();
        }

        return true;
    } catch (const std::exception& e) {
        LOG("Exception caught during client handling: " + std::string(e.what()));

        // Crear y configurar la respuesta de error 500
        Response errorResponse;
        errorResponse.setStatusCode(500); // Internal Server Error
        try {
            errorResponse.setBody(readFile(request->getErrorPagePath(500))); // Cargar página de error
        } catch (const std::exception& ex) {
            LOG("Failed to read error page for 500: " + std::string(ex.what()));
            errorResponse.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
        }

        LOG_INFO("Sending 500 response...");
        sendRResponse(session_info->pfd.fd, errorResponse.toString());
        return false; // Indicar fallo
    }
}

