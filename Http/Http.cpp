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
bool    Http::configure(const std::string&  configfile)
{
    std::vector<ServerConfig> serverConfigs = ConfigParser::parseServerConfigFile(configfile);

    if (serverConfigs.empty())
    {
        LOG("Configuration file does not have a valid Server Configuration.");
        return false;
    }
    try{ 
        for (std::vector<ServerConfig>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it) {
            Server* server = new Server(*it);
            servers.push_back(server);
        }

    } catch (const std::exception& e) {
        LOG(e.what());
        return false; 
    }
    if (servers.empty())
        LOG("No servers could be started.");
    return true;

}

void    Http::free_servers()
{
    for (std::list<Server*>::iterator it = servers.begin(); it != servers.end(); it++)
        delete (*it);
    LOG_INFO("Shutting down servers ...");
    servers.clear();
}

void    Http::quitSignal()
{
    free_servers();
}

void Http::setupSignalHandlers(Http* http)
{
    g_http_instance = http;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}


void    Http::loadServers(std::vector<pollfd>& master_fds, size_t&  num_servers)
{
    for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
    {
        std::vector<int> server_fds = (*srv_it)->getPassiveSocketFd();
        for (std::vector<int>::iterator fd_it = server_fds.begin(); fd_it != server_fds.end(); fd_it++)
        {
            pollfd server_fd;
            server_fd.fd = *fd_it;
            server_fd.events = POLLIN;
            server_fd.revents = 0;
            master_fds.push_back(server_fd);
            num_servers++;
        }
    }
}

void    Http::loadNewConnections(size_t& total_clients, std::vector<pollfd>& master_fds)
{
    // Iterar en nuestros vectores para repopular master_fds con socket_activos y fd de pipes.
    for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
    {
        // 1. Anadir Fd del pipe de un proceso CGI
        for (std::vector<ClientInfo*>::iterator cli_it = (*srv_it)->clients.begin(); 
            cli_it != (*srv_it)->clients.end(); cli_it++)
        {
            pollfd active_fd;
            active_fd.fd = (*cli_it)->pfd.fd;
            active_fd.events = POLLIN;
            active_fd.revents = 0;
            master_fds.push_back(active_fd);
            total_clients++;
        }
        for (std::vector<CgiProcess*>::iterator cgi_it = (*srv_it)->cgis.begin(); 
            cgi_it != (*srv_it)->cgis.end();)
        {
            std::cout << "New CGI PID: " << (*cgi_it)->pid << " of Client " << (*cgi_it)->client_fd << std::endl;
            if ((*srv_it)->IsTimeoutCGI(*cgi_it))
            {
                kill((*cgi_it)->pid, SIGKILL);  // Force kill
                // Clean up resources
                close((*cgi_it)->output_pipe_fd.fd);
                delete *cgi_it;
                cgi_it = (*srv_it)->cgis.erase(cgi_it);
                std::cout << "Se borro el cgiprocess por un timeout" << std::endl;
                continue;
            }
            pollfd pipe_fd;
            pipe_fd.fd = (*cgi_it)->output_pipe_fd.fd;
            //std::cout << "New fd " << pipe_fd.fd << " CGI to master_fds" << std::endl;
            pipe_fd.events = POLLIN;
            pipe_fd.revents = 0;
            master_fds.push_back(pipe_fd);
            ++cgi_it;
        }
    }
}

void    Http::CheckUserConnected(size_t& previous_clients_size, std::vector<pollfd>& master_fds)
{
    size_t tempflag_printing = 0;

    // Log FD count if changed
    tempflag_printing = master_fds.size();
    if (tempflag_printing != previous_clients_size)
    {
        std::ostringstream  printing;
        printing << master_fds.size();
        printing << " FD are used" << std::endl;
        LOG_INFO(printing.str());
    }
    previous_clients_size = tempflag_printing;
}


