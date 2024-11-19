#include "Handlers.hpp"
#include <sys/time.h>
#include <time.h>


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}


void CgiHandler::handle(const Request* request, Response* response, LocationConfig& locationconfig) 
{
    std::string scriptPath = locationconfig.root; // Ruta del archivo
    if (locationconfig.root.find(".py") == std::string::npos)
            scriptPath += locationconfig.index;
    std::map<std::string, std::string> env;

    // Configurar las variables de entorno comunes
    env["REDIRECT_STATUS"] = "200"; //seguridad php
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath;
    env["REQUEST_METHOD"] = request->getMethod();
    env["CONTENT_TYPE"] = request->getHeader("Content-Type");
    env["PATH_INFO"] = locationconfig.cgi_pass;//request->getPath();
   // env["QUERY_STRING"] = request->getUri();
   // env["REMOTEaddr"] = to_string(locationconfig->HostHeader.host);
    //env["REMOTE_IDENT"] =
    //env["REMOTE_USRER"] =
    //env["REQUEST_URI"] = request->getPath() + request->getUri();
    //env["SERVER_NAME"]
    //env["SERVER_PORT"] =
    //env["SERVER_PROTOCOL"] = "HTTP/1.1";
    //env["SERVER_SOFTWARE"]
   

    std::ostringstream oss;
    oss << request->getBody().size();
    env["CONTENT_LENGTH"] = oss.str();

    // Ejecutar el CGI sin cuerpo
    std::string output = executeCgi(scriptPath, env, request->getBody());

    response->setHeader("Content-Type", "text/html");
    response->setBody(output);

    // Establecer el estado de la respuesta
    response->setStatusCode(200);
    
   
}


// Constantes
const time_t CGI_TIMEOUT = 5;  // 5 secondes timeout

// Structura para almacenar las informaciones del proceso CGI
struct CgiProcess {
    pid_t pid;
    time_t start_time;
    
    CgiProcess() : pid(-1), start_time(0) {}
};

// Variable global para el proceso en curso
static CgiProcess current_process;

void handle_alarm(int sig) {
    (void)sig;  
    if (current_process.pid > 0) {
        time_t current_time = time(NULL);
        time_t elapsed = current_time - current_process.start_time;
        
        if (elapsed >= CGI_TIMEOUT) {
            LOG_INFO("CGI Timeout: Process running for " + elapsed);
            kill(current_process.pid, SIGKILL);
        }
    }
}

std::string CgiHandler::executeCgi(const std::string& scriptPath, 
                                 const std::map<std::string, std::string>& env, 
                                 const std::string& inputData) 
{
    int pipeIn[2];
    int pipeOut[2];

    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        LOG("Pipe Error");
        return "Internal Error";
    }


    std::vector<char*> envp; // Declarar envp aquí para que esté accesible en ambos bloques
    // Configurar la gestion de la senal antes del fork
    struct sigaction sa;
    sa.sa_handler = handle_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    // initializae el tiempo del proceso
    current_process.start_time = time(NULL);
    current_process.pid = fork();

    if (current_process.pid < 0) {
        LOG("Fork failed");
        return "Internal Error";
    }

    if (current_process.pid == 0) {  // Proceso hijo
        // Configurar la alarma para el timeout
        alarm(CGI_TIMEOUT);

        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);

        // Cerrar los pipes en el hijo
        close(pipeIn[1]);
        close(pipeOut[0]);

        // Preparar el entorno
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
            std::string envVar = it->first + "=" + it->second;
            envp.push_back(strdup(envVar.c_str()));
        }
        envp.push_back(NULL); // Terminar el array con NULL
        
        // Argumentos para execve

        char* args[3];
        if (scriptPath.find(".py") != std::string::npos) {
            args[0] = const_cast<char*>("/usr/bin/python3");
            args[1] = const_cast<char*>(scriptPath.c_str());
            args[2] = NULL;
        } else if (scriptPath.find(".php") != std::string::npos) {
            args[0] = const_cast<char*>("/usr/bin/php-cgi");
            args[1] = const_cast<char*>(scriptPath.c_str());
            args[2] = NULL;
        }

        execve(args[0], args, &envp[0]);

        // Si execve falla
        perror("execve falló");  // Imprimir el error específico
        exit(1);
    }
    
    // Proceso Padre
    close(pipeIn[0]);
    close(pipeOut[1]);

    //  Escribir el cuerpo de la solicitud en el pipe

    write(pipeIn[1], inputData.c_str(), inputData.size());
    close(pipeIn[1]);

    // Leer la salida del script
    std::string result;
    char buffer[4096];
    ssize_t bytesRead;
    time_t start_read = time(NULL);

    while (true) {
        // Comprobar el timeout durante la
        if (time(NULL) - start_read >= CGI_TIMEOUT) {
            kill(current_process.pid, SIGKILL);
            LOG("CGI Timeout during read");
            return "Request Timeouted";
        }

        bytesRead = read(pipeOut[0], buffer, sizeof(buffer));
        if (bytesRead <= 0) break;
        result.append(buffer, bytesRead);
    }

    close(pipeOut[0]); // Cerrar después de leer

    // Esperar el fin del proceso con el timeout
    int status;
    time_t wait_start = time(NULL);
    while (true) {
        pid_t wpid = waitpid(current_process.pid, &status, WNOHANG);
        if (wpid == current_process.pid) break;
        
        if (time(NULL) - wait_start >= CGI_TIMEOUT) {
            kill(current_process.pid, SIGKILL);
            LOG("CGI Timeout waiting for process");
            return "Request Timeouted";
        }
        usleep(1000);  // Evitar sobrecarga de CPU
    }

    // COmprobar si el proceso se han terminado por una alarma
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return "Request timeouted";
    }

    // Eliminar encabezados
    size_t headerEnd = result.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        result = result.substr(headerEnd + 4);
    }

    return result;
}

