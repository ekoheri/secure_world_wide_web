#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h> // string manipulation
#include <time.h>
#include <sys/time.h>

#include "http.h"
#include "log.h"

RequestHeader parse_request_line(char *request) {
    RequestHeader req_header = {
        .method = "",
        .uri = "",
        .http_version = ""
    };

    char request_message[BUFFER_SIZE];
    char request_line[BUFFER_SIZE];
    char *words[3] = {NULL, NULL, NULL};

    // Inisialisasi semua field struct
    memset(&req_header, 0, sizeof(req_header));

    if (request == NULL || strlen(request) == 0) {
        return req_header;  // Kembalikan struct kosong jika input tidak valid
    }

    // Baca baris pertama dari rangkaian data request
    strncpy(request_message, request, BUFFER_SIZE);
    request_message[BUFFER_SIZE - 1] = '\0'; 
    char *line = strtok(request_message, "\r\n");
    if (line == NULL) {
        return req_header;
    }
    strncpy(request_line, line, BUFFER_SIZE);
    request_line[BUFFER_SIZE - 1] = '\0';

    //log
    write_log(" * Request Line : %s", request_line);

    // Pilah request line berdasarkan spasi
    int i = 0;
    char *token = strtok(request_line, " ");
    while (token != NULL && i < 3) {
        words[i++] = token;
        token = strtok(NULL, " ");
    }

    // Pastikan ada 3 komponen dalam request line
    if (i < 3) {
        return req_header;
    }

    // kata 1 : Method, kata 2 : uri, kata 3 : versi HTTP
    strcpy(req_header.method, words[0]);
    strcpy(req_header.uri, words[1]);
    strcpy(req_header.http_version, words[2]);

    // Hapus tanda / pada URI
    if (req_header.uri[0] == '/') {
        memmove(req_header.uri, req_header.uri + 1, strlen(req_header.uri));
    }

    // Cek apakah ada query string
    char *query_start = strchr(req_header.uri, '?');
    if (query_start != NULL) {
        // Pisahkan query string dari URI
        *query_start = '\0'; 
        // Salin query string
        strcpy(req_header.query_string, query_start + 1); 
    } else {
        // Tidak ada query string
        req_header.query_string[0] = '\0'; 
    }

    // Cek apakah ada body data
    char *body_start = strstr(request, "\r\n\r\n");
    if (body_start != NULL) {
        // Pindahkan pointer ke awal body data
        body_start += 4; // Melewati CRLF CRLF
        // Salin data POST dari body
        strcpy(req_header.post_data, body_start);
    } else {
        req_header.post_data[0] = '\0'; // Tidak ada body data
    }

    // Jika URI kosong, maka isi URI dengan resource default
    // yaitu index.html
    if (strlen(req_header.uri) == 0) {
        strcpy(req_header.uri, "index.html");
    }

    return req_header;  // Kembalikan struct
}

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
}

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
}

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
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "Response-Time: %s\r\n"
        "\r\n",  // Akhir dari header
        res_header.http_version, res_header.status_code, 
        res_header.status_message, res_header.mime_type, responseTime);
    
    // responseTime harus di-free, karena menggunakan malloc
    free(responseTime);
    return header;
}

char *create_response(int *response_size, const ResponseHeader *res_header, const char *body, int body_size) {
    char *response = NULL;

    // Generate response header
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
    write_log(" * Response : %s %d %s", 
        res_header->http_version, 
        res_header->status_code, 
        res_header->status_message);
    return response;
}

char *handle_method(int *response_size, RequestHeader req_header) {
    char *response = NULL;  // Single pointer untuk response
    char *response_header = NULL;

    // Pengecekan apakah method, uri, atau http_version kosong
    // Jika iya maka response 400 Bad Request
    if (strlen(req_header.method) == 0 || strlen(req_header.uri) == 0 || strlen(req_header.http_version) == 0) {
        ResponseHeader res_header = {
            .http_version = "HTTP/1.1",
            .status_code = 400,
            .status_message = "Bad Request",
            .mime_type = "text/html"
        };

        char *_400 = "<h1>400 Bad Request</h1>";
        response = create_response(response_size, &res_header, _400, strlen(_400));
        return response;
    }

    // Buka file (resource) yang diminta oleh web browser
    char fileURL[100];
    snprintf(fileURL, 
        sizeof(fileURL), "%s%s", 
        FOLDER_DOCUMENT, req_header.uri);
    FILE *file = fopen(fileURL, "rb");

    // Jika file tidak ditemukan, kirimkan status 404 Not Found
    if (!file) {
        ResponseHeader res_header = {
            .http_version = req_header.http_version,
            .status_code = 404,
            .status_message = "Not Found",
            .mime_type = "text/html"
        };

        char *_404 = "<h1>Not Found</h1>";
        response = create_response(response_size, &res_header, _404 , strlen(_404));
        return response;
    } 

    // Jika file resource ditemukan
    if (file) {
        //Jika ekstensinya PHP, maka jalankan CGI
        const char *extension = strrchr(req_header.uri, '.');
        if (extension && strcmp(extension, ".php") == 0) {
            // Jalankan CGI
            char command[BUFFER_SIZE];
            snprintf(command, sizeof(command),
                "%s --target=%s"
                " --method=%s "
                " --data_query_string=\"%s\""
                " --data_post=\"%s\"",
                CGI_PATH, fileURL, req_header.method, 
                req_header.query_string, req_header.post_data);
                
            FILE *fp = popen(command, "r");
            if (!fp) {
                perror("popen");
                exit(EXIT_FAILURE);
            }

            // Baca output dari program CGI
            // Baca output dari program CGI
            char response_body[BUFFER_SIZE];
            size_t output_len = 0;
            while (
                fgets(response_body + output_len, 
                sizeof(response_body) - output_len, fp) != NULL) {
                output_len += strlen(response_body + output_len);
            }

            pclose(fp);

            ResponseHeader res_header = {
                .http_version = req_header.http_version,
                .status_code = 200,
                .status_message = "OK",
                .mime_type = "text/html"
            };
            
            response = create_response(response_size, &res_header, response_body, output_len);
            return response;
        } else {
            // Jika file resource ditemukan
            // Ambil data MIME type
            const char *mime = get_mime_type(req_header.uri);

            // Generate header 200 OK
            ResponseHeader res_header = {
                .http_version = req_header.http_version,
                .status_code = 200,
                .status_message = "OK",
                .mime_type = mime
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
        } // end if not php
    } //end if found
}