void    Http::CGI_events(size_t& cgi_index, std::vector<pollfd>& master_fds)
{
    // std::cout << "First cgi index : " << cgi_index << std::endl;
    // std::cout << "Master_fds size : " << master_fds.size() << std::endl;

    for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
    {
        std::vector<CgiProcess*>::iterator cgi_it = (*srv_it)->cgis.begin();
        while (cgi_it != (*srv_it)->cgis.end())
        {
            //std::cout << "Hello " << std::endl;
            if (cgi_index >= master_fds.size())
            {
                LOG_INFO("CGI has not been polled: Index out of bounds");
                break;
            }

            if (master_fds[cgi_index].revents & POLLIN)
            {
                LOG_INFO("CGI pipe has data available");
                // Leer datos en primero antes de comprobar el stauts del proceso
                (*srv_it)->handleCGIresponse(*cgi_it);
                
                int status;
                pid_t result = waitpid((*cgi_it)->pid, &status, WNOHANG);
                
                if (result > 0)
                {
                    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                        LOG_INFO("CGI process finished successfully");
                    else
                    {
                        std::ostringstream out;
                        out << WEXITSTATUS(status);
                        LOG_INFO("CGI process failed with status: " + out.str());
                        Response errorResponse;
                        errorResponse.setStatusCode(500);
                        errorResponse.setBody("CGI execution failed");
                        errorResponse.setContentType("text/plain");
                        (*srv_it)->sendResponse((*cgi_it)->client_fd, errorResponse.toString());
                    }
                    close((*cgi_it)->output_pipe_fd.fd);
                    delete *cgi_it;
                    cgi_it = (*srv_it)->cgis.erase(cgi_it);
                }
                else if (result == 0)
                {
                    close((*cgi_it)->output_pipe_fd.fd);
                    close((*cgi_it)->client_fd);
                    delete *cgi_it;
                    cgi_it = (*srv_it)->cgis.erase(cgi_it);
                    LOG_INFO("No more CGI process");
                    ++cgi_it;
                }
                else
                {
                    LOG_INFO("CGI waitpid error");
                    ++cgi_it;
                }
            }
            else if (master_fds[cgi_index].revents & (POLLERR | POLLHUP | POLLNVAL))
            {    
                // Comprobar que no haya datos que leer aun
                (*srv_it)->handleCGIresponse(*cgi_it);
                
                LOG_INFO("CGI pipe closed");
                close((*cgi_it)->output_pipe_fd.fd);
                
                // COmprobar si el proceso sigue corriendo
                int status;
                pid_t result = waitpid((*cgi_it)->pid, &status, WNOHANG);
                if (result == 0)
                    kill((*cgi_it)->pid, SIGTERM);
                delete *cgi_it;
                cgi_it = (*srv_it)->cgis.erase(cgi_it);
            }
            else
            {
                ++cgi_it;
            }
            ++cgi_index;
        }
    }
}

void    Http::Clients_events(size_t& fd_index, std::vector<pollfd>& master_fds)
{
    for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
    {
        std::vector<ClientInfo*>::iterator cli_it = (*srv_it)->clients.begin();
        while (cli_it != (*srv_it)->clients.end())
        {
            if (fd_index >= master_fds.size())
            {
                //LOG_INFO("Client has not been polled: Index out of bounds");
                break;
            }

            if ((*srv_it)->IsTimeout(*cli_it))
            {
                // Check if client has active CGI processes
                bool hasCGI = false;
                for (std::vector<CgiProcess*>::iterator cgi_it = (*srv_it)->cgis.begin(); 
                        cgi_it != (*srv_it)->cgis.end(); ++cgi_it)
                {
                    if ((*cgi_it)->client_fd == (*cli_it)->pfd.fd) {
                        hasCGI = true;
                        break;
                    }
                }
                if (!hasCGI) {
                    close((*cli_it)->pfd.fd);
                    delete *cli_it;
                    cli_it = (*srv_it)->clients.erase(cli_it);
                    LOG_INFO("Client Timeouted");
                    continue;
                }
            }

            if (master_fds[fd_index].revents & POLLIN)
            {
                (*srv_it)->handleClient(*cli_it);
                ++cli_it;
            }
            else if (master_fds[fd_index].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                close((*cli_it)->pfd.fd);
                delete *cli_it;
                cli_it = (*srv_it)->clients.erase(cli_it);
                LOG_INFO("Connection Removed");
            }
            else
            {
                ++cli_it;
            }
            fd_index++;
        }
    }
}

void    Http::Server_events(size_t& fd_index, std::vector<pollfd>& master_fds)
{
    for (std::list<Server*>::iterator srv_it = servers.begin(); srv_it != servers.end(); srv_it++)
    {
        std::vector<int> server_fds = (*srv_it)->getPassiveSocketFd();
        for (std::vector<int>::iterator fd_it = server_fds.begin(); fd_it != server_fds.end(); fd_it++, fd_index++)
        {
            if (master_fds[fd_index].revents & POLLIN)
            {
                (*srv_it)->acceptClient(*fd_it);
            }
            else if (master_fds[fd_index].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                LOG_INFO("Passive socket error");
            }
        }
    }
}

void    Http::launch_all()
{
    std::list<Server*>::iterator it = servers.begin();
    if (it == servers.end())
        return ;
    try {
        for (; it != servers.end(); it++)
            (*it)->init();
    } catch (std::exception& e){
        delete (*it);
        servers.erase(it);
        return ;
    }
    LOG_INFO("All servers are launching... Press CTRL+C to quit");

    std::vector<pollfd> master_fds; // SOCKET_PASSIVO - FDCGI- FDcliente
    
    size_t num_servers = 0;
    size_t previous_nb_client_connections = 0;
    loadServers(master_fds, num_servers);

    while (42)
    {
        // repopular a partir de los socket pasivos
        //std::cout << "loop" << std::endl;
        master_fds.resize(num_servers);
        size_t total_clients = 0;
        loadNewConnections(total_clients, master_fds);
        CheckUserConnected(previous_nb_client_connections, master_fds);
        // Poll all FDs
        int poll_count = poll(&master_fds[0], master_fds.size(), 1000);
        if (poll_count < 0)
        {
            if (errno != EINTR)
                LOG("Poll error in master loop !!!");
            continue;
        }

        // Reset fd_index for handling events
        size_t fd_index = 0;
        size_t cgi_index = num_servers + total_clients;
        CGI_events(cgi_index, master_fds);
        fd_index += num_servers;
        Clients_events(fd_index, master_fds);
        fd_index = 0;
        Server_events(fd_index, master_fds);
    }
}




