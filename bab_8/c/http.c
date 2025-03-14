#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "http.h"
#include "config.h"
#define BUFFER_SIZE 1024

extern Config config;

RequestHeader parse_request_line(char *request) {
    RequestHeader req_header = {0}; // Inisialisasi semua field ke 0
    const char *line = request;
    int line_count = 0;

    while (line && *line) {
        const char *next_line = strstr(line, "\r\n");
        size_t line_length = next_line ? (size_t)(next_line - line) : strlen(line);
        
        if (line_length == 0) {
            line = next_line ? next_line + 2 : NULL;
            break;
        }

        char *line_copy = strndup(line, line_length);

        if (line_count == 0) { // Request Line (Baris Pertama)
            char *words[3] = {NULL, NULL, NULL};
            int i = 0;
            char *token = strtok(line_copy, " ");

            while (token && i < 3) {
                words[i++] = token;
                token = strtok(NULL, " ");
            }

            if (i == 3) {
                strncpy(req_header.method, words[0], sizeof(req_header.method) - 1);
                req_header.uri = strdup(words[1]);
                strncpy(req_header.http_version, words[2], sizeof(req_header.http_version) - 1);

                // Hapus tanda '/' pada URI jika ada
                if (req_header.uri[0] == '/') {
                    memmove(req_header.uri, req_header.uri + 1, strlen(req_header.uri));
                }

            }
        }

        free(line_copy);
        line = next_line ? next_line + 2 : NULL; // Pindah ke baris berikutnya
        line_count++;
    }

    // Jika URI kosong, gunakan "index.html"
    if (!req_header.uri || strlen(req_header.uri) == 0) {
        free(req_header.uri);
        req_header.uri = strdup("index.html");
    }

    return req_header;
} //end parse_request_line

const char *get_mime_type(const char *file) {
    // Cari extension dari file
    const char *dot = strrchr(file, '.');

    // Jika tidak ditemukan extension atau MIME type yang cocok,
    // kembalikan "text/html" sebagai default
    if (!dot) return "text/html";
    else if (strcmp(dot, ".html") == 0) return "text/html";
    else if (strcmp(dot, ".css") == 0) return "text/css";
    else if (strcmp(dot, ".js") == 0) return "application/js";
    else if (strcmp(dot, ".jpg") == 0) return "image/jpeg";
    else if (strcmp(dot, ".png") == 0) return "image/png";
    else if (strcmp(dot, ".gif") == 0) return "image/gif";
    else if (strcmp(dot, ".ico") == 0) return "image/ico";
    else return "text/html";  // Default MIME type
} //end get_mime_type

char *get_time_string() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // Mengambil waktu dalam format struct tm (GMT)
    struct tm *tm_info = gmtime(&tv.tv_sec);

    // Alokasikan buffer yang cukup besar untuk waktu dan milidetik
    char *buf = (char *)malloc(64);  // Ukuran buffer yang memadai
    if (!buf) return NULL;  // Cek jika malloc gagal

    // Format waktu tanpa milidetik terlebih dahulu
    strftime(buf, 64, "%a, %d %b %Y %H:%M:%S", tm_info);
    
    // Tambahkan milidetik ke string
    int millis = tv.tv_usec / 1000;
    snprintf(buf + strlen(buf), 64 - strlen(buf), ".%03d GMT", millis);

    return buf;
} //end get_time_string

char *generate_response_header(ResponseHeader res_header) {
    char *header = malloc(BUFFER_SIZE);
    if (header == NULL) {
        return NULL;
    }

    // Ambil tanggal dan jam
    char *responseTime = get_time_string();

    snprintf(
        header, BUFFER_SIZE,
        "%s %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "Response-Time: %s\r\n"
        "\r\n",  // Akhir dari header
        res_header.http_version, res_header.status_code, 
        res_header.status_message, res_header.mime_type, 
        res_header.content_length, responseTime);
    
    // responseTime harus di-free, karena menggunakan malloc
    free(responseTime);
    return header;
} // generate_response_header

char *create_response(
    int *response_size, 
    ResponseHeader *res_header, 
    const char *body, int body_size) {

    char *response = NULL;

    // Generate response header
    res_header->content_length = body_size;
    char *response_header = generate_response_header(*res_header);
    if (!response_header) return NULL;

    // Allocate memory for full response
    int header_size = strlen(response_header);
    response = (char *)malloc(header_size + body_size);
    if (response) {
        memcpy(response, response_header, header_size);   // Copy header
        memcpy(response + header_size, body, body_size);  // Copy body
        *response_size = header_size + body_size;
    }

    free(response_header);

    //log
    printf(" * Response : %s %d %s\n", 
        res_header->http_version, 
        res_header->status_code, 
        res_header->status_message);
    return response;
}

char *handle_method(int *response_size, RequestHeader req_header) {
    char *response = NULL;  // Single pointer untuk response
    char *response_header = NULL;

    // Pengecekan apakah method, uri, atau http_version kosong
    if (strlen(req_header.method) == 0 || strlen(req_header.uri) == 0 || strlen(req_header.http_version) == 0) {
        ResponseHeader res_header = {
            .http_version = "HTTP/1.1",
            .status_code = 400,
            .status_message = "Bad Request",
            .mime_type = "text/html",
            .content_length = 0
        };

        char *_400 = "<h1>400 Bad Request</h1>";
        response = create_response(response_size, &res_header, _400, strlen(_400));
        return response;
    }

    // Buka file (resource) yang diminta oleh web browser
    char fileURL[256];
    snprintf(fileURL, sizeof(fileURL), "%s%s", config.document_root, req_header.uri);
    FILE *file = fopen(fileURL, "rb");

    if (!file) {
        // Jika file tidak ditemukan, kirimkan status 404 Not Found
        ResponseHeader res_header = {
            .http_version = req_header.http_version,
            .status_code = 404,
            .status_message = "Not Found",
            .mime_type = "text/html",
            .content_length = 0
        };

        char *_404 = "<h1>Not Found</h1>";
        response = create_response(response_size, &res_header, _404 , strlen(_404));
        return response;
    } 

    if(file) {
        // Jika file resource ditemukan
        // Ambil data MIME type
        const char *mime = get_mime_type(req_header.uri);

        // Generate header 200 OK
        ResponseHeader res_header = {
            .http_version = req_header.http_version,
            .status_code = 200,
            .status_message = "OK",
            .mime_type = mime,
            .content_length = 0
        };

        // Baca file resource dari server
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *response_body = (char *)malloc(fsize + 1);
        fread(response_body, 1, fsize, file);
        fclose(file);

        response = create_response(response_size, &res_header, response_body, fsize);
        free(response_body);
        return response;  // Kembalikan response sebagai return value
    }
} // handle_method
