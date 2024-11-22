import socket
import selectors
import signal
import os
import fcntl
import errno
from http import parse_request_line, handle_method  # Mengimpor dari modul yang Anda buat
from config import Config, load_config  # Mengimpor dari modul konfigurasi
from log import write_log, create_log_directory  # Mengimpor dari modul log

config = load_config("server.conf")
MAX_EVENTS = config.max_event

sel = selectors.DefaultSelector()
sock_server = None


def set_nonblocking(sock):
    """Set the socket to non-blocking mode."""
    flags = fcntl.fcntl(sock, fcntl.F_GETFL)
    fcntl.fcntl(sock, fcntl.F_SETFL, flags | os.O_NONBLOCK)


def start_server():
    """Initialize and start the server."""
    global sock_server
    sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR | socket.SO_REUSEPORT, 1)

    set_nonblocking(sock_server)

    # Bind the socket to IP and port
    sock_server.bind((config.server_name, config.server_port))
    sock_server.listen(3)

    # Register the server socket for epoll events
    sel.register(sock_server, selectors.EVENT_READ, accept_connection)

    write_log("Web Server sedang berjalan")


def accept_connection(sock_server):
    """Accept a new client connection."""
    try:
        sock_client, client_addr = sock_server.accept()
        write_log(f"Proses accept dari {client_addr[0]}")
        set_nonblocking(sock_client)
        sel.register(sock_client, selectors.EVENT_READ, handle_client)
    except BlockingIOError:
        pass


def handle_client(sock_client):
    """Handle a request from a connected client."""
    try:
        request = sock_client.recv(config.request_buffer_size)
        if not request:
            sel.unregister(sock_client)
            sock_client.close()
            return

        request_line = request.decode().splitlines()[0]
        req_header = parse_request_line(request_line)
        response, response_size = handle_method(req_header)

        if response:
            sock_client.sendall(response.encode())
        else:
            write_log("Response data ke browser NULL")
        sel.unregister(sock_client)
        sock_client.close()
    except Exception as e:
        write_log(f"Error handling client: {e}")
        sel.unregister(sock_client)
        sock_client.close()


def stop_server(signal, frame):
    """Gracefully stop the server."""
    global sock_server
    write_log("Web Server telah dihentikan.")
    if sock_server:
        sock_server.close()
    sel.close()
    os._exit(0)


def set_daemon():
    """Daemonize the server process."""
    pid = os.fork()
    if pid > 0:
        os._exit(0)

    os.setsid()

    # Change the working directory to root
    os.chdir("/")

    # Redirect stdin, stdout, stderr to /dev/null
    with open("/dev/null", "r+b") as devnull:
        os.dup2(devnull.fileno(), 0)
        os.dup2(devnull.fileno(), 1)
        os.dup2(devnull.fileno(), 2)


def run_server():
    """Run the server to handle incoming connections and requests."""
    while True:
        events = sel.select()
        for key, _ in events:
            callback = key.data
            callback(key.fileobj)


if __name__ == "__main__":
    # Handle SIGINT and SIGTERM to stop the server
    signal.signal(signal.SIGTERM, stop_server)
    signal.signal(signal.SIGINT, stop_server)

    # Load configuration
    load_config("server.conf")

    # Make the server a daemon
    set_daemon()

    # Create log directory
    create_log_directory()

    # Start and run the server
    start_server()
    run_server()