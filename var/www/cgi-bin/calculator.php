#!/usr/bin/env php-cgi
<?php
// Imprimir los encabezados HTTP
header("Content-Type: text/html");

// Función para realizar la operación
function calculate($operation, $num1, $num2) {

    switch ($operation) {
        case $operation ='sum':
            return $num1 + $num2;
        case $operation = 'subtract':
            return $num1 - $num2;
        default:
            return NULL;
    }
}

// Verificar si el método es POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Obtener los valores del formulario
    $num1 = isset($_POST['num1']) ? (float)$_POST['num1'] : 0;
    $num2 = isset($_POST['num2']) ? (float)$_POST['num2'] : 0;
    $operation = isset($_POST['operation']) ? $_POST['operation'] : '';

    $result = calculate($operation, $num1, $num2);
    
    // Contenido HTML de respuesta
    echo "<!DOCTYPE html>";
    echo "<html>";
    echo "<head><title>Resultado</title></head>";
    echo "<body>";
    if ($result !== null) {
        echo "<h1>Resultado: $result</h1>";
    } else {
        echo "<h1>Operación no válida.</h1>";
    }
    echo "<a href='/cgi-bin'>Volver</a>";
    echo "</body>";
    echo "</html>";
} else {
    // Si no es un POST, mostrar el formulario
    echo "<!DOCTYPE html>";
    echo "<html>";
    echo "<head><title>Calculadora</title></head>";
    echo "<body>";
    echo "<form method='post' action='/cgi-bin'>";
    echo "<label for='num1'>Numero 1:</label>";
    echo "<input type='number' name='num1' id='num1' required>";
    echo "<br>";
    echo "<label for='num2'>Numero 2:</label>";
    echo "<input type='number' name='num2' id='num2' required>";
    echo "<br>";
    echo "<label for='operation'>Operacion:</label>";
    echo "<select name='operation' id='operation' required>";
    echo "<option value='sum'>Suma</option>";
    echo "<option value='subtract'>Resta</option>";
    echo "</select>";
    echo "<br>";
    echo "<input type='submit' value='Calcular'>";
    echo "</form>";
    echo "</body>";
    echo "</html>";
}
?>
