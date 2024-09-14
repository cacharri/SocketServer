## SERVER CLASS ##

This refactored version of the Server class has several advantages:
1. It uses poll() to handle multiple client connections efficiently, allowing for non-blocking I/O.
The launch() method now contains the main event loop, which continuously checks for incoming connections and client activity.
The acceptClient() method handles new client connections, setting them to non-blocking mode and adding them to the fds vector.
The handleClient() method processes incoming client requests and sends responses.
The removeClient() method properly closes and removes disconnected clients from the fds vector.
Error handling is improved with the use of exceptions and proper cleanup.
The dynamic buffer resizing approach is maintained for handling varying message sizes.
This implementation is cleaner and more organized while maintaining the advantages of your original poll-based approach. It allows for efficient handling of multiple client connections without blocking on any single client.
Some additional improvements you might consider:
1. Implement a maximum client limit to prevent resource exhaustion.
2. Add timeout handling for idle connections.
Implement proper HTTP request parsing and routing.
Add support for configuration options (e.g., max clients, timeout duration).
Remember to thoroughly test this implementation, especially under high concurrency scenarios, to ensure it meets your performance and stability requirements.


# SocketServer
C++ objects for socket programming enabling us to build custom Servers

- Mejorar el objecto Server
    - hacer reader un nuevo objeto lexer
    - hacer writer un nuevo objeto Writer implementando (RF, error responses, status line creation, header creation, body creation)
    - Gestionar bien los buffer de comunicacion
    - Mirar CGI
