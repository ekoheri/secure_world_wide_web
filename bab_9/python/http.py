import os
import mimetypes
import subprocess
import sys
from datetime import datetime
from config import cfg
from log import write_log

# Data struktur untuk RequestHeader dan ResponseHeader
class RequestHeader:
    def __init__(self, method="", uri="", http_version="", query_string="", post_data = ""):
        self.method = method
        self.uri = uri
        self.http_version = http_version
        self.query_string = query_string
        self.post_data = post_data

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
        # print("Request yang diterima tidak valid:", request)  # Debug log
        write_log("Request tidak valid: %s", request)
        return req_header

    request_lines = request.split("\r\n")
    if not request_lines:
        return req_header

    # Ambil baris pertama (request line)
    request_line = request_lines[0]
    words = request_line.split()
    if len(words) < 3:
        return req_header

    req_header.method = words[0]
    req_header.uri = words[1].lstrip("/")  # Hilangkan leading slash
    req_header.http_version = words[2]

    # Cek apakah ada query string di URI
    query_start = req_header.uri.find("?")
    if query_start != -1:
        # Pisahkan query string dari URI
        req_header.query_string = req_header.uri[query_start + 1:]
        req_header.uri = req_header.uri[:query_start]
    
    # Cek apakah ada body data
    body_start = request.find("\r\n\r\n")
    if body_start != -1:
        # Salin data POST dari body
        req_header.post_data = request[body_start + 4:]

    if not req_header.uri:  # Jika URI kosong, gunakan halaman default
        req_header.uri = "index.html"

    write_log(" * Request: %s %s %s", \
        req_header.method, \
        req_header.uri, \
        req_header.http_version)
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
    write_log(" * Respose: %s %s %s", \
        res_header.http_version, \
        res_header.status_code, \
        res_header.status_message)
    return response

def run_php_script(target, query_string, post_data):
    command = (
        f"php -r 'parse_str(\"{query_string}\", $_GET); "
        f"parse_str(\"{post_data}\", $_POST); "
        f"include \"{target}\";'"
    )

    # Buffer untuk menyimpan seluruh respons
    response = ""
    try:
        # Menjalankan skrip PHP dan menangkap outputnya
        result = subprocess.run(command, shell=True, capture_output=True, text=True)

        # Mengecek jika ada kesalahan dalam proses PHP
        if result.returncode != 0:
            # Menambahkan error PHP ke response
            response += "<p>Terjadi kesalahan dalam menjalankan skrip PHP.</p>\n"
            response += result.stderr  # Menambahkan stderr (error dari PHP) jika ada
        else:
            # Menambahkan output PHP ke response
            response += result.stdout

    except Exception as e:
        # Jika terjadi kesalahan pada eksekusi, tangani error-nya
        response += f"<p>Terjadi kesalahan dalam menjalankan skrip PHP: {str(e)}</p>\n"

    return response  # Mengembalikan respons lengkap
#End def 

# Fungsi untuk menangani method (misalnya GET)
def handle_method(req_header):
    if not req_header.method or not req_header.uri or not req_header.http_version:
        # Handle Bad Request
        res_header = ResponseHeader(
            status_code=400, 
            status_message="Bad Request", 
            mime_type="text/html"
        )
        body = "<h1>400 Bad Request</h1>".encode()
        res_header.content_length = len(body)
        return create_response(res_header, body)

    # Tentukan path file yang diminta
    file_path = os.path.join(cfg.document_root, req_header.uri)
    if not os.path.exists(file_path) or not os.path.isfile(file_path):
        # Handle Not Found
        res_header = ResponseHeader(
            http_version=req_header.http_version, 
            status_code=404, 
            status_message="Not Found", 
            mime_type="text/html"
        )
        body = "<h1>404 Not Found</h1>".encode()
        res_header.content_length = len(body)
        return create_response(res_header, body)

    _, extension = os.path.splitext(req_header.uri)
    if extension == ".php":
        res_header = ResponseHeader(
            http_version=req_header.http_version,
            status_code=200,
            status_message="OK",
            mime_type="text/html"
        )
        
        if req_header.method == "POST":
            body_content = run_php_script(file_path, "", req_header.post_data)
        elif req_header.method == "GET":
            body_content = run_php_script(file_path, req_header.query_string, "")

        res_header.content_length = len(body_content)        
        body = body_content.encode()
        return create_response(res_header, body)
