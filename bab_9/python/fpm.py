import socket
import struct
from config import cfg

BUFFER_SIZE = 4096
FCGI_VERSION_1 = 1
FCGI_BEGIN_REQUEST = 1
FCGI_PARAMS = 4
FCGI_RESPONDER = 1
FCGI_STDIN = 5

class ResponsePHPFPM:
    def __init__(self, header: str, body: str):
        self.header = header
        self.body = body

def create_fcgi_header(type, request_id, content_length, padding_length=0):
    return struct.pack(
        "!BBHHBB",
        FCGI_VERSION_1,
        type,
        request_id,
        content_length,
        padding_length,
        0
    )

def send_fcgi_begin_request(sock, request_id):
    body = struct.pack("!HB5x", FCGI_RESPONDER, 0)
    header = create_fcgi_header(FCGI_BEGIN_REQUEST, request_id, len(body))
    sock.sendall(header + body)

def encode_fcgi_params(params):
    result = b''
    for name, value in params.items():
        name_bytes = name.encode()
        value_bytes = value.encode()
        result += struct.pack("B", len(name_bytes)) + struct.pack("B", len(value_bytes))
        result += name_bytes + value_bytes
    return result

def send_fcgi_params(sock, request_id, params):
    encoded_params = encode_fcgi_params(params)
    header = create_fcgi_header(FCGI_PARAMS, request_id, len(encoded_params))
    sock.sendall(header + encoded_params)
    empty_header = create_fcgi_header(FCGI_PARAMS, request_id, 0)
    sock.sendall(empty_header)

def send_fcgi_stdin(sock, request_id, data):
    header = create_fcgi_header(FCGI_STDIN, request_id, len(data))
    sock.sendall(header + data.encode())
    empty_header = create_fcgi_header(FCGI_STDIN, request_id, 0)
    sock.sendall(empty_header)

def receive_fcgi_response(sock):
    buffer = b''
    while True:
        data = sock.recv(BUFFER_SIZE)
        if not data:
            break
        buffer += data

    if len(buffer) < 8:
        return ResponsePHPFPM("", "")

    # ** Parsing FastCGI Header (8 byte pertama) **
    fcgi_header = buffer[:8]  # Ambil header FastCGI
    version, fcgi_type, request_id, content_length, padding_length, reserved = struct.unpack("!BBHHBB", fcgi_header)

    # ** Ambil HTTP Header dan Body **
    content_data = buffer[8: 8 + content_length]  # Ambil hanya data konten
    padding_end = 8 + content_length + padding_length  # Lompat padding

    header_end = content_data.find(b"\r\n\r\n")  # Cek pemisah header-body
    if header_end != -1:
        header_data = content_data[:header_end].decode(errors="ignore").strip()
        body_data = content_data[header_end+4:].decode(errors="ignore").strip("\x00")
    else:
        header_data = ""
        body_data = content_data.decode(errors="ignore").strip("\x00")

    #print(f"Raw Header: {header_data}")
    #print(f"Raw Body: {body_data.encode('utf-8')}")

    return ResponsePHPFPM(header_data, body_data)

def php_fpm_request(directory, script_name, method, query_string, path_info, post_data, content_type):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("127.0.0.1", 9000))
    
    request_id = 1
    params = {
        "SCRIPT_FILENAME": f"{cfg.document_root}/{directory}/{script_name}",
        "SCRIPT_NAME": script_name,
        "REQUEST_METHOD": method,
        "QUERY_STRING": query_string,
        "PATH_INFO": path_info,
        "REQUEST_URI": script_name + ("?" + query_string if query_string else ""),
        "SERVER_PROTOCOL": "HTTP/1.1",
        "GATEWAY_INTERFACE": "CGI/1.1",
        "REMOTE_ADDR": "127.0.0.1",
        "REMOTE_PORT": "12345",
        "SERVER_SOFTWARE": "Python-FCGI",
        "HTTP_HOST": "localhost",
        "CONTENT_LENGTH": str(len(post_data)) if post_data else "0",
        "CONTENT_TYPE": content_type if post_data else ""
    }
    
    send_fcgi_begin_request(sock, request_id)
    send_fcgi_params(sock, request_id, params)
    send_fcgi_stdin(sock, request_id, post_data)
    response = receive_fcgi_response(sock)
    sock.close()
    return response

#if __name__ == "__main__":
#    response_fpm = php_fpm_request("/var/www/html", "test_qs.php", "GET", "nama=eko heri", "", "", "")
#    print(f"Header: {response_fpm.header}")
#    print(f"Body: {response_fpm.body}")
