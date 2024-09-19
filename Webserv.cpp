#include "Server/Server.hpp"
#include "Sockets/MotherSocket.hpp"
#include "Config/ServerConfig.hpp"
#include "Logger/Logger.hpp"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <signal.h>

std::vector<Server*> g_servers;

void signalHandler(int signum)
{
    std::cout << "\nShutting down servers..." << std::endl;
    for (size_t i = 0; i < g_servers.size(); ++i)
        delete g_servers[i];

    g_servers.clear();
    exit(signum);
}

int main(int argc, char **argv) {
    

    if (argc != 2) 
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }



    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        ConfigParser parser;
        std::vector<ServerConfig> serverConfigs = parser.parseConfigFile(argv[1]);

        if (serverConfigs.empty()) {
            std::cerr << "No valid server configurations found." << std::endl;
            return 1;
        }

        for (std::vector<ServerConfig>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) {
            try {
                Server* server = new Server(*it);
                g_servers.push_back(server);
            } catch (const std::exception& e) {
                std::cerr << "Failed to create server: " << e.what() << std::endl;
            }
        }

        if (g_servers.empty()) {
            std::cerr << "No servers could be started." << std::endl;
            return 1;
        }

        std::cout << "All servers started. Press Ctrl+C to stop." << std::endl;

        // Run all servers
        for (size_t i = 0; i < g_servers.size(); ++i) {
            g_servers[i]->init();
            g_servers[i]->launch();
        }

        // Wait for all servers to finish (this point is reached only if all servers stop)
        for (size_t i = 0; i < g_servers.size(); ++i) {
            delete g_servers[i];
        }
        g_servers.clear();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}