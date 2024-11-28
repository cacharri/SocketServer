#ifndef CGI_PROCESS_HPP
#define CGI_PROCESS_HPP

#include <sys/time.h>
#include <time.h>
#include <poll.h>

// Structure for CGI process information
struct CgiProcess
{
    int     client_fd;
    pollfd  output_pipe_fd;
    pid_t   pid;
    time_t  start_time;
    
    CgiProcess() : client_fd(-1), pid(-1), start_time(0)
    {
        output_pipe_fd.fd = -1;
        output_pipe_fd.events = 0;
        output_pipe_fd.revents = 0;
    }
};

#endif 