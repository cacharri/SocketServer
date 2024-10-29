#!/usr/bin/php-cgi
<?php
// Ruta al archivo que contendrá el conteo de visitas
$counter_file = 'visit_count.txt';

// Verificar si el archivo existe
if (file_exists($counter_file)) {
    // Leer el conteo actual
    $visit_count = intval(file_get_contents($counter_file));
} else {
    // Si no existe, iniciar el conteo en 0
    $visit_count = 0;
}

// Incrementar el conteo
$visit_count++;

// Escribir el nuevo conteo en el archivo
file_put_contents($counter_file, $visit_count);

// Imprimir los encabezados HTTP
header("Content-Type: text/html");

// Contenido HTML
echo "<!DOCTYPE html>";
echo "<html>";
echo "<head><title>Contador de Visitas</title></head>";
echo "<body>";
echo "<h1>Visitas: $visit_count</h1>";
echo "</body>";
echo "</html>";
?>
