#include "SimpleServer.hpp"

SimpleServer::SimpleServer(int domain, int service, int protocol, int port, unsigned long interface, int input_backlog)
{
    listening_socket = new ListeningSocket(domain, service, protocol, port, interface, input_backlog);
}

SimpleServer::~SimpleServer()
{
    delete listening_socket;
}

ListeningSocket *SimpleServer::get_listening_socket()
{
    return (listening_socket);
}