#include "../Http/Http.hpp"
#include "../Logger/Logger.hpp"

// variables locales al archivo
namespace {
    Http* g_http_instance = NULL;  

    void signal_handler(int signum) {
        if (g_http_instance) {
            g_http_instance->free_servers();
            exit(signum);
        }
    }
}


// FORMA DE COPLIEN
Http::Http()
{

}

Http::Http(const Http& copy)
{
    (void)copy;
}
Http&   Http::operator=(const Http& assign)
{
    (void)assign;
    return (*this);
}

Http::~Http()
{
    quitSignal();
}

// CLASE DE ERROR de Htpp
Http::ServerError::ServerError(const std::string& error_descr)
{
    error_string = error_descr;
}

Http::ServerError::~ServerError() throw()
{
    
}

const char * Http::ServerError::what() const throw()
{
	return error_string.c_str();
}

// METHODOS
void    Http::configure(const std::string&  configfile)
{
    std::vector<ServerConfig> serverConfigs = ConfigParser::parseServerConfigFile(configfile);

    if (serverConfigs.empty())
        LOG_EXCEPTION(ServerError("Configuration file does not have a valid Server Configuration."));
    
    try{
        
        for (std::vector<ServerConfig>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) {
            Server* server = new Server(*it);
            servers.push_back(server);
        }

    } catch (const std::exception& e) {
        LOG_EXCEPTION(ServerError(e.what()));
    }
    if (servers.empty())
        LOG_EXCEPTION(ServerError("No servers could be started."));
}

void    Http::free_servers()
{
    LOG_INFO("Shutting down servers ...");
    for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
        delete *it;
    
    servers.clear();
}

void    Http::quitSignal()
{
    free_servers();
}

void    Http::launch_all()
{
    LOG_INFO("All servers are launching... Press CTRL+C to quit");
    
    for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
        (*it)->init();

    // Creamos un vector los pollfds de nuestros socket pasivos de nuestros servidores
    std::vector<pollfd> master_fds;
    // Creamos un vector los pollfds de nuestros socket activos de nuestros servidores
    // Anadimos el socket de escucha(pasivo) de cada servidor
    for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
    {
        pollfd server_fd;
        server_fd.fd = (*it)->getPassiveSocketFd();
        std::cout << "Append to master_fds: " << (*it)->getPassiveSocketFd() << std::endl;
        server_fd.events = POLLIN;
        server_fd.revents = 0;
        master_fds.push_back(server_fd);
    }

    // Bucle principale
    while (42)
    {
        for (std::list<Server*>::iterator it_servers = servers.begin(); it_servers != servers.end(); it_servers++)
        {
            for (std::vector<ClientInfo*>::iterator it_clients = (*it_servers)->clients.begin(); it_clients != (*it_servers)->clients.end(); it_clients++)
                master_fds.push_back((*it_clients)->pfd);
        }

        int poll_count = poll(master_fds.data(), master_fds.size(), -1);
        
        if (poll_count < 0)
        {
            if (errno != EINTR)
                LOG("Poll error in master loop");
            continue;
        }

        for (std::vector<pollfd>::iterator iter_poll_fd = master_fds.begin(); iter_poll_fd != master_fds.end(); iter_poll_fd++)
        {
            // if ((*iter_poll_fd).revents & POLLIN)
            // {
            //     (*it)->handleConnections(); 
            // }
        }
    }
}

void Http::setupSignalHandlers(Http* http)
{
    g_http_instance = http;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}



