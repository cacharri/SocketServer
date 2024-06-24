
#ifndef MOTHERSOCKET_HPP
# define MOTHERSOCKET_HPP

# include <sys/types.h> // variables for paramters to socket()
# include <sys/socket.h> // socket func
# include <netinet/in.h> // sockaddr_in structs 
# include <arpa/inet.h>

# include <stdio.h>
# include <netdb.h>
# include <stdlib.h>

# include <iostream>
# include <sstream>
# include <unistd.h>
# include <fcntl.h>
# include <map>

# include <vector>

class MotherSocket
{
    public:
        //MotherSocket();
        MotherSocket(int domain, int service, int protocol, int port, unsigned long interface);
        MotherSocket(MotherSocket& copy);
        ~MotherSocket();

        struct sockaddr_in	get_address();
        int					get_socket();
        int                 get_queue();
        void				test_connection(int socket_to_test);

        void	            to_passive_socket(int queuelimit);
        void                to_active_socket();

        //int     get_info();

    private:
        void	            set_opt(int input_socket, int optionlevel, int option);

        int					socketfd;
        struct sockaddr_in	socketstruct;
        int                 queue;

	
        
};

#endif