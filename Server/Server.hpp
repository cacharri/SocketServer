/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 13:58:50 by Smagniny          #+#    #+#             */
/*   Updated: 2024/09/21 16:27:19 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Sockets/MotherSocket.hpp"
#include "../Config/ConfigParser.hpp"

#include "../Response/Response.hpp"
#include "../Request/Request.hpp"
#include "../Router/Router.hpp"
#include "../Handlers/Handlers.hpp"

#include <vector>
#include <poll.h>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <unistd.h>

/*

1. The MotherSocket class now uses exception handling for error management.
2. The Server class inherits from MotherSocket and encapsulates server-specific functionality.
3. Instead of a fixed large buffer, we now use a dynamic approach:
    The receiveMessage method in the Server class handles varying message sizes by resizing the buffer as needed.
    Start with a smaller initial buffer (4096 bytes).
    Read data in chunks.
    If the buffer is full, we resize it dynamically.
    We've added proper resource management (ex: deleting the buffer in the destructor).
    
5. Copy constructors and assignment operators are disabled to prevent accidental copying.

1. It uses poll() to handle multiple client connections efficiently, allowing for non-blocking I/O.
2. The launch() method now contains the main event loop, which continuously checks for incoming connections and client activity.
3. The acceptClient() method handles new client connections, setting them to non-blocking mode and adding them to the fds vector.
4. The handleClient() method processes incoming client requests and sends responses.
5. The removeClient() method properly closes and removes disconnected clients from the fds vector.
*/


class Server : public MotherSocket {
public:
    explicit Server(const ServerConfig& serverConfig);
    ~Server();

    void    launch();
    void    init();
    class ServerError: public std::exception {
        private:
            std::string error_string;
        public:
            ServerError(const std::string& error);
            virtual const char* what() const throw();
            virtual ~ServerError() throw();
    };

private:
    Router              router;
    ServerConfig        config;
    std::vector<pollfd> fds;
    static const size_t INITIAL_BUFFER_SIZE = 4096;
    char* buffer;
    size_t bufferSize;

    void acceptClient();
    void handleClient(size_t index);
    void sendResponse(int clientSocket, const std::string& response);
    std::string receiveMessage(int clientSocket);
    void removeClient(size_t index);

    // Disable copy constructor and assignment operator
    Server(const Server&);
    Server& operator=(const Server&);
};

#endif