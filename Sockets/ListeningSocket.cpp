#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int input_backlog): BindingSocket(domain, service, protocol, port, interface)
{
    backlog = input_backlog;
    start_listening();
    test_connection(listening);
}

void    ListeningSocket::start_listening()
{
    listening = listen(get_socket(), backlog);
}


int ListeningSocket::get_backlog()
{
    return (backlog);
}

ListeningSocket::~ListeningSocket()
{
    
}