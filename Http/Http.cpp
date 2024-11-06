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
        // (*it)->~Server();
        delete (*it);
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

    std::vector<pollfd> master_fds;
    
    for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
    {
        pollfd server_fd;
        server_fd.fd = (*it)->getPassiveSocketFd();
        server_fd.events = POLLIN;
        server_fd.revents = 0;
        master_fds.push_back(server_fd);
    }

    size_t num_servers = servers.size();
    size_t tempflag_printing = 0;
    size_t tempflag2_printing2 = 0;

    while (42)
    {
        master_fds.resize(num_servers);
        
        for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
        {
            for (std::vector<ClientInfo*>::iterator cli_it = (*srv_it)->clients.begin(); 
                cli_it != (*srv_it)->clients.end(); cli_it++)
            {
                pollfd  active_fd;
                active_fd.fd = (*cli_it)->pfd.fd;
                active_fd.events = POLLIN;
                active_fd.revents = 0;
                master_fds.push_back(active_fd);
            }
        }
        tempflag_printing = master_fds.size();
        if (tempflag_printing != tempflag2_printing2)
        {
            std::ostringstream  printing;
            printing << master_fds.size();
            printing << " FD are used" << std::endl;
            LOG_INFO(printing.str());
        }
        tempflag2_printing2 = tempflag_printing;

        int poll_count = poll(&master_fds[0], master_fds.size(), 1000);
        
        if (poll_count < 0)
        {
            if (errno != EINTR)
                LOG("Poll error in master loop !!!");
            continue;
        }


        size_t master_fd_index = 0;
        // Empezar por ver los eventos de los sockets pasivos
        for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); it++, master_fd_index++)
        {
            // Controlar socket pasivo
            if (master_fds[master_fd_index].revents & POLLIN)
            {
                (*it)->acceptClient();
            }
        }
        // Despues Controlar clientes anadidos (sockets activos) en todos los servidores. (partiendo  del iterador master_fd_index para acceder a los socket activos en mastr_fds)
        for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
        {
            std::vector<ClientInfo*>::iterator cli_it = (*srv_it)->clients.begin();
            //std::cout << "fd of client: " << (*cli_it)->pfd.fd << std::endl;
            while (cli_it != (*srv_it)->clients.end())
            {
                if (master_fd_index >= master_fds.size())
                {
                    LOG_INFO("Client has not been polled: Index out of bounds");
                    break;
                }
                if ((*srv_it)->IsTimeout(*cli_it))
                {
                    close((*cli_it)->pfd.fd);
                    delete  *cli_it;
                    cli_it = (*srv_it)->clients.erase(cli_it);
                    LOG_INFO("Client Timeouted");
                    continue;
                }
                if (master_fds[master_fd_index].revents & POLLIN)
                {
                    (*srv_it)->handleClient(*cli_it); // try catch error & removing pollfd from master_fds if crash;
                }
                else if (master_fds[master_fd_index].revents & (POLLERR | POLLHUP | POLLNVAL))
                {
                    close((*cli_it)->pfd.fd);
                    delete  *cli_it;
                    cli_it = (*srv_it)->clients.erase(cli_it);
                    LOG_INFO("Conexion Removed");
                    continue;
                }
                cli_it++;
                master_fd_index++;
            }
        }
    }
}

void Http::setupSignalHandlers(Http* http)
{
    g_http_instance = http;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}



