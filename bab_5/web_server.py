import signal
import socket
import sys

PORT = 8080
ip_address = "127.0.0.1"
BUFFER_SIZE = 1024

# Deklarasi variable sock_server
sock_server = None

def start_server():
    global sock_server
    sock_server = socket.socket(\
        socket.AF_INET, \
        socket.SOCK_STREAM)
    sock_server.setsockopt(\
        socket.SOL_SOCKET, \
        socket.SO_REUSEADDR, 1)

    # Mengisi alamat server
    sock_server.bind((ip_address, PORT))

    # Mendengarkan koneksi masuk
    sock_server.listen(3)
    print("Server sedang berjalan.", end="")
    print("Tekan Ctrl+C untuk menghentikan.")
    print(f"Akses URL http://{ip_address}:{PORT}")
#end def start_server

def handle_client(sock_client):
    try:
        # Membaca permintaan dari browser
        request = sock_client.recv(BUFFER_SIZE)
        if not request:
            print("Proses baca dari client gagal")
            sock_client.close()
            return
        # End if
        # Menampilkan permintaan dari browser
        print("Request dari browser : ")
        print(f"{request.decode()}")
        # Membuat respons HTTP sederhana
        response_header = (
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
        )
        response_body = (
            "<!DOCTYPE html>"
            "<html>"
            "<head><title>Hello World</title></head>"
            "<body>"
            "<h1>Hello, World!</h1>"
            "<p>Ini python</p>"
            "</body>"
            "</html>"
        )
        # Mengirim respons ke klien
        sock_client.sendall(
            (response_header+response_body).encode()
        )
    except Exception as e:
        print(f"Proses kirim data ke client gagal: {e}")
    finally:
        # Menutup koneksi
        sock_client.close()
#end def handle_client
        
def run_server():
    global sock_server
    if sock_server:
        while True:
            sock_client, addr = sock_server.accept()
            print(f"Proses accept dari {addr} berhasil")
            # Panggil fungsi handle_client
            handle_client(sock_client)
        #End while
    #End if
#End run_server

def stop_server(signal_received, frame):
    global sock_server
    if sock_server:
        sock_server.close()  # Tutup socket server
    print("\nServer telah dihentikan.")
    sys.exit(0)  # Keluar dari program
#end def stop_server

if __name__ == "__main__":
    # Mengatur signal handler untuk menangani Ctrl+C
    signal.signal(signal.SIGINT, stop_server)
    # Menjalankan server
    start_server()
    run_server()
#end def main
