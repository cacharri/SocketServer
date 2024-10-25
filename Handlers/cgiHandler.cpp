#include "Handlers.hpp"


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

void CgiHandler::handle(const Request& request, Response& response, const LocationConfig& locationconfig) {
   std::string scriptPath = locationconfig.root + "/" + locationconfig.index; // ruta del archivo
    // Crear un mapa para las variables de entorno
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = request.getMethod();
    env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    std::ostringstream oss;
    oss << request.getBody().size();
    env["CONTENT_LENGTH"] = oss.str();
    env["SCRIPT_FILENAME"] = scriptPath;

    // se pueden llenar mas variables de entorno

    std::string output = executeCgi(scriptPath, env, request.getBody());

    response.setBody(output);
    response.setStatus(200, "OK");
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

    if (pid < 0) {
        std::cerr << "Error al hacer fork" << std::endl;
        return "Error al ejecutar el CGI";
    }

    if (pid == 0) { // Proceso hijo
        // Redirigir la entrada y salida estándar
        dup2(pipeIn[0], STDIN_FILENO);   // Entrada del script
        dup2(pipeOut[1], STDOUT_FILENO); // Salida del script

        // Cerrar los pipes en el hijo
        close(pipeIn[1]);
        close(pipeOut[0]);

        // Establecer las variables de entorno
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
            setenv(it->first.c_str(), it->second.c_str(), 1);
        }
        setenv("REDIRECT_STATUS", "200", 1);
        // Ejecutar el script PHP
        execlp("php-cgi", "php-cgi", scriptPath.c_str(), (char*)NULL);
        // Si execlp falla
        std::cerr << "Error al ejecutar el script" << std::endl;
        exit(1);
    } else { // Proceso padre
        // Cerrar los extremos que no se utilizan
        close(pipeIn[0]);
        close(pipeOut[1]);

        // Escribir el cuerpo de la solicitud en el pipe
        write(pipeIn[1], inputData.c_str(), inputData.size());
        close(pipeIn[1]); // Cerrar después de escribir

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
        return result;
    }
}
