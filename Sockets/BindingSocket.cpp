
#include "BindingSocket.hpp"


BindingSocket::BindingSocket(int domain, int service, int protocol, int port, unsigned long interface): SimpleSocket(domain, service, protocol, port, interface)
{
    //cout << "Parametric constructor BindinSocket" << endl;
    connect_to_network(get_socket(), get_address());
    test_connection( socket_name );
}

void    BindingSocket::connect_to_network(int socket, struct sockaddr_in address)
{
    socket_name = bind(socket, (struct sockaddr *)&address, sizeof(address));
}

BindingSocket::~BindingSocket()
{
    
}


int BindingSocket::get_connection()
{
    return (socket_name);
}