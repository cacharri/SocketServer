#include "../includes/MotherSocket.hpp"


// default constructor
// MotherSocket::MotherSocket()
// {
//     //cout << "Default constructor MotherSocket" << endl;
//     address.sin_family  = AF_INET;
//     address.sin_port = htons(8000);
//     address.sin_addr.s_addr = htonl(ipv4?);
//     socketfd = socket(AF_INET, SOCK_STREAM, 0);
//     test_connection(socketfd);
// }

// parametric constructor
MotherSocket::MotherSocket(int domain, int service, int protocol, int port, unsigned long interface)
{
	//cout << "Parametric constructor MotherSocket" << endl;
	socketstruct.sin_family = domain;
	socketstruct.sin_port = htons(port); // changes byte order from unsigned short integer 16 bits
	socketstruct.sin_addr.s_addr = htonl(interface);// changes byte order from unsigned integer 32 bits

	socketfd = socket(domain, service, protocol);

	test_connection(socketfd);

	set_opt(socketfd, SOL_SOCKET, SO_REUSEADDR); //SPECIFY SOL_SOCKET LEVEL(tcp?) TO USE SO_REUSEADDR OPTION

	// Make the socket non-blocking
    if (fcntl(socketfd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl: Failed to set socket to non-blocking");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
}
//copy constrcutor
MotherSocket::MotherSocket(MotherSocket& copy)
{
}
//destructor
MotherSocket::~MotherSocket()
{
	//cout << "Destructor MotherSocket" << endl;
}

//methods

// int     MotherSocket::get_info()
// {
// 	struct addrinfo hints, *res, *p;
// 	memset(&hints, 0, sizeof hints);
// 	hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6, whichever
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE; // Fill in my IP for me

// 	getaddrinfo(NULL, "http", &hints, &res);
// 	for(p = res; p != NULL; p = p->ai_next) {
// 		// Create and bind socket with p->ai_addr and p->ai_addrlen
// 	}
// 	freeaddrinfo(res); // Free the linked list
// }

//getters

struct sockaddr_in  MotherSocket::get_address()
{
	return (socketstruct);
}

int MotherSocket::get_socket()
{
	return (socketfd);
}

int MotherSocket::get_queue()
{
    return (queue);
}


// Test connection func
void    MotherSocket::test_connection(int socket_to_test)
{
	if (socket_to_test < 0)
	{
		perror("Cannot read from socket :(");
		close(socket_to_test);
		exit(EXIT_FAILURE);
	}
}

//SOL_SOCKET as optionlevel and SO_REUSEADDR as option. (REUSEADDR binds to socket even if the socket is still in use +-~ )
// socketfd, index of the config option, name of the option,
void	MotherSocket::set_opt(int input_socket, int optionlevel, int option)
{
	int flagres = 1; 
	if (setsockopt(input_socket, optionlevel, option, &flagres, sizeof(int)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(input_socket);
        exit(1);
    }
}

void	MotherSocket::to_passive_socket(int queuelimit)
{
	// void	MotherSocket::naming_the_socket(int input_sockett struct sockaddr_in *socketstruct)
	// {
	// 	if (bind(input_socket, (struct sockaddr *)socketstruct, sizeof(*socketstruct)) == -1)
	// 	{
	// 		perror("Bind: Failed to name the socket");
	// 		close(socket);
	// 		exit(EXIT_FAILURE);
	// 	}
	// }
	if (bind(socketfd, (struct sockaddr *)&socketstruct, sizeof(socketstruct)) == -1)
	{
		perror("Bind: Failed to name the socket");
		close(socketfd);
		exit(EXIT_FAILURE);
	}

	if (listen(socketfd, queuelimit) != 0)
	{
		perror("listen: Failed to set passive socket");
		close(socketfd);
		exit(EXIT_FAILURE);
	}

}

void	MotherSocket::to_active_socket()
{
	if (connect(socketfd, (struct sockaddr *)&socketstruct, sizeof(socketstruct)) == -1)
	{
		perror("Bind: Failed to name the socket");
		close(socketfd);
		exit(EXIT_FAILURE);
	}
}


