import os
import mimetypes
import re
from datetime import datetime
from urllib.parse import unquote
from config import cfg
from fpm import php_fpm_request

# Data struktur untuk RequestHeader dan ResponseHeader
class RequestHeader:
    def __init__(self, directory ="/", method="", uri="", http_version="", query_string = "", path_info="", body_data="", request_time="", content_length=0):
        self.directory = directory
        self.method = method
        self.uri = uri
        self.http_version = http_version
        self.query_string = query_string
        self.path_info =path_info
        self.body_data = body_data 
        self.request_time = request_time
        self.content_length = content_length

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
    # **Pisahkan header dan body berdasarkan pemisah (\r\n\r\n atau \n\n)**
    header_body_split = request.find("\r\n\r\n")
    if header_body_split == -1:
        header_body_split = request.find("\n\n")  # Cek jika pakai LF saja
    header_data = request[:header_body_split] if header_body_split != -1 else request
    body_data = request[header_body_split+4:] if header_body_split != -1 else ""

    # **Pisahkan baris-baris header**
    lines = header_data.splitlines()
    
    # **Parsing Baris Pertama (Request Line)**
    if lines:
        words = lines[0].split(" ", 2)
        if len(words) == 3:
            req_header.method = words[0]
            full_uri = words[1]
            req_header.http_version = words[2]

            # **Pisahkan query string**
            uri, _, query_string = full_uri.partition('?')
            req_header.query_string = unquote(query_string) if query_string else ""
            
            # **Gunakan REGEX untuk Parsing Directory, URI, dan Path Info**
            pattern = r"^(/?.*?/)?([^/?]+\.[a-zA-Z0-9]+)(/[^?]*)?$"
            match = re.match(pattern, uri)
            if match:
                req_header.directory = match.group(1) or "/"
                req_header.uri = match.group(2)
                req_header.path_info = match.group(3) or ""
            else:
                req_header.uri = uri  # Jika regex gagal, gunakan langsung
                
    # **Parsing Header**
    for line in lines[1:]:
        if not line.strip():
            break  # Akhir dari header
        
        if line.startswith("Request-Time: "):
            req_header.request_time = line[len("Request-Time: "):]
        elif line.startswith("Content-Length: "):
            req_header.content_length = int(line[len("Content-Length: "):])

    # **Parsing Body (gunakan Content-Length jika ada)**
    if req_header.content_length > 0:
        req_header.body_data = unquote(body_data[:req_header.content_length])

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
    file_path = os.path.join(cfg.document_root, req_header.directory.lstrip("/"), req_header.uri)
    # print(f"File : {file_path}")
    if not os.path.exists(file_path) or not os.path.isfile(file_path):
        # Handle Not Found
        res_header = ResponseHeader(http_version=req_header.http_version, status_code=404, status_message="Not Found", mime_type="text/html")
        body = "<h1>404 Not Found</h1>".encode()
        res_header.content_length = len(body)
        return create_response(res_header, body)

    # File ditemukan
    # **Jika file adalah PHP, jalankan melalui PHP-FPM**
    if req_header.uri.endswith(".php"):
        php_response = php_fpm_request(
            req_header.directory,
            req_header.uri,
            req_header.method,
            req_header.query_string,
            req_header.path_info,
            req_header.body_data,
            ""
        )

        if php_response.body is None:
            res_header = ResponseHeader(
                http_version=req_header.http_version,
                status_code=500,
                status_message="Internal Server Error",
                mime_type="text/html"
            )
            body = b"<h1>500 Internal Server Error</h1>"
            res_header.content_length = len(body)
            return create_response(res_header, body)

        # **Ambil MIME Type dari Header PHP-FPM (default ke "text/html")**
        mime_type = "text/html"
        for line in php_response.header.split("\n"):
            if line.lower().startswith("content-type:"):
                mime_type = line.split(":", 1)[1].strip()
                break

        res_header = ResponseHeader(
            http_version=req_header.http_version,
            status_code=200,
            status_message="OK",
            mime_type=mime_type,
            content_length=len(php_response.body)
        )
        return create_response(res_header, php_response.body.encode())
    else:
        # Jika bukan PHP
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
