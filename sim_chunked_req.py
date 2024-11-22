import socket
import time

def send_chunked_request():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('127.0.0.1', 8080))

    # Test avec un contenu plus substantiel
    content = "Este es un mensaje largo que será enviado en chunks para probar el encoding chunked"
    chunk_size = 10  # Taille de chaque chunk

    # En-têtes
    headers = (
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
    )

    print("total lenght of message: " + str(len(content)))
    print("Envoi des en-têtes...")
    s.sendall(headers.encode('utf-8'))

    # Diviser le contenu en chunks
    start = 0
    while start < len(content):
        # Extraire le chunk
        print(start)
        chunk = content[start:start + chunk_size]
        
        # Format du chunk: taille en hex + CRLF + données + CRLF
        chunk_header = "{:x}\r\n".format(len(chunk))
        chunk_data = chunk + "\r\n"
        
        print(f"Envoi chunk: Taille={len(chunk)} ({chunk_header}), Data={chunk}")
        
        # Envoyer le chunk complet
        s.sendall(chunk_header.encode('utf-8'))
        s.sendall(chunk_data.encode('utf-8'))
        
        start += chunk_size
        time.sleep(0.1)  # Petit délai pour debug

    # Envoyer le chunk final
    print("Envoi du chunk final")
    s.sendall(b"0\r\n\r\n")

    # Recevoir et afficher la réponse
    print("\nAttente de la réponse...")
    response = b""
    s.settimeout(2)  # Timeout de 2 secondes
    
    try:
        while True:
            data = s.recv(4096)
            if not data:
                break
            response += data
    except socket.timeout:
        print("Timeout atteint")
    
    print("\nRéponse du serveur:")
    print(response.decode('utf-8', errors='ignore'))
    
    s.close()

if __name__ == "__main__":
    send_chunked_request()