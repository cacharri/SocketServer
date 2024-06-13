#ifndef CLIENTDUMBSERVER_HPP
# define CLIENTDUMBSERVER_HPP

# include "SimpleServer.hpp"
# include <unistd.h>

class ClientDumbServer: public SimpleServer
{
    private:
        int client_sockets[10];
        char    buffer[20000];
        int     socket;
        void    accepter();
       // void    handler();
        //void    responder();
        void    reader(int client_socket);
        void    writer(int client_socket);

    public:
        ClientDumbServer();
        ~ClientDumbServer();

        void    launch();
        
        //ListeningSocket *get_listening_socket();
};

#endif