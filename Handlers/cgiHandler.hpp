#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "CgiProcess.hpp"
#include <fcntl.h>  
#include <unistd.h>
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "../Config/ConfigParser.hpp"


class CgiHandler
{
public:
    CgiHandler();
    ~CgiHandler();
    void handle(const Request* request, Response* response, LocationConfig& locationconfig, int client_fd);
private:
    void    executeCgi(CgiProcess* cgi_process, const std::map<std::string, std::string>& env, const std::string& inputData ); // execve( envp[PATH_INFO], args, envp)   args = [scriptPath, request->body ....]
};

#endif