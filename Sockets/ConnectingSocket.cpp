#include "ConnectingSocket.hpp"

ConnectingSocket::ConnectingSocket(int domain, int service, int protocol, int port, unsigned long interface): SimpleSocket(domain, service, protocol, port, interface)
{
    //cout << "Parametric constructor ConnectingSocket" << endl;
    connect_to_network(get_socket(), get_address());
    test_connection( connection );
}

void    ConnectingSocket::connect_to_network(int socket, struct sockaddr_in address)
{
    connection = connect(socket, (struct sockaddr *)&address, sizeof(address));
}

ConnectingSocket::~ConnectingSocket()
{
}


int ConnectingSocket::get_connection()
{
    return (connection);
}