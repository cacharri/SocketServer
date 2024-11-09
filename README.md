# WEBSERV DOCUMENTATION

## Table of Contents
1. [Usage](#usage)
2. [Server Object](#server-object)
3. [Config Object](#config-object)
4. [Router Object](#router-object)
5. [Handlers](#handlers)
   - [RequestHandler Class](#requesthandler-class)
   - [GetHandler Class](#gethandler-class)
   - [PostHandler Class](#posthandler-class)
   - [CgiHandler Class](#cgihandler-class)
6. [Request Object](#request-object)
7. [Response Object](#response-object)
8. [To Do](#to-do)
9. [Sources](#sources)

## Usage
1. Set the Nginx configuration through the `server.config` file.
2. Compile the project using `Make` from the `Makefile`.
3. Execute the server with `./Webserver server.config`.

## Server Object
The main object for managing an HTTP 1.1 server. It inherits from the `MotherSocket` object, which instantiates non-blocking IPv4 sockets, both passive and active.

### Member Variables
- **Config**: Stores a copy of the server configuration (inspired by Nginx).
- **Router**: Maps endpoints to their associated data structures and functions.
- **Vector<ClientsInfo>**: Data structure for session management (timeout, keepAlive) and `pollfd` struct for handling incoming socket (client FD).
- **Buffer**: For receiving messages from the socket.
- **Default Configuration Variables**: Timeout and buffer size.

### Functions
- **Init()**: Adds the O_NONBLOCK flag to the server socket and creates the first passive client in the managed clients vector. Puts the passive socket in listening mode for up to 10 incoming connections.
- **AcceptClient()**: Accepts new connections on the passive socket and registers each connection with a `ClientInfo` structure.
- **RemoveClient()**: Closes the connection FD and removes the `ClientInfo` structure from the clients vector.
- **ReceiveMessage()**: Returns a string with bytes from the associated connection FD up to `CLIENT_MAX_BODY_SIZE`.
- **SendResponse()**: Writes and sends a response on the active connection socket.
- **AnalyzeBasicHeaders()**: Logic for general headers: HOST, CONNECTION, KEEP-ALIVE.
- **IsTimeout()**: Checks if the client has been inactive for a sufficient amount of time.
- **Launch()**: Starts the main server loop, waiting for events on client sockets, accepting new connections, and managing client activity and timeouts.
- **HandleClient()**: Manages communication with a specific client, receiving messages, analyzing headers, routing requests, and sending responses.

## Config Object
- **parseConfigFile(const std::string& filename)**: Parses the `server.config` file and stores parameters in `LocationConfig` and `ServerConfig` structures.

## Router Object
Responsible for managing routes and handling HTTP requests. It associates routes with their respective handlers and processes incoming requests.

### Member Variables
- **routes**: A map that associates routes with a vector of route configurations (`RouteConfig`).

### Functions
- **Router()**: Constructor that initializes the Router object.
- **~Router()**: Destructor that frees memory for handlers and route configurations.
- **loadEndpoints(const std::string& endpoint, const LocationConfig& locConfig)**: Loads allowed endpoints (GET, POST, CGI) and associates them with their handlers.
- **addRoute(const std::string& path, const LocationConfig& locationconfig, RequestHandler *requesthandler, std::string HandledMethod)**: Adds a new route to the routes map with its configuration and corresponding handler.
- **route(const Request& request, Response& response)**: Processes an incoming request, verifies the route and method, and calls the corresponding handler.
- **isCgiRequest(const std::string& path)**: Checks if the requested path is a CGI request.
- **HasValidMethod(std::vector<RouteConfig *>& ConfigsAllowed, const std::string& input_method)**: Checks if the request method is valid for the given route.

## Handlers
Handlers are responsible for processing specific HTTP requests (GET, POST, CGI) and generating appropriate responses.

### RequestHandler Class
- **handle(const Request& request, Response& response, const LocationConfig& locationconfig)**: Pure virtual method to be implemented by derived classes for handling requests.
- **~RequestHandler()**: Virtual destructor.

### GetHandler Class (inherits from RequestHandler)
- **GetHandler()**: Constructor that initializes the GET handler.
- **~GetHandler()**: Destructor that cleans up resources.
- **handle(const Request& request, Response& response, const LocationConfig& locationconfig)**: Handles GET requests, reads files, and generates responses.

### PostHandler Class (inherits from RequestHandler)
- **PostHandler()**: Constructor that initializes the POST handler.
- **~PostHandler()**: Destructor that cleans up resources.
- **handle(const Request& request, Response& response, const LocationConfig& locationconfig)**: Handles POST requests, processes form data, and generates responses.
- **saveFile(const std::string& filename, const std::string& data)**: Saves a file to the system.
- **parseMultipartFormData(const std::string& data, const std::string& boundary, const std::string& post_upload_store)**: Parses multipart form data.
- **parseUrlFormData(const std::string& body)**: Parses URL-encoded form data.
- **urlDecode(const std::string &str)**: Decodes a URL-encoded string.
- **escapeHtml(const std::string& data)**: Escapes special characters in HTML.

### CgiHandler Class (inherits from RequestHandler)
- **CgiHandler()**: Constructor that initializes the CGI handler.
- **~CgiHandler()**: Destructor that cleans up resources.
- **handle(const Request& request, Response& response, const LocationConfig& locationconfig)**: Handles CGI requests and executes scripts.
- **executeCgi(const std::string& scriptPath, const std::map<std::string, std::string>& env, const std::string& inputData)**: Executes a CGI script and returns its output.

## Request Object
Represents an HTTP request. It analyzes and stores request information, including method, URI, headers, and body.

### Member Variables
- **method**: Stores the HTTP method (GET, POST, etc.).
- **uri**: Stores the requested URI.
- **httpVersion**: Stores the HTTP protocol version.
- **headers**: Map that stores request headers.
- **body**: Stores the request body.

### Functions
- **Request(const std::string& rawRequest)**: Constructor that initializes the object from a raw request string.
- **parse(const std::string& rawRequest)**: Parses the raw request and extracts method, URI, headers, and body.
- **getMethod() const**: Returns the HTTP method of the request.
- **getUri() const**: Returns the URI of the request.
- **getHeader(const std::string& key) const**: Returns the value of a specific header.
- **getBody() const**: Returns the body of the request.
- **getHttpVersion() const**: Returns the HTTP protocol version.
- **setBody(const std::string& requestBody)**: Sets the body of the request.
- **print() const**: Prints request information to the console.
- **getPath() const**: Returns the path of the URI, excluding any query string.

## Response Object
Represents an HTTP response. It constructs and stores response information, including status code, headers, and body.

### Member Variables
- **statusCode**: Stores the HTTP status code.
- **statusMessage**: Stores the corresponding status message.
- **headers**: Map that stores response headers.
- **body**: Stores the response body.

### Functions
- **Response(int code)**: Constructor that initializes the object with a status code and sets the default status message.
- **setStatus(int code, const std::string& message)**: Sets the status code and message.
- **setHeader(const std::string& key, const std::string& value)**: Sets a header in the response.
- **setBody(const std::string& content)**: Sets the response body and updates the content length.
- **setContentType(const std::string& type)**: Sets the content type in the header.
- **setContentLength()**: Sets the content length in the header.
- **toString() const**: Converts the response to a string in HTTP format.
- **getStatusCode() const**: Returns the status code of the response.
- **getStatusMessage() const**: Returns the status message of the response.
- **getBody() const**: Returns the body of the response.

## To Do
- Make post work with json
- Implement PATH_INFO macro for cgi compilator binary Primer argumento de execve
- El CGI tiene que ejecutarse en el buen directorio para el acceso al fichero del camino relativo. Ein ? a revisar(frase del subject traducido del frances)
- Implement DELETE
- Control memory leaks at error points
- Control error codes (hagamos una lista de los qe controlamos)
- Control de la entrega del body cuando hay un errorCode
- Mejorar ConfigParser en el manejo de error_pages para manejar multiples paginas y error status
- Control Headers:
   - Review chunked mode
   - listar los headers de Request que tenemos en cuenta.
   - Escribir los headers de la respuesta.
- Compare with nginx

## Sources
- [HTTP Content Negotiation](https://http.dev/content-negotiation)
