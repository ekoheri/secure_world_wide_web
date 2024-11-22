import socket
import sys

PORT = 8080
BUFFER_SIZE = 1024

def main(server_ip):
    # 1. Inisialisasi Socket
    try:
        # Cek IP Address server apakah aktif atau tidak.
        server_addr = socket.gethostbyname(server_ip)
    except socket.gaierror:
        print("IP address server kosong atau tidak valid")
        sys.exit(1)


    sock_client = socket.socket(\
        socket.AF_INET, \
        socket.SOCK_STREAM)
        
    # 2. Connect
    try:
        serv_addr = (server_addr, PORT)
        sock_client.connect(serv_addr)
    except socket.error as e:
        print(f"Koneksi ke server gagal: {e}")
        sys.exit(1)

    while True:
        # 3. Send Request
        buffer = input("Client: ")
        
        # Kirim pesan ke server
        sock_client.sendall(buffer.encode('utf-8'))
        
        # 4. Read Response
        if buffer == "exit":
            print("Mengakhiri chat...")
            break
        
        # Bersihkan buffer sebelum membaca
        response = sock_client.recv(BUFFER_SIZE).decode('utf-8')
        if response:
            print(f"Server: {response}")
    #End While

    #5. menutup koneksi socket client
    sock_client.close()
#End def main

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("IP address server tidak diisi!")
        sys.exit(1)
    main(sys.argv[1])
