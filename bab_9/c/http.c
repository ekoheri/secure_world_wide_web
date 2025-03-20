#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <regex.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "http.h"
#include "config.h"
#include "fpm.h"

#define BUFFER_SIZE 1024

extern Config config;

int request_buffer_size = BUFFER_SIZE * 4;
int response_buffer_size = BUFFER_SIZE * 8;

// Fungsi untuk mengonversi hex menjadi karakter
char hex_to_char(char first, char second) {
    char hex[3] = {first, second, '\0'};
    return (char) strtol(hex, NULL, 16);
}

// Fungsi URL decoding
void url_decode(const char *src, char *dest) {
    while (*src) {
        if (*src == '%') {
            if (isxdigit(src[1]) && isxdigit(src[2])) {
                *dest++ = hex_to_char(src[1], src[2]);
                src += 3;  // Lewati %xx
            } else {
                *dest++ = *src++;
            }
        } else if (*src == '+') {
            *dest++ = ' ';  // Konversi '+' menjadi spasi
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';  // Null-terminate string hasil decode
}

RequestHeader parse_request_line(char *request) {
    RequestHeader req_header = {0};  // Inisialisasi struktur
    req_header.directory = strdup("/");
    req_header.uri = strdup("index.html");
    req_header.query_string = strdup("");
    req_header.path_info = strdup("");
    req_header.request_time = strdup("");
    req_header.content_type = strdup("");
    req_header.content_length = 0;
    req_header.body_data = strdup("");

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
        if (!line_copy) break;

        if (line_count == 0) {  // **Parsing Baris Pertama**
            char *words[3] = {NULL, NULL, NULL};
            int i = 0;
            char *token = strtok(line_copy, " ");
            while (token && i < 3) {
                words[i++] = token;
                token = strtok(NULL, " ");
            }

            if (i == 3) {
                strncpy(req_header.method, words[0], sizeof(req_header.method) - 1);
                free(req_header.uri);
                req_header.uri = strdup(words[1]);
                strncpy(req_header.http_version, words[2], sizeof(req_header.http_version) - 1);

                char *original_uri = strdup(words[1]);  
                if (!original_uri) return req_header;

                // Pisahkan query string jika ada
                char *query_start = strchr(original_uri, '?');
                if (query_start) {
                    *query_start = '\0';  // Pisahkan URI dan Query String
                    char *query_decoded = malloc(strlen(query_start + 1) + 1);
                    if (query_decoded) {
                        url_decode(query_start + 1, query_decoded);
                        req_header.query_string = query_decoded;
                    } 
                } 

                // **Gunakan REGEX untuk Parsing directory, URI dan PATH INFO**
                regex_t regex;
                regmatch_t matches[4];
                const char *pattern = "^(/?.*/)?([^/]+\\.[a-zA-Z0-9]+)(/.*)?$";
                regcomp(&regex, pattern, REG_EXTENDED);

                if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
                    fprintf(stderr, "Regex compilation failed!\n");
                    return req_header;
                }
                
                if (regexec(&regex, original_uri, 4, matches, 0) == 0) {
                    if (matches[1].rm_so != -1) {  // Directory
                        free(req_header.directory);
                        req_header.directory = strndup(original_uri + matches[1].rm_so,
                                                       matches[1].rm_eo - matches[1].rm_so);
                    }
                    if (matches[2].rm_so != -1) {  // URI (File)
                        free(req_header.uri);
                        req_header.uri = strndup(original_uri + matches[2].rm_so,
                                                 matches[2].rm_eo - matches[2].rm_so);
                    }
                    if (matches[3].rm_so != -1 && matches[3].rm_eo > matches[3].rm_so) {  
                        free(req_header.path_info);
                        req_header.path_info = strndup(original_uri + matches[3].rm_so, 
                                                    matches[3].rm_eo - matches[3].rm_so);
                    } 
                } else {
                    printf("Regex tidak cocok, periksa pola regex!\n");
                    return req_header; // Hindari segfault dengan keluar lebih awal
                }
                regfree(&regex);
            }
            
        } else { // Header Lines
            if (strncmp(line_copy, "Request-Time: ", 14) == 0) {
                req_header.request_time = strdup(line_copy + 14);
            } else if (strncmp(line_copy, "Content-Type: ", 14) == 0) {
                req_header.content_type = strdup(line_copy + 14);
            } else if (strncmp(line_copy, "Content-Length: ", 16) == 0) {
                req_header.content_length = atoi(line_copy + 16);
            }
        }

        free(line_copy);
        line = next_line ? next_line + 2 : NULL;
        line_count++;
    }

    if (line && *line) {
        char *body_decoded = malloc(strlen(line) + 1);
        if (body_decoded) {
            url_decode(line, body_decoded);
            req_header.body_data = body_decoded;
        }
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
    printf(" * Response : %s %d %s\n", res_header->http_version, res_header->status_code, res_header->status_message);
    return response;
}

char* get_content_type(const char *header) {
    const char *content_type_start = strstr(header, "Content-Type:");
    if (!content_type_start) return NULL;  // Jika tidak ditemukan, return NULL

    content_type_start += 13;  // Geser ke setelah "Content-Type:"

    // Hilangkan spasi yang mungkin ada di depan
    while (*content_type_start == ' ') {
        content_type_start++;
    }

    // Ambil nilai Content-Type
    char *content_type = strdup(content_type_start);
    char *newline_pos = strchr(content_type, '\n'); // Cari akhir baris
    if (newline_pos) *newline_pos = '\0';  // Potong di newline

    return content_type;  // Return hasil (jangan lupa free() setelah digunakan)
}

char *handle_method(int *response_size, RequestHeader req_header) {
    char *response = NULL;  // Single pointer untuk response

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
    snprintf(fileURL, sizeof(fileURL), "%s%s%s", config.document_root, req_header.directory, req_header.uri);
    //printf("URI : %s\n", fileURL);
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
        const char *extension = strrchr(req_header.uri, '.');
        
        if (extension && strcmp(extension, ".php") == 0) {
            ResponseHeader res_header;
            int php_has_error = 0;  // Flag untuk menandakan ada error
            Response_PHP_FPM php_fpm = php_fpm_request(
                req_header.directory,
                req_header.uri, 
                req_header.method, 
                req_header.query_string,
                req_header.path_info, 
                req_header.body_data, 
                req_header.content_type);

            char *_php_header = php_fpm.header ? strdup(php_fpm.header) : strdup(""); 
            char *_php_body = php_fpm.body ? strdup(php_fpm.body) : strdup(""); 

            if (strstr(_php_header, "Status: 500 Internal Server Error")) {
                php_has_error = 1;
                res_header.http_version = req_header.http_version,
                res_header.status_code = 500,
                res_header.status_message = "Internal Server Error",
                res_header.mime_type = "text/html",
                res_header.content_length = strlen(_php_body);
            } else {
                //Content-Type: application/json
                res_header.http_version = req_header.http_version,
                res_header.status_code = 200,
                res_header.status_message = "OK",
                res_header.mime_type = get_content_type(_php_header),
                res_header.content_length = strlen(_php_body);
            }

            response = create_response(response_size, &res_header, _php_body , strlen(_php_body));
            free(_php_body);

            // Bebaskan `php_fpm.body` untuk mencegah memory leak
            free(php_fpm.header);
            free(php_fpm.body);
            return response;
        }
        else {
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
    }
} // handle_method
