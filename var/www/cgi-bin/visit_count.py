#!/usr/bin/env python3

import os

# Ruta al archivo que contendrá el conteo de visitas
counter_file = 'visit_count.txt'

# Verificar si el archivo existe
if os.path.exists(counter_file):
    # Leer el conteo actual
    with open(counter_file, 'r') as file:
        visit_count = int(file.read())
else:
    # Si no existe, iniciar el conteo en 0
    visit_count = 0

# Incrementar el conteo
visit_count += 1

# Escribir el nuevo conteo en el archivo
with open(counter_file, 'w') as file:
    file.write(str(visit_count))

# Encabezado HTTP
print("Content-Type: text/html")
print()  # Línea en blanco necesaria para separar encabezado y cuerpo

# Contenido HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Contador de Visitas</title></head>")
print("<body>")
print(f"<h1>Visitas: {visit_count}</h1>")
print("</body>")
print("</html>")
