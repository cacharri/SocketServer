#include "Handlers.hpp"


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

void CgiHandler::handle(const Request* request, Response* response, const LocationConfig& locationconfig) {
    std::string scriptPath = locationconfig.root + "/" + locationconfig.index; // Ruta del archivo
    std::map<std::string, std::string> env;

    // Configurar las variables de entorno comunes
    env["REDIRECT_STATUS"] = "200"; //seguridad php
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath;
    env["REQUEST_METHOD"] = request->getMethod();
    env["CONTENT_TYPE"] = request->getHeader("Content-Type");
    env["PATH_INFO"] = "/urs/bin/php-cgi";//request->getPath();
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
    response->setStatus(200, "OK");
   
}



std::string CgiHandler::executeCgi(const std::string& scriptPath, const std::map<std::string, std::string>& env, const std::string& inputData) {
    int pipeIn[2];  // Pipe para entrada
    int pipeOut[2]; // Pipe para salida

    // Crear los pipes
    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        std::cerr << "Error al crear los pipes" << std::endl;
        return "Error al ejecutar el CGI";
    }

    pid_t pid = fork(); // Crear un nuevo proceso
    std::vector<char*> envp; // Declararo envp aquí para que esté accesible en ambos bloques

    if (pid < 0) {
        std::cerr << "Error al hacer fork" << std::endl;
        return "Error al ejecutar el CGI";
    }

    if (pid == 0) { // Proceso hijo
    // Redirigir la entrada y salida estándar
    dup2(pipeIn[0], STDIN_FILENO);
    dup2(pipeOut[1], STDOUT_FILENO);

    // Cerrar los pipes en el hijo
    close(pipeIn[1]);
    close(pipeOut[0]);

    // Preparar el entorno
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
        std::string envVar = it->first + "=" + it->second;
     //   std::cout << it->second << std::endl;
        envp.push_back(strdup(envVar.c_str()));
    }
    
    envp.push_back(NULL); // Terminar el array con NULL
    

    // Argumentos para execve
    char* args[3];// = { const_cast<char*>(scriptPath.c_str()), NULL };

    std::cerr << "Ejecutando: " << scriptPath << std::endl;
    for (size_t i = 0; i < envp.size(); ++i) {
        std::cerr << "Variable de entorno: " << envp[i] << std::endl;
    }
    // Ejecutar el script CGI
    //execve(scriptPath.c_str(), args, &envp[0]);
   
  if ((scriptPath.find(".py")!= std::string::npos)) {
    args[0] = const_cast<char*>("/usr/bin/python3"); // Cambiar a Python
    args[1] = const_cast<char*>(scriptPath.c_str());
    args[2] = NULL;
} else if (scriptPath.find(".php") != std::string::npos){
    args[0] = const_cast<char*>("/usr/bin/php-cgi");
    args[1] = const_cast<char*>(scriptPath.c_str());
    args[2] = NULL;
}


    execve(args[0], args, &envp[0]);
   // std::cout << "AQUIIIIIIIII   " <<std::endl;
    // Si execve falla
    perror("execve falló");  // Imprimir el error específico
    exit(1);
} else { // Proceso padre
        // Cerrar los extremos que no se utilizan
        close(pipeIn[0]);
        close(pipeOut[1]);
         

        // Escribir el cuerpo de la solicitud en el pipe
        write(pipeIn[1], inputData.c_str(), inputData.size());
        close(pipeIn[1]); // Cerrar después de escribir
std::cout << "path::::  " << scriptPath << std::endl;
        // Leer la salida del script
        std::string result;
        char buffer[128];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0) {
            
            result.append(buffer, bytesRead);
        }

        close(pipeOut[0]); // Cerrar después de leer

        // Esperar al proceso hijo
        waitpid(pid, NULL, 0);

        size_t pos = result.find("\r\n\r\n");
        if (pos != std::string::npos) {
            // Eliminar los encabezados
            result = result.substr(pos + 4);  // Mover el puntero al inicio del cuerpo
        }

        // Liberar la memoria de las variables de entorno
        for (size_t i = 0; i < envp.size(); ++i) {
            free(envp[i]);
        }

        return result;
    }
}
