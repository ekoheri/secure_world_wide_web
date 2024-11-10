import socket

PORT = 8080
ip_address = "127.0.0.1"
BUFFER_SIZE = 1024

def main():
    #1. Inisialisasi socket
    sock_server = socket.socket( \
        socket.AF_INET, \
        socket.SOCK_STREAM)

    #2. Setting socket option
    sock_server.setsockopt( \
        socket.SOL_SOCKET, \
        socket.SO_REUSEADDR, 1)
    
    #3. Bind IP Address dan Port
    serv_addr = (ip_address, PORT)
    sock_server.bind(serv_addr)
    
    #4. Listen
    sock_server.listen(3)
    print(f"Server siap IP: {ip_address} Port: {PORT}")
    
    #5. Accept
    sock_client, addr = sock_server.accept()
    print(f"Terhubung dengan {addr}")
    
    while True:
        # 6. Read Request
        buffer = sock_client.recv(BUFFER_SIZE).decode('utf-8')
        if buffer:
            print(f"Client: {buffer}")
        print("Server: ", end='')

        # 7. Send Response
        buffer = input()
        if buffer == "exit":
            print("Mengakhiri chat...")
            break
        
        sock_client.sendall(buffer.encode('utf-8'))
    #End While
        
    #8. Close Socket Client
    sock_client.close()
    #9. Close Socket Server
    sock_server.close()
#End def main
    
if __name__ == "__main__":
    main()
