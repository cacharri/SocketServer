#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP

# include "BindingSocket.hpp"


class ListeningSocket: public  BindingSocket
{
private:
    int backlog;
    int listening;
public:
    ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int input_backlog);
    ~ListeningSocket();


    void    start_listening();
    int     get_backlog();
};

#endif