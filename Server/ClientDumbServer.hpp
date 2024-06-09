#ifndef CLIENTDUMBSERVER_HPP
# define CLIENTDUMBSERVER_HPP

# include "SimpleServer.hpp"
# include <unistd.h>

class ClientDumbServer: public SimpleServer
{
    private:
        char    buffer[20000];
        int     socket;
        void    accepter();
        void    handler();
        void    responder();

    public:
        ClientDumbServer();
        ~ClientDumbServer();

        void    launch();
        //ListeningSocket *get_listening_socket();
};

#endif