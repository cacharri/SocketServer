WEBSERV DOCUMENTATION

## OBJETO SERVER ##

- Mejoras hechas
1. Clase principal para la gestión de fds pasivos y activos asicomo la gestion de conexiones entrantes.
    -   Objeto derivado de MotherSocket, clase que se usa para configurar los socket manejados por Server.
    1.1. MotherSocket tiene un ternario ahora para comprobar que haya una interfaz con la que interactuar.
2. Ahora se utiliza un buffer dinámico que se expande en función de los bytes entrantes por el fd.
3. Se utiliza un vector de structuras pollfd junto con poll() para manejar la disponibilidad de los fds.
4. Renforzado el error management con Excepciones propias a cada objeto y añadido un objeto Logger.

Funcionamiento:

 launch() -> bucle principal que recoje requests nuevas y comprueba el estado de las sessiones 
 acceptClient() -> Maneja conexiones a traves de un vector y define esta como no bloqueante.
 handleclient() -> Recoje e procesa el requests entrante y responde.
 receiveMessage() -> lee del socket de la conexión por chunks de 4096 bytes y los va guardando 
 sendResponse() -> Envia una página.
 removeClient() -> Elimina y cierra sockets del vector 

Por hacer ?
    - Procesar el request en handleclient() tras recibirlo con receiveMessage() y crear un Data structure
    - Leer el Data Structure y responder con la pagina, la pagina de error, o una ejecución cgi.
        - Como se gestiona POST ?
    - Poner un limite de sessiones para prevenir vectores<Server> enormes.
    - Poner un timeout en las sessiones para no agotar recursos.

## OBJETO CONFIG ##

Funcionamiento:
    parseconfigFIle(const std::string& filename) -> Guarda en 2 estructuras de Datos LocationConfig y ServerConfig los párametros de configuration del programa obtenidos a través del archivo "server.config"


