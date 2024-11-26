#include "cgiHandler.hpp"

CgiHandler::CgiHandler()
{

}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::handle(const Request* request, Response* response, ClientInfo& clientinfo, LocationConfig& locationconfig) 
{
    std::string scriptPath = locationconfig.root; // Ruta del archivo a ejecutar 'greet.py'
    if (locationconfig.root.find(".py") == std::string::npos)
            scriptPath += locationconfig.index;

    // crear un entorno
    std::map<std::string, std::string> env;
    // Configurar las variables de entorno comunes
    env["REDIRECT_STATUS"] = "200"; //seguridad php
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath;
    env["REQUEST_METHOD"] = request->getMethod();
    env["CONTENT_TYPE"] = request->getHeader("Content-Type");
    env["PATH_INFO"] = locationconfig.cgi_pass;//request->getPath();
    std::ostringstream oss;
    oss << request->getBody().size();
    env["CONTENT_LENGTH"] = oss.str();
    // env["QUERY_STRING"] = request->getUri();
    // env["REMOTEaddr"] = to_string(locationconfig->HostHeader.host);
    //env["REMOTE_IDENT"] =
    //env["REMOTE_USRER"] =
    //env["REQUEST_URI"] = request->getPath() + request->getUri();
    //env["SERVER_NAME"]
    //env["SERVER_PORT"] =
    //env["SERVER_PROTOCOL"] = "HTTP/1.1";
    //env["SERVER_SOFTWARE"]

    // crear un nueva instancia de struct CgiProcess
    CgiProcess  cgi_process;
    cgi_process.owner_client_fd = clientinfo.pfd.fd;
    // Ejecutar el CGI
    executeCgi(cgi_process, env, request->getBody());


    response->setStatusCode(42);

    std::ostringstream oss;
    oss << cgi_process.owner_client_fd;
    response->setHeader("conn_fd", oss.str());
    oss.clear();
    oss << cgi_process.output_pipe_fd.fd;
    response->setHeader("piped_fd", oss.str());
    oss.clear();
    oss << cgi_process.pid;
    response->setHeader("pid", oss.str());
    oss.clear();
    oss << cgi_process.start_time;
    response->setHeader("start_time", oss.str());
}


void    CgiHandler::executeCgi(CgiProcess& cgi_process, 
                                 const std::map<std::string, std::string>& env, 
                                 const std::string& inputData) 
{
    int pipex[2];

    if (pipe(pipex) == -1) {
        LOG("Pipe Error");
        return ;
    }

    std::vector<char*> envp; // Declarar envp aquí para que esté accesible en ambos bloques

    // initializae el tiempo del proceso
    cgi_process.start_time = time(NULL);
    cgi_process.output_pipe_fd.fd  = pipex[0];
    cgi_process.output_pipe_fd.events = POLLIN;
    cgi_process.output_pipe_fd.revents = 0;
    cgi_process.pid = fork();

    if (cgi_process.pid < 0) {
        LOG("Fork failed");
        return ;
    }
    
    if (cgi_process.pid == 0) {  // Proceso hijo
        // Configurar la alarma para el timeout
        //alarm(CGI_TIMEOUT);

        dup2(pipex[1], STDOUT_FILENO);

        // Cerrar los pipes en el hijo
        close(pipex[0]);
        close(pipex[1]);

        // Preparar el entorno
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it)
        {
            std::string envVar = it->first + "=" + it->second;
            envp.push_back(strdup(envVar.c_str()));
        }
        envp.push_back(NULL); // Terminar el array con NULL

        // Argumentos para execve
        char* args[3];
        args[0] = const_cast<char*>(env.at("SCRIPT_FILENAME").c_str());
        args[1] = const_cast<char*>(inputData.c_str());
        args[2] = NULL;

        execve(env.at("PATH_INFO").c_str(), args, &envp[0]);

        // Si execve falla
        perror("execve falló");  // Imprimir el error específico
        exit(1);
    }
    
    // Proceso Padre
    close(pipex[1]);
    close(pipex[0]);

    // // Leer la salida del script
    // std::string result;
    // char buffer[4096];
    // ssize_t bytesRead;
    // time_t start_read = time(NULL);

    // while (true) {
    //     // Comprobar el timeout durante la
    //     if (time(NULL) - start_read >= CGI_TIMEOUT) {
    //         kill(current_process.pid, SIGKILL);
    //         LOG("CGI Timeout during read");
    //         return "Request Timeouted";
    //     }

    //     bytesRead = read(pipeOut[0], buffer, sizeof(buffer));
    //     if (bytesRead <= 0) break;
    //     result.append(buffer, bytesRead);
    // }

    // close(pipeOut[0]); // Cerrar después de leer

    // // Esperar el fin del proceso con el timeout
    // int status;
    // time_t wait_start = time(NULL);
    // while (true) {
    //     pid_t wpid = waitpid(current_process.pid, &status, WNOHANG);
    //     if (wpid == current_process.pid) break;
        
    //     if (time(NULL) - wait_start >= CGI_TIMEOUT) {
    //         kill(current_process.pid, SIGKILL);
    //         LOG("CGI Timeout waiting for process");
    //         return "Request Timeouted";
    //     }
    //     usleep(1000);  // Evitar sobrecarga de CPU
    // }

    // // Comprobar si el proceso se han terminado por una alarma
    // if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    //     return "Request timeouted";
    // }

    // // Eliminar encabezados
    // size_t headerEnd = result.find("\r\n\r\n");
    // if (headerEnd != std::string::npos) {
    //     result = result.substr(headerEnd + 4);
    // }

    // return result;
}

// Variable global para el proceso en curso
// static CgiProcess current_process;

// void handle_alarm(int sig) {
//     (void)sig;  
//     if (current_process.pid > 0) {
//         time_t current_time = time(NULL);
//         time_t elapsed = current_time - current_process.start_time;
        
//         if (elapsed >= CGI_TIMEOUT) {
//             std::ostringstream oss;
//             oss << elapsed;
//             LOG_INFO("CGI Timeout: Process running for " + oss.str());
//             kill(current_process.pid, SIGKILL);
//         }
//     }
// }
