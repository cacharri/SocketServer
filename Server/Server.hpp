/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <smagniny@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 13:58:50 by Smagniny          #+#    #+#             */
/*   Updated: 2024/12/02 16:32:08 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

// Forward declarations
class Request;
class Response;

#include "../Sockets/MotherSocket.hpp"
#include "../Config/ConfigParser.hpp"
#include "../Handlers/cgiHandler.hpp"
#include "../Response/Response.hpp"
#include "../Request/Request.hpp"
#include "../Router/Router.hpp"
#include "../Handlers/Handlers.hpp"
#include "../Client/Client.hpp"
#include <functional>
#include <ctime>
#include <arpa/inet.h>
#include <netinet/in.h>
/*

1. The Server class inherits from MotherSocket and encapsulates server-specific functionality.  
2. It uses poll() to handle multiple client connections efficiently, allowing for non-blocking I/O.
3. The launch() method now contains the main event loop, which continuously checks for incoming connections and client activity.
4. The acceptClient() method handles new client connections, setting them to non-blocking mode and adding them to the fds vector.
5. The handleClient() method processes incoming client requests and sends responses.
6. The removeClient() method properly closes and removes disconnected clients from the fds vector.

*/

class Server : public MotherSocket
{

public:
    std::vector<ClientInfo*> clients;
    std::vector<CgiProcess*> cgis;

    Server(const ServerConfig& serverConfig);
    ~Server();

    // Core functions

    //void        launch();
    void        init();
    void        acceptClient(int listenFd);
    void        sendResponse(int clientSocket, const std::string& response);
    void        handleClient(ClientInfo* client);
    void        handleCGIresponse(CgiProcess* cgi);
    void        removeClient(ClientInfo* client);
    bool        IsTimeout(ClientInfo* client);
    bool        IsTimeoutCGI(CgiProcess* cgi);

    class ServerError: public std::exception {
        private:
            std::string error_string;
        public:
            ServerError(const std::string& error);
            virtual const char* what() const throw();
            virtual ~ServerError() throw();
    };

private:
    Router                  router;
    ServerConfig            config;
    static const time_t CONNECTION_TIMEOUT = 1;
    static const time_t CGI_TIMEOUT = 2;      

    // Headers functions
    void        analyzeBasicHeaders(const Request* request, Response* response, ClientInfo* client);
    void        setErrorPageFromStatusCode(Response*    response);
    std::string getErrorPagePath(Response*    response);
    bool        IsCgiRequest(Response* res);
    std::string getFormattedDate();

    
    // Disable copy constructor and assignment operator
    Server(const Server&);
    Server& operator=(const Server&);

};

bool isValidHostHeader(const std::string &hostHeader);


#endif
