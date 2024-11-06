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
    for (std::list<Server*>::iterator iter = servers.begin(); iter != servers.end(); iter++)
    {
        (*iter)->init();
        (*iter)->launch();
    }
}

void Http::setupSignalHandlers(Http* http)
{
    g_http_instance = http;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}



