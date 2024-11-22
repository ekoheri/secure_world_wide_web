import os
import mimetypes
from datetime import datetime
from config import cfg

# Data struktur untuk RequestHeader dan ResponseHeader
class RequestHeader:
    def __init__(self, method="", uri="", http_version=""):
        self.method = method
        self.uri = uri
        self.http_version = http_version

class ResponseHeader:
    def __init__(self, http_version="HTTP/1.1", status_code=200, status_message="OK", mime_type="text/html", content_length=0):
        self.http_version = http_version
        self.status_code = status_code
        self.status_message = status_message
        self.mime_type = mime_type
        self.content_length = content_length

# Fungsi untuk parsing request line
def parse_request_line(request):
    req_header = RequestHeader()
    # Pastikan request yang diterima adalah string
    if not isinstance(request, str):
        print("Request yang diterima tidak valid:", request)  # Debug log
        return req_header

    request_lines = request.split("\r\n")
    #if not request_lines:
    #    return req_header

    # Ambil baris pertama (request line)
    request_line = request_lines[0]
    words = request_line.split()
    if len(words) < 3:
        return req_header

    req_header.method = words[0]
    req_header.uri = words[1].lstrip("/")  # Hilangkan leading slash
    req_header.http_version = words[2]

    if not req_header.uri:  # Jika URI kosong, gunakan halaman default
        req_header.uri = cfg.default_page

    return req_header

# Fungsi untuk mendapatkan MIME type dari file
def get_mime_type(file):
    mime_type, _ = mimetypes.guess_type(file)
    return mime_type or "text/html"

# Fungsi untuk mendapatkan waktu dalam format HTTP
def get_time_string():
    now = datetime.utcnow()
    return now.strftime("%a, %d %b %Y %H:%M:%S GMT")

# Fungsi untuk membuat header response HTTP
def generate_response_header(res_header):
    response_time = get_time_string()
    header = (
        f"{res_header.http_version} {res_header.status_code} {res_header.status_message}\r\n"
        f"Content-Type: {res_header.mime_type}\r\n"
        f"Content-Length: {res_header.content_length}\r\n"
        f"Connection: close\r\n"
        f"Cache-Control: no-cache\r\n"
        f"Response-Time: {response_time}\r\n"
        "\r\n"  # Akhir dari header
    )
    return header

# Fungsi untuk membuat response
def create_response(res_header, body):
    response_header = generate_response_header(res_header)
    response = response_header.encode() + body
    return response

# Fungsi untuk menangani method (misalnya GET)
def handle_method(req_header):
    if not req_header.method or not req_header.uri or not req_header.http_version:
        # Handle Bad Request
        res_header = ResponseHeader(status_code=400, status_message="Bad Request", mime_type="text/html")
        body = "<h1>400 Bad Request</h1>".encode()
        res_header.content_length = len(body)
        return create_response(res_header, body)

    # Tentukan path file yang diminta
    file_path = os.path.join(cfg.document_root, req_header.uri)
    if not os.path.exists(file_path) or not os.path.isfile(file_path):
        # Handle Not Found
        res_header = ResponseHeader(http_version=req_header.http_version, status_code=404, status_message="Not Found", mime_type="text/html")
        body = "<h1>404 Not Found</h1>".encode()
        res_header.content_length = len(body)
        return create_response(res_header, body)

    # File ditemukan
    mime_type = get_mime_type(file_path)
    with open(file_path, "rb") as file:
        body = file.read()

    res_header = ResponseHeader(
        http_version=req_header.http_version,
        status_code=200,
        status_message="OK",
        mime_type=mime_type,
        content_length=len(body)
    )
    return create_response(res_header, body)
