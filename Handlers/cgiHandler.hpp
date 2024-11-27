#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../Config/ConfigParser.hpp"

#define CGI_TIMEOUT 5

// Structura para almacenar las informaciones del proceso CGI
struct CgiProcess
{
    //int     owner_client_fd;
    pollfd  output_pipe_fd;
    pid_t   pid;
    time_t  start_time;
    CgiProcess() : pid(-1), start_time(0)
    {
        output_pipe_fd.fd = -1;
        output_pipe_fd.events = 0;
        output_pipe_fd.revents = 0;
    }
};

class CgiHandler
{
public:
    CgiHandler();
    ~CgiHandler();
    void handle(const Request* request, Response* response, LocationConfig& locationconfig);
private:
    void    executeCgi(CgiProcess& cgi_process, const std::map<std::string, std::string>& env, const std::string& inputData ); // execve( envp[PATH_INFO], args, envp)   args = [scriptPath, request->body ....]
};

#endif