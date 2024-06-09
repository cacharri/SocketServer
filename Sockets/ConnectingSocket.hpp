
#ifndef CONNECTINGSOCKET_HPP
# define CONNECTINGSOCKET_HPP

# include "SimpleSocket.hpp"

class ConnectingSocket:  public SimpleSocket
{
private:
    int connection;
public:
    //ConnectingSocket();
    ConnectingSocket(int domain, int service, int protocol, int port, unsigned long interface);
    ~ConnectingSocket();

    void    connect_to_network(int socket, struct sockaddr_in address);
    int     get_connection();
};


#endif