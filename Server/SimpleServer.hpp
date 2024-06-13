#ifndef SIMPLESERVER_HPP
# define SIMPLESERVER_HPP

# include "../includes/socklib.hpp"

class SimpleServer
{
    private:
        ListeningSocket *listening_socket;
        virtual void    accepter() =0;
        //virtual void    handler() =0;
        //virtual void    responder() =0;

    public:
        SimpleServer(int domain, int service, int protocol, int port, unsigned long interface, int input_backlog);
        ~SimpleServer();

        virtual void    launch() =0;
        ListeningSocket *get_listening_socket();
};

#endif