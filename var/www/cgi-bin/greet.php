
<?php
// Imprimir los encabezados HTTP
header("Content-Type: text/html");

// Verificar si el método es POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Obtener el nombre del formulario
    $name = htmlspecialchars($_POST['name']);
    
    // Contenido HTML de respuesta
    echo "<!DOCTYPE html>";
    echo "<html>";
    echo "<head><title>Saludo</title></head>";
    echo "<body>";
    echo "<h1>Hola, $name!</h1>";
    echo "</body>";
    echo "</html>";
} else {
    // Si no es un POST, mostrar el formulario
    echo "<!DOCTYPE html>";
    echo "<html>";
    echo "<head><title>Formulario de Saludo</title></head>";
    echo "<body>";
    echo "<form method='post' action='/cgi-bin'>";
    echo "<label for='name'>Ingresa tu nombre:</label>";
    echo "<input type='text' name='name' id='name' required>";
    echo "<input type='submit' value='Enviar'>";
    echo "</form>";
    echo "</body>";
    echo "</html>";
}
?>
