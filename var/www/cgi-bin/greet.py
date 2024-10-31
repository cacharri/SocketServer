#!/usr/bin/env python3

import cgi
import html


# Crear un objeto FieldStorage para manejar la entrada del formulario
form = cgi.FieldStorage()

# Verificar si el método es POST
if "name" in form:
    # Obtener el nombre del formulario
    name = html.escape(form.getvalue("name"))
    
    # Contenido HTML de respuesta
    print(f"""
    <!DOCTYPE html>
    <html>
    <head><title>Saludo</title></head>
    <body>
    <h1>Hola, {name}!</h1>
    </body>
    </html>
    """)
else:
    # Si no es un POST, mostrar el formulario
    print(f"""
    <!DOCTYPE html>
    <html>
    <head><title>Formulario de Saludo</title></head>
    <body>
    <form method='post' action='/cgi-bin2'>
        <label for='name'>Ingresa tu nombre:</label>
        <input type='text' name='name' id='name' required>
        <input type='submit' value='Enviar'>
    </form>
    </body>
    </html>
    """)
