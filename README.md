# SocketServer

SocketServer es un servidor HTTP/1.1 desarrollado en C++98 que permite la creación de servidores personalizados mediante programación de sockets. Este proyecto está inspirado en la funcionalidad de Nginx y está diseñado para ser ligero y eficiente.

## Tabla de Contenidos

- [Características](#características)
- [Instalación](#instalación)
- [Configuración](#configuración)
- [Uso](#uso)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Módulos Principales](#módulos-principales)
- [Comparación con Nginx](#comparación-con-nginx)
- [Fuentes](#fuentes)

## Características

- **Compatibilidad con HTTP/1.1**: Soporte para métodos GET, POST y CGI.
- **Manejo de Rutas**: Enrutamiento de solicitudes a controladores específicos.
- **Soporte CGI**: Ejecución de scripts CGI para contenido dinámico.
- **Configuración Personalizable**: Archivo de configuración similar a Nginx para definir parámetros del servidor.
- **Gestión de Conexiones**: Manejo eficiente de múltiples conexiones con soporte para keep-alive y timeouts.
- **Gestión de Headers y Códigos de Error**: Manejo y validación de cabeceras HTTP y respuestas adecuadas ante errores.

## Instalación

Para compilar y ejecutar SocketServer, sigue estos pasos:

1. **Clona el repositorio**:

   ```bash
   git clone https://github.com/cacharri/SocketServer.git
   cd SocketServer
   ```

2. **Compila el proyecto** utilizando el Makefile proporcionado:

   ```bash
   make
   ```

   Esto generará un ejecutable llamado `Webserver`.

## Configuración

Antes de ejecutar el servidor, es necesario configurar los parámetros en el archivo `server.config`. Este archivo define la configuración del servidor, incluyendo puertos, rutas y opciones específicas.

Ejemplo de `server.config`:

```
server {
    listen 8080;
    server_name localhost;

    location / {
        root /var/www/html;
        index index.html;
    }

    location /cgi-bin/ {
        root /var/www/cgi-bin;
        cgi on;
    }
}
```

## Uso

Una vez configurado, puedes iniciar el servidor con el siguiente comando:

```bash
./Webserver server.config
```

El servidor estará activo y escuchando en el puerto especificado en la configuración.

## Estructura del Proyecto

- **Client/**: Manejo de conexiones de clientes.
- **Config/**: Análisis y gestión del archivo de configuración.
- **Handlers/**: Controladores para diferentes tipos de solicitudes HTTP.
- **Headers/**: Manejo de cabeceras HTTP.
- **Http/**: Definiciones y utilidades relacionadas con el protocolo HTTP.
- **Logger/**: Registro de eventos y errores del servidor.
- **Request/**: Análisis y representación de solicitudes HTTP entrantes.
- **Response/**: Construcción y envío de respuestas HTTP.
- **Router/**: Enrutamiento de solicitudes a los controladores correspondientes.
- **Server/**: Funcionalidad principal del servidor, incluyendo gestión de sockets.
- **Sockets/**: Abstracciones para programación de sockets en C++98.

## Módulos Principales

### Server

Gestiona la inicialización del servidor, acepta nuevas conexiones y maneja la comunicación con los clientes. Hereda de `MotherSocket` para manejar sockets IPv4 no bloqueantes.

### Router

Asocia rutas específicas con sus controladores correspondientes y procesa las solicitudes entrantes, determinando el controlador adecuado para cada una.

### Handlers

Incluye clases como `GetHandler`, `PostHandler` y `CgiHandler`, que manejan los métodos HTTP correspondientes y generan las respuestas apropiadas.

## Comparación con Nginx

Se ha tomado inspiración de Nginx para la estructura de configuración y gestión de rutas. Sin embargo, a diferencia de Nginx, este servidor está diseñado con un enfoque educativo y simplificado en C++98. Las principales diferencias incluyen:

- Implementación de un sistema de configuración simplificado.
- Gestión de conexiones no bloqueantes a través de `poll()`.
- Soporte básico para CGI sin necesidad de módulos adicionales.

## Fuentes

- HTTP/1.1 RFC 2616
- Documentación de Nginx
- Material de referencia sobre programación de sockets en C++98


