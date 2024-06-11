#include "ClientDumbServer.hpp"

#include <iostream>
#include <sstream>
#include <string.h>
#include <poll.h>
#include <arpa/inet.h>
//#include <cstring>

ClientDumbServer::ClientDumbServer(): SimpleServer(AF_INET, SOCK_STREAM, 0, 8000, INADDR_ANY, 10) {
    memset(client_sockets, -1, sizeof(client_sockets));
    launch();   
}

ClientDumbServer::~ClientDumbServer() {

}

void ClientDumbServer::accepter() {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int client_socket = accept(get_listening_socket()->get_socket(), (struct sockaddr *)&address, &addrlen);
    if (client_socket < 0) {
        perror("accept");
        close(get_listening_socket()->get_socket());
        exit(EXIT_FAILURE);
    }
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), ip_address, INET_ADDRSTRLEN);
    std::cout << "Received connection at: " << client_socket << " from IP: " << ip_address << std::endl;

    for (int i = 0; i < 10; ++i) {
        if (client_sockets[i] == -1) {
            client_sockets[i] = client_socket;
            break;
        }
    }
}

void ClientDumbServer::reader(int client_socket) {
    char buffer[1024] = {0};
    ssize_t bytes = read(client_socket, buffer, sizeof(buffer));
    if (bytes < 0) {
        perror("Failed to read from client");
    } else if (bytes == 0) {
        std::cout << "Client disconnected: " << client_socket << std::endl;
        close(client_socket);
        for (int i = 0; i < 10; ++i) {
            if (client_sockets[i] == client_socket) {
                client_sockets[i] = -1;
                break;
            }
        }
    } else {
        std::cout << "Received from client " << client_socket << ": " << buffer << std::endl;
        // Implement logic to process the client's request
        writer(client_socket);
    }
}


void ClientDumbServer::writer(int client_socket) {
    std::string body = "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>Random Moving Text</title>"
        "<style>"
        "    body {"
        "        height: 100vh;"
        "        margin: 0;"
        "        display: flex;"
        "        overflow: hidden;"
        "        background: #f0f0f0;"
        "    }"
        "    .moving-text {"
        "        font-size: 40px;"
        "        font-weight: bold;"
        "        position: absolute;"
        "        transition: all 2s ease;"
        "    }"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"moving-text\" id=\"movingText\">Hello From Server</div>"
        "<script>"
        "function randomizeTextPosition() {"
        "    const textElement = document.getElementById('movingText');"
        "    const maxWidth = window.innerWidth - textElement.clientWidth;"
        "    const maxHeight = window.innerHeight - textElement.clientHeight;"
        "    const randomX = Math.floor(Math.random() * maxWidth);"
        "    const randomY = Math.floor(Math.random() * maxHeight);"
        "    const randomColor = '#' + Math.floor(Math.random()*16777215).toString(16);"
        "    textElement.style.left = `${randomX}px`;"
        "    textElement.style.top = `${randomY}px`;"
        "    textElement.style.color = randomColor;"
        "}"
        "setInterval(randomizeTextPosition, 2000);"
        "</script>"
        "</body>"
        "</html>";

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "\r\n"
             << body;

    ssize_t bytes = write(client_socket, response.str().c_str(), response.str().size());
    if (bytes < 0) {
        perror("Failed to send response");
    } else if (static_cast<size_t>(bytes) < response.str().size()) {
        std::cerr << "Warning: Not all bytes sent, sent only " << bytes << " bytes.\n";
    }
}

void ClientDumbServer::launch() {
    struct pollfd fds[10 + 1]; 
    memset(client_sockets, -1, sizeof(client_sockets));

    while (true) {
        memset(fds, 0, sizeof(fds));
        fds[0].fd = get_listening_socket()->get_socket();
        fds[0].events = POLLIN; 
        int nfds = 1;

        for (int i = 0; i < 10; ++i) {
            if (client_sockets[i] != -1) {
                fds[nfds].fd = client_sockets[i];
                fds[nfds].events = POLLIN; 
                ++nfds;
            }
        }

        int poll_result = poll(fds, nfds, -1); 

        if (poll_result == -1) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            accepter();
        }

        for (int i = 1; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                reader(fds[i].fd);
            }
        }
    }
}

