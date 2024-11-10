import os
import mimetypes
from datetime import datetime, timezone

BUFFER_SIZE = 1024
FOLDER_DOCUMENT = "dokumen/"

class RequestInfo:
    def __init__(self):
        self.method = ""
        self.uri = ""
        self.http_version = ""
#End class RequestInfo

def parse_request_line(request):
    req_info = RequestInfo()
    if not request:
        return req_info

    lines = request.splitlines()
    request_line = lines[0] if lines else ""
    words = request_line.split(" ")

    if len(words) < 3:
        print("Error: Request line tidak lengkap")
        return req_info

    req_info.method, req_info.uri, req_info.http_version = words[:3]
    
    if req_info.uri.startswith('/'):
        req_info.uri = req_info.uri[1:]
    
    if not req_info.uri:
        req_info.uri = "index.html"
    
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
        response_header = generate_response_header(
            req_info.http_version, 200, "OK", mime_type
        )

        with open(file_path, "rb") as file:
            response_body = file.read()
        
        response = response_header.encode() + response_body

    return response
# End def handle_method
