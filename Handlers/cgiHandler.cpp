#include "cgiHandler.hpp"

CgiHandler::CgiHandler()
{
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig, int client_fd) 
{
    std::string scriptPath = locationconfig.root;
    if (locationconfig.root.find(".py") == std::string::npos)
        scriptPath += locationconfig.index;

    std::cout << "ScriptPath: " << scriptPath << std::endl;
    // Variables de entorno
    std::map<std::string, std::string> env;
    env["REDIRECT_STATUS"] = "200";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath;
    env["REQUEST_METHOD"] = request->getMethod();
    env["CONTENT_TYPE"] = request->getHeader("Content-Type");
    env["PATH_INFO"] = locationconfig.cgi_pass;
    
    std::ostringstream oss;
    oss << request->getBody().size();
    env["CONTENT_LENGTH"] = oss.str();

    // CReaci'on de la estructura CGIProcess 
    CgiProcess* cgi_process = new CgiProcess();
    cgi_process->client_fd = client_fd;
    executeCgi(cgi_process, env, request->getBody());
    // Almacenar una copia de cgi_process en el miembro de response
    response->setCgiProcess(cgi_process);
    response->setStatusCode(103);
    delete cgi_process;
}


void    CgiHandler::executeCgi(CgiProcess* cgi_process, 
                                 const std::map<std::string, std::string>& env, 
                                 const std::string& inputData) 
{
    int pipex[2];

    if (pipe(pipex) == -1) {
        LOG("Pipe Error");
        return;
    }

    int read_fd = pipex[0];
    int write_fd = pipex[1];

    std::cout << "Created pipe - Read fd: " << read_fd << ", Write fd: " << write_fd << std::endl;

    cgi_process->output_pipe_fd.fd = read_fd;
    cgi_process->output_pipe_fd.events = POLLIN;
    cgi_process->output_pipe_fd.revents = 0;

    // fd del pipe no bloqueante
    int flags = fcntl(read_fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);
    }

    std::cout << "Pipe READ fd stored as: " << cgi_process->output_pipe_fd.fd << std::endl;
    cgi_process->start_time = time(NULL);
    cgi_process->pid = fork();
    //std::cout << "New CGI process with PID: " << cgi_process->pid << std::endl;

    if (cgi_process->pid < 0) {
        LOG("Fork failed");
        close(read_fd);
        close(write_fd);
        return;
    }
    
    if (cgi_process->pid == 0) {  // Child process
        close(read_fd);  // Close read end in child
        
        if (dup2(write_fd, STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(write_fd);

        std::string scriptPath = env.at("SCRIPT_FILENAME");
        std::string workDir = scriptPath.substr(0, scriptPath.find_last_of("/"));
        
        // CD al directorio del archivo
        if (chdir(workDir.c_str()) != 0) {
            perror("chdir failed");
            exit(1);
        }

        // environment variables
        std::vector<char*> envp;
        envp.push_back(strdup("PYTHONUNBUFFERED=1"));  // Force Python to be unbuffered
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
            std::string envVar = it->first + "=" + it->second;
            envp.push_back(strdup(envVar.c_str()));
        }
        envp.push_back(NULL);

        std::string scriptName = scriptPath.substr(scriptPath.find_last_of("/") + 1);


        char* args[4];
        args[0] = const_cast<char*>(env.at("PATH_INFO").c_str());  // Python interpreter path
        args[1] = const_cast<char*>(scriptName.c_str());           // Script name only
        args[2] = const_cast<char*>(inputData.c_str());             // POST body input
        args[3] = NULL;
        //std::cout << "Executing " << args[0] << " " << args[1] << " " << args[2] << std::endl;
        execve(args[0], args, &envp[0]);

        perror("execve failed");
        for (std::vector<char*>::iterator it = envp.begin(); it != envp.end(); ++it) {
            if (*it) free(*it);
        }
        exit(1);
    }
    
    // Parent process
    close(write_fd);  // cerrar escritura
    std::cout << "Parent process using pipe fd: " << read_fd << std::endl;
}

