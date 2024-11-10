import os
import mimetypes
from datetime import datetime, timezone
from template_engine import \
    parse_template, \
    compile_template

BUFFER_SIZE = 1024
FOLDER_DOCUMENT = "dokumen/"

class RequestInfo:
    def __init__(self):
        self.method = ""
        self.uri = ""
        self.http_version = ""
        self.query_string = ""
        self.post_data = ""
#End class RequestInfo

def parse_request_line(request):
    req_info = RequestInfo()
    if not request:
        return req_info
    # End if not request
    lines = request.splitlines()
    request_line = lines[0] if lines else ""
    words = request_line.split(" ")

    if len(words) < 3:
        print("Error: Request line tidak lengkap")
        return req_info
    # End if len(words)
    req_info.method, req_info.uri, req_info.http_version = words[:3]
    
    # Cek apakah ada query string
    query_start = req_info.uri.find("?")
    if query_start != -1:
        # Pisahkan query string dari URI
        req_info.query_string = req_info.uri[query_start + 1:]
        req_info.uri = req_info.uri[:query_start]
    else:
        req_info.query_string = ""
    #End if query_start
    # Cek apakah ada body data
    body_start = request.find("\r\n\r\n")
    if body_start != -1:
        # Pindahkan pointer ke awal body data
        body_start += 4  # Melewati CRLF CRLF
        # Salin data POST dari body
        req_info.post_data = request[body_start:]
    else:
        req_info.post_data = ""
    #End if body_start
    if req_info.uri.startswith('/'):
        req_info.uri = req_info.uri[1:]
    #End if req_info.uri
    if not req_info.uri:
        req_info.uri = "index.html"
    #End if not req_info.uri
    return req_info
#End def parse_request_line

def get_time_string():
    dt = datetime.now(timezone.utc)
    millis = int(dt.microsecond / 1000)
    return dt.strftime( \
        "%a, %d %b %Y %H:%M:%S") + \
        f".{millis:03d} GMT"
#End def get_time_string

def generate_response_header( \
    http_version, \
    status_code, \
    status_message, mime_type):

    response_time = get_time_string()
    header = (
        f"{http_version} {status_code} {status_message}\r\n"
        f"Content-Type: {mime_type}\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        f"Response-Time: {response_time}\r\n"
        "\r\n"
    )
    return header
#End def generate_response_header

def handle_method(req_info):
    file_path = os.path.join(FOLDER_DOCUMENT, req_info.uri)
    
    if not os.path.exists(file_path):
        # File tidak ditemukan
        response_header = generate_response_header(
            req_info.http_version, 404, "Not Found", "text/html"
        )
        response_body = "<h1>Not Found</h1>"
        response = (response_header + response_body).encode()
    else:
        # File ditemukan
        mime_type, _ = mimetypes.guess_type(req_info.uri)
        if mime_type is None:
            mime_type = "text/html"
        #End if mime_type
        response_header = generate_response_header(
            req_info.http_version, 200, "OK", mime_type
        )
        with open(file_path, "r") as file:
            file_content = file.read()
        if mime_type == "text/html" and "{%" in file_content:
            tokens = parse_template(file_content)
            if(req_info.method == "GET"):
                compiled_content = compile_template( \
                    tokens, query_string=req_info.query_string)
            # End if(req_info.method == "GET")
            if(req_info.method == "POST"):
                compiled_content = compile_template( \
                    tokens, post_data=req_info.post_data)
            # End if(req_info.method == "POST")
            response_body = compiled_content.encode('utf-8')
        else:
            with open(file_path, "rb") as file:
                response_body = file.read()
        #End if mime_type
        response = response_header.encode() + response_body
    # End else file ditemukan
    return response
# End def handle_method
