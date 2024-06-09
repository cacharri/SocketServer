#include "ClientDumbServer.hpp"

#include <iostream>
#include <sstream>
#include <string.h>
//#include <cstring>

ClientDumbServer::ClientDumbServer(): SimpleServer(AF_INET,
    SOCK_STREAM, 0, 8000, INADDR_ANY, 10)
{
    launch();   
}

ClientDumbServer::~ClientDumbServer()
{

}

// ListeningSocket *ClientDumbServer::get_listening_socket()
// {
//     return (socket);
// }

void    ClientDumbServer::accepter()
{
    struct sockaddr_in address = get_listening_socket()->get_address(); // get_listening_socket is inherited from SimpleServer and returns a ListeningSocket pointer object.
    int addrlen = sizeof(address);
    socket = accept(get_listening_socket()->get_socket(), (struct sockaddr *)&address, (socklen_t *)&addrlen);


    read(socket, buffer, sizeof(buffer));
    std::cout << "Recieved connection at: " << address.sin_addr.s_addr << std::endl;
    //printf("%s", buffer);
}

void    ClientDumbServer::handler()
{
    std::cout << buffer << std::endl;
}

void ClientDumbServer::responder()
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
        "setInterval(randomizeTextPosition, 2000); // Change position and color every 2 seconds"
        "</script>"
        "</body>"
        "</html>";

    

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "\r\n"
             << body;

    ssize_t bytes = write(socket, response.str().c_str(), response.str().size());
    if (bytes < 0)
        perror("Failed to send response");
    else if (static_cast<size_t>(bytes) < response.str().size())
        std::cerr << "Warning: Not all bytes sent, sent only " << bytes << " bytes.\n";
    close(socket);
}


void    ClientDumbServer::launch()
{
    while (42)
    {
        std::cout << "=======   WAITING  ========" << std::endl;
        accepter();
        handler();
        responder();
        std::cout << "=======   DONE  ========" << std::endl;
    }
}