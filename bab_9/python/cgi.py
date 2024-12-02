import socket
import signal
import sys
import os
import time

from config import cfg, load_config
from log import create_log_directory, write_log
from http import handle_method, parse_request_line, RequestHeader

sock_server = None
addrlen = 0

def start_server():
    global sock_server
    opt = 1

    # Inisialisasi socket server
    sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    if sock_server == 0:
        #print("Inisialisasi socket server gagal", file=sys.stderr)
        write_log("CGI Inisialisasi socket server gagal")
        sys.exit(1)

    # Mengatur opsi socket
    sock_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR | socket.SO_REUSEPORT, opt)

    # Bind IP Address dengan port
    server_address = (cfg.server_name, cfg.server_port)
    try:
        sock_server.bind(server_address)
    except Exception as e:
        # print(f"Proses bind gagal: {e}", file=sys.stderr)
        write_log("CGI Proses bind gagal")
        sock_server.close()
        sys.exit(1)

    # Listen untuk mendengarkan koneksi masuk
    sock_server.listen(3)
    # print("Server CGI sedang berjalan.")
    # print("Tekan Ctrl+C untuk menghentikan.")
    # print(f"Akses URL http://{cfg.server_name}:{cfg.server_port}")
    write_log("CGI Server berjalan")


def handle_client(sock_client):
    try:
        request_buffer_size = 4096
        request = sock_client.recv(request_buffer_size)
        if not request:
            return
        
        request_str = request.decode("utf-8")
        req_header = parse_request_line(request_str)

        # Handle method and get response
        response = handle_method(req_header)
        if response:
            sock_client.sendall(response)
    except Exception as e:
        # print(f"Proses menangani request client gagal: {e}", file=sys.stderr)
        write_log("Proses menangani request client gagal %s", e)
    finally:
        sock_client.close()


def run_server():
    if sock_server:
        while True:
            try:
                sock_client, client_addr = sock_server.accept()
                client_ip = client_addr[0]
                #print(f"Proses accept dari {client_ip} berhasil")
                write_log("Proses accept dari %s berhasil", client_ip)

                # Panggil fungsi handle_client
                handle_client(sock_client)
            except Exception as e:
                # print(f"Proses accept gagal: {e}", file=sys.stderr)
                write_log("Proses accept gagal %s", e)
                continue


def stop_server(signal, frame):
    if sock_server:
        sock_server.close()
    # print("\nServer telah dihentikan.")
    write_log("CGI Server telah dihentikan.")
    sys.exit(0)

def set_daemon():
    try:
        # Fork pertama: Membuat proses baru dan menghapus kait terminal
        pid = os.fork()
        if pid > 0:
            sys.exit(0)  # Proses induk keluar, hanya proses anak yang tetap berjalan
        
        # Set session ID untuk memisahkan daemon dari terminal
        os.setsid()
        
        # Fork kedua: Menghindari proses daemon yang menjadi 'orphan' setelah parent process keluar
        pid = os.fork()
        if pid > 0:
            sys.exit(0)  # Proses anak pertama keluar
        
        # Menutup file descriptor standar (stdin, stdout, stderr)
        sys.stdout.flush()
        sys.stderr.flush()
        with open('/dev/null', 'r') as null_in, open('/dev/null', 'a') as null_out:
            os.dup2(null_in.fileno(), sys.stdin.fileno())
            os.dup2(null_out.fileno(), sys.stdout.fileno())
            os.dup2(null_out.fileno(), sys.stderr.fileno())
    except Exception as e:
        print(f"Gagal membuat daemon: {e}")
        sys.exit(1)

def main():
    # Jika dihentikan dengan kill
    signal.signal(signal.SIGTERM, stop_server)
    # Jika ditekan Ctrl+c maka server dihentikan
    signal.signal(signal.SIGINT, stop_server)

    load_config("cgi.conf")

    set_daemon()

    create_log_directory()
    # Server dijalankan
    start_server()
    run_server()

if __name__ == "__main__":
    main()
