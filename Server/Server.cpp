/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: smagniny <santi.mag777@student.42madrid    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/13 00:28:54 by smagniny          #+#    #+#             */
/*   Updated: 2024/06/13 12:09:51 by smagniny         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"


Server::Server():
    //CHANGE THIS VALUES FOR SOCKET CONFIGURATION THROUGH CONSTRUCTOR
    MotherSocket(AF_INET, SOCK_STREAM, 0, 8000, INADDR_ANY) 
{
    MotherSocket::to_passive_socket(10); // make created socket a passive with bind and listen funcs (vs connect for socket client)
    launch();
}

Server::~Server()
{
}

void Server::accepter()
{
    //creation of tmp sockaddr_in struct to fill later for incoming request from listener socket
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    
    //filling sockadrr_in through new client socket received at listener socket
    int client_fd = accept(get_socket(), (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) {
        perror("accept");
        close(get_socket());
        exit(EXIT_FAILURE);
    }
    //display struct addr details
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), ip_address, INET_ADDRSTRLEN);
    std::cout << "ListenerServerSocket: Received Connection: from " << ip_address << std::endl;

    //append to pollfds vector
    struct pollfd client_socketstruct = {client_fd, POLLIN, 0};
    fds.push_back(client_socketstruct);
    std::cout << "NEW ClientSocket FD: " << client_socketstruct.fd << std::endl;
}

void Server::reader(int client_socket)
{
    ssize_t bytes = read(client_socket, buffer, sizeof(buffer));
    
    std::cout << "reader output: "<< std::strerror(errno) << std::endl;
    if (bytes > 0)
    {
        std::cout << "Received data: \n" << std::string(buffer, bytes) << std::endl;
        writer(client_socket); // IMPORTANT esta funcion va a ser un objeto que va a manejar todo lo que es respuestas (see RFC error responses,status line creation, header creation, body creation)
    }
    else if (bytes == 0)
    {
        std::cout << "Client disconnected: FD " << client_socket << std::endl;
        close(client_socket); // Close the socket

        // Remove the client_socket from fds vector
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); )
        {
            if (it->fd == client_socket)
                it = fds.erase(it); // Erase and move to the next iterator
            else
                ++it;
        }
    }
    else
    {  // bytes < 0
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read failed");
            close(client_socket); // Close the socket only on unrecoverable errors

            // Remove the client_socket from fds vector
            for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); )
            {
                if (it->fd == client_socket)
                    it = fds.erase(it); // Erase and move to the next iterator
                else
                    ++it;
            }
        }
    }
}

void Server::writer(int client_socket)
{
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
    if (bytes < 0)
        perror("Failed to send response");
    else if (static_cast<size_t>(bytes) < response.str().size())
        std::cerr << "Warning: Not all bytes sent, sent only " << bytes << " bytes.\n";

}



void Server::launch()
{
    struct pollfd listen_fd = {get_socket(), POLLIN, 0};
    fds.push_back(listen_fd);

    while (42)
    {
        int poll_count = poll(fds.data(), fds.size(), -1);  // -1 es el arg para especificar el tiempo de polling
        if (poll_count == -1) {
            perror("poll failed");
            break;
        }

        for (size_t i = 0; i < fds.size(); i++)
        {
            if ((fds[i].revents & POLLIN) && fds[i].fd == listen_fd.fd)
            {
                std::cout << "accepter x1" << std::endl;
                accepter();                
            }
            else if (fds[i].revents & POLLIN && fds[i].fd)
            {
                std::cout << "reader x1" << std::endl;
                reader(fds[i].fd);
            }
                // Re-check if the current fd is still valid after handling
                //if (i < fds.size() && fds[i].fd == listen_fd.fd || fds[i].fd != -1)
                    //++i;  // Only increment if fd is still valid
            else if (i)
            {
                if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                {
                    std::cout << "close & erase: " << fds[i].fd << std::endl;
                    close(fds[i].fd);// aqui cierro un socket que podria seguir abierto
                    fds.erase(fds.begin() + i); 
                }
            }
                // This doesn't require incrementing i
                // close it (if necesary) and erase from vector
                //close(fds[i].fd);
        }
    }
}
