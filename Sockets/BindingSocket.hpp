#ifndef BINDINGSOCKET_HPP
# define BINDINGSOCKET_HPP

#include "SimpleSocket.hpp"

class BindingSocket:  public SimpleSocket
{
private:
    int socket_name;
public:
    //BindingSocket();
    BindingSocket(int domain, int service, int protocol, int port, unsigned long interface);
    ~BindingSocket();

    void    connect_to_network(int socket, struct sockaddr_in address);
    int     get_connection();
};


#endif