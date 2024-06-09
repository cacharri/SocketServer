#ifndef SIMPLESOCKET_HPP
# define SIMPLESOCKET_HPP

# include <stdio.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h>

class SimpleSocket
{
    public:
        //SimpleSocket();
        SimpleSocket(int domain, int service, int protocol, int port, unsigned long interface);
        SimpleSocket(SimpleSocket& copy);
        ~SimpleSocket();

        struct sockaddr_in get_address();
        int     get_socket();


        virtual void    connect_to_network(int socket, struct sockaddr_in address)=0;
        void    test_connection(int socket_to_test);

    private:
        int socketfd;
        struct sockaddr_in address;
        
};

#endif