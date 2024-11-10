import socket

BUFFER_SIZE = 1024
RESPONSE_SIZE = 8192
PORT = 80

def handle_url(url):
    http_prefix = "http://"
    if url.startswith(http_prefix):
        url = url[len(http_prefix):]  # Menghapus 'http://'
    colon_pos = url.find(':')
    slash_pos = url.find('/')
    # Inisialisasi path dan port default
    path = '/'
    port = PORT
    hostname = ""
    if colon_pos != -1 and \
    (slash_pos == -1 or colon_pos < slash_pos):
        # URL mengandung port
        hostname = url[:colon_pos]  # Ambil hostname
        port = int(url[colon_pos + 1:slash_pos]) \
            if slash_pos != -1 \
            else int(url[colon_pos + 1:]) # Ambil port
        if slash_pos != -1:
            path = url[slash_pos:]  # Ambil path
    elif slash_pos != -1:
        hostname = url[:slash_pos]  # Ambil hostname
        path = url[slash_pos:]  # Ambil path
    else:
        hostname = url  # Hanya hostname
    return hostname, path, port
#End handle_url

def client_request(url):
    hostname, path, port = handle_url(url)
    
    sock = socket.socket( \
        socket.AF_INET, \
        socket.SOCK_STREAM)
    sock.connect((hostname, port))

    # Membuat permintaan HTTP GET
    request = (f"GET {path} HTTP/1.1\r\n"
            f"Host: {hostname}\r\n"
            f"User-Agent: EkoBrowser/1.1\r\n"
            f"Connection: close\r\n\r\n")
    sock.sendall(request.encode())

    response = b""
    while True:
        data = sock.recv(BUFFER_SIZE)
        if not data:
            break
        response += data

    sock.close()
    return response.decode()
#End request_client

def parse_tag_html(response, tag_open, tag_close, tag_name):

    start = 0
    while True:
        start = response.find(tag_open, start)
        if start == -1:
            break
        start += len(tag_open)
        end = response.find(tag_close, start)
        if end == -1:
            break
        print(f"{tag_name}: {response[start:end]}")
        start = end + len(tag_close)
#End parse_tag_html

def display_html():
    url = input("Masukkan URL: ")
    response = client_request(url)
    
    print("Display HTML:")
    parse_tag_html(response, "<h1>", "</h1>", "H1")
    parse_tag_html(response, "<p>", "</p>", "P")
    parse_tag_html(response, "<a>", "</a>", "Hyperlink")
#End display_html

if __name__ == "__main__":
    display_html()
