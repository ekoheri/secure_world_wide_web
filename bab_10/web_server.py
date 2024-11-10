import socket
import signal
import sys
from http import \
    parse_request_line, \
    handle_method

PORT = 8080
IP_ADDRESS = "127.0.0.1"
BUFFER_SIZE = 1024

sock_server = None

def start_server():
    global sock_server

    # 1. Inisialisasi socket server
    sock_server = socket.socket( \
        socket.AF_INET, \
        socket.SOCK_STREAM)
    sock_server.setsockopt( \
        socket.SOL_SOCKET, \
        socket.SO_REUSEADDR, 1)

    # 2. Bind IP Address dengan port
    sock_server.bind((IP_ADDRESS, PORT))

    # 3. Listen untuk koneksi masuk
    sock_server.listen(3)
    print("Server sedang berjalan.")
    print("Tekan Ctrl+C untuk menghentikan.")
    print(f"Akses URL  http://{IP_ADDRESS}:{PORT}\n")
#End start_server
    
def handle_client(sock_client):
    try:
        # Terima data request dari client
        request = sock_client.recv(BUFFER_SIZE).decode('utf-8')
        if not request:
            return

        # Parsing request
        req_info = parse_request_line(request)
        
        if not req_info.method:
            return  # Error handling jika request tidak valid

        # Proses request dan buat response
        response = handle_method(req_info)
        
        # Kirim response ke client
        if response:
            sock_client.sendall(response)
        else:
            print("Response data is NULL")

    except Exception as e:
        print(f"Terjadi error saat menangani client: {e}")
    finally:
        sock_client.close()
#End handle_client
        
def run_server():
    global sock_server
    while True:
        try:
            # Terima koneksi dari client
            sock_client, addr = sock_server.accept()
            print(f"Proses accept dari {addr} berhasil")

            # Tangani koneksi client
            handle_client(sock_client)

        except Exception as e:
            print(f"Terjadi error: {e}")
            continue
#End run_server
        
def stop_server(signum, frame):
    global sock_server
    if sock_server:
        sock_server.close()
    print("\nServer telah dihentikan.")
    sys.exit(0)

if __name__ == "__main__":
    # Tangani SIGINT (Ctrl+C) untuk menghentikan server
    signal.signal(signal.SIGINT, stop_server)

    # Jalankan server
    start_server()
    run_server()
#End main