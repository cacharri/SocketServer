
#include "includes/Server.hpp"


int main()
{
	ConfigParser parser;
    std::vector<ServerConfig> servers = parser.parseConfigFile("server.config");

	 std::vector<ServerConfig>::const_iterator iter;
    for (iter = servers.begin(); iter != servers.end(); ++iter) {
        const ServerConfig& serverConfig = *iter;
        Server server(serverConfig);
	}
	// std::cout << "Starting server..." << std::endl;
	// std::cout << "Binding socket..." << std::endl;
	// BindingSocket	bs(AF_INET, SOCK_STREAM, 0, 8000, INADDR_ANY);
	// std::cout << "Listening socket..." << std::endl;
	// ListeningSocket	ls(AF_INET, SOCK_STREAM, 0, 8001, INADDR_ANY, 10);
	// std::cout << "Running Succeeded !" << std::endl;

}