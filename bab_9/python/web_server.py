import socket
import signal
import sys
from http import handle_method, parse_request_line, RequestHeader
from config import cfg, load_config

sock_server = None
addrlen = 0

def start_server():
    global sock_server
    opt = 1

    # Inisialisasi socket server
    sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    if sock_server == 0:
        print("Inisialisasi socket server gagal", file=sys.stderr)
        sys.exit(1)

    # Mengatur opsi socket
    sock_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR | socket.SO_REUSEPORT, opt)

    # Bind IP Address dengan port
    server_address = (cfg.server_name, cfg.server_port)
    try:
        sock_server.bind(server_address)
    except Exception as e:
        print(f"Proses bind gagal: {e}", file=sys.stderr)
        sock_server.close()
        sys.exit(1)

    # Listen untuk mendengarkan koneksi masuk
    sock_server.listen(3)
    print("Server sedang berjalan.")
    print("Tekan Ctrl+C untuk menghentikan.")
    print(f"Akses URL http://{cfg.server_name}:{cfg.server_port}")


def handle_client(sock_client):
    try:
        request = sock_client.recv(cfg.request_buffer_size)
        if not request:
            return
        
        request_str = request.decode("utf-8")
        req_header = parse_request_line(request_str)

        # Handle method and get response
        response = handle_method(req_header)
        if response:
            sock_client.sendall(response)
    except Exception as e:
        print(f"Proses menangani request client gagal: {e}", file=sys.stderr)
    finally:
        sock_client.close()


def run_server():
    if sock_server:
        while True:
            try:
                sock_client, client_addr = sock_server.accept()
                client_ip = client_addr[0]
                print(f"Proses accept dari {client_ip} berhasil")

                # Panggil fungsi handle_client
                handle_client(sock_client)
            except Exception as e:
                print(f"Proses accept gagal: {e}", file=sys.stderr)
                continue


def stop_server(signal, frame):
    if sock_server:
        sock_server.close()
    print("\nServer telah dihentikan.")
    sys.exit(0)


def main():
    # Jika ditekan Ctrl+c maka server dihentikan
    signal.signal(signal.SIGINT, stop_server)

    load_config("server.conf")

    # Server dijalankan
    start_server()
    run_server()


if __name__ == "__main__":
    main()
