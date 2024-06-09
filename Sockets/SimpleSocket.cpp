
#include "SimpleSocket.hpp"


// default constructor
// SimpleSocket::SimpleSocket()
// {
//     //cout << "Default constructor SimpleSocket" << endl;
//     address.sin_family  = AF_INET;
//     address.sin_port = htons(8000);
//     address.sin_addr.s_addr = htonl(ipv4?);
//     socketfd = socket(AF_INET, SOCK_STREAM, 0);
//     test_connection(socketfd);
// }
// parametric constructor
SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, unsigned long interface)
{
    //cout << "Parametric constructor SimpleSocket" << endl;
    address.sin_family = domain;
    address.sin_port = htons(port); // changes byte order from unsigned short integer 16 bits
    address.sin_addr.s_addr = htonl(interface);// changes byte order from unsigned integer 32 bits
    socketfd = socket(domain, service, protocol);
    test_connection(socketfd);
}
//copy constrcutor
SimpleSocket::SimpleSocket(SimpleSocket& copy)
{
}
//destructor
SimpleSocket::~SimpleSocket()
{
    //cout << "Destructor SimpleSocket" << endl;
}


//getters

struct sockaddr_in  SimpleSocket::get_address()
{
    return (address);
}

int SimpleSocket::get_socket()
{
    return (socketfd);
}


// Test connection virtual func
void    SimpleSocket::test_connection(int socket_to_test)
{
    if (socket_to_test < 0)
    {
        perror("Failed to connect...");
        exit(EXIT_FAILURE);
    }
}