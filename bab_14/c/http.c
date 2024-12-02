#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <dlfcn.h>

#include "http.h"
#include "config.h"
#include "encryption.h"

extern Config config;

void handle_url(char *url, char *hostname, char *path, int *port, char *protocol) {
    strcpy(protocol, "http");
    char *http_prefix = "http://";
    char *https_prefix = "https://";

    if (strncmp(url, http_prefix, strlen(http_prefix)) == 0) {
        strcpy(protocol, "HTTP");
        url += strlen(http_prefix);
        *port = 80;
    } else if (strncmp(url, https_prefix, strlen(https_prefix)) == 0) {
        strcpy(protocol, "HTTPS");
        url += strlen(https_prefix);
        *port = 443;
    }

    char *colon_pos = strchr(url, ':');
    char *slash_pos = strchr(url, '/');
    strcpy(path, "/");

    if (colon_pos != NULL && (slash_pos == NULL || colon_pos < slash_pos)) {
        strncpy(hostname, url, colon_pos - url);
        hostname[colon_pos - url] = '\0';
        *port = atoi(colon_pos + 1);
        if (slash_pos != NULL) {
            strcpy(path, slash_pos);
        }
    } else if (slash_pos != NULL) {
        strncpy(hostname, url, slash_pos - url);
        hostname[slash_pos - url] = '\0';
        strcpy(path, slash_pos);
    } else {
        strcpy(hostname, url);
    }
}

ResponseHeaders parse_response_headers(const char *response) {
    ResponseHeaders headers = {NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL}; // Inisialisasi struct dengan NULL

    // Memisahkan setiap baris header
    char *line = strtok(strdup(response), "\r\n"); // Gunakan strdup untuk mengubah string menjadi modifiable
    
    // Memasukkan baris pertama (HTTP version dan status code)
    if (line != NULL) {
        headers.http_version = strdup(line); // Simpan seluruh baris
    }

    // Memproses baris selanjutnya
    while ((line = strtok(NULL, "\r\n")) != NULL) {
        if (strncmp(line, "Content-Type: ", 15) == 0) {
            headers.content_type = strdup(line + 15);
        } else if (strncmp(line, "Content-Length: ", 16) == 0) {
            headers.content_length = atoi(line + 16);
        } else if (strncmp(line, "Connection: ", 12) == 0) {
            headers.connection = strdup(line + 12);
        } else if (strncmp(line, "Cache-Control: ", 16) == 0) {
            headers.cache_control = strdup(line + 16);
        } else if (strncmp(line, "Encrypted: ", 11) == 0) {
            headers.encrypted = strdup(line + 11);
        } else if (strncmp(line, "Response-Time: ", 15) == 0) {
            headers.response_time = strdup(line + 15);
        }
    }

    return headers; // Mengembalikan struct yang berisi header
}

// Fungsi untuk membersihkan memori
void free_response_headers(ResponseHeaders *headers) {
    free(headers->http_version);
    free(headers->content_type);
    free(headers->connection);
    free(headers->cache_control);
    free(headers->encrypted);
    free(headers->response_time);
}

char *handle_respose(char *url, char *form_data) {
    int sock_client, response;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE] = {0};
    char *response_buffer;
    char protocol[5] = {0};
    char hostname[BUFFER_SIZE] = {0};
    char path[BUFFER_SIZE] = {0};
    int port = 0;
    int total_bytes_received = 0;
    long response_buffer_size = 8 * BUFFER_SIZE;
    
    char *header = NULL;
    char *body = NULL;

    handle_url(url, hostname, path, &port, protocol);

    if ((server = gethostbyname(hostname)) == NULL) {
        body = "<h1>Error : Hostname tidak valid!</h1>";
    }

    if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        body = "<h1>Error : Inisialisasi socket client gagal!</h1>";
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock_client, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        body = "<h1>Error : Koneksi ke server gagal!</h1>";
    }

    if(strcmp(form_data, "")==0 || form_data == NULL) {
        snprintf(buffer, BUFFER_SIZE, 
            "GET %s %s/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "User-Agent: EkoBrowser/1.0\r\n"
            "\r\n", path, protocol, hostname);
    } else {
        snprintf(buffer, BUFFER_SIZE, 
            "POST %s %s/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "User-Agent: EkoBrowser/1.0\r\n"
            "\r\n\r\n%s\r\n", 
            path, protocol, hostname, form_data);

        //printf("%s", buffer);
    }
    if (send(sock_client, buffer, strlen(buffer), 0) < 0) {
        close(sock_client);
        body = "<h1>Error : Gagal mengirim permintaan ke server!</h1>";
    }

    response_buffer = (char *)malloc(response_buffer_size);
    if (response_buffer == NULL) {
        fprintf(stderr, "Malloc response buffer gagal\n");
        exit(EXIT_FAILURE);
    }
    // Mengosongkan buffer
    memset(response_buffer, 0, response_buffer_size);

    memset(buffer, 0, BUFFER_SIZE);
    while ((response = read(sock_client, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[response] = '\0';

        if (total_bytes_received + response < response_buffer_size - 1) {
            strcat(response_buffer, buffer);
            total_bytes_received += response;
        } else {
            //Jika buffer terlalu kecil, maka re-alokasi ukuran buffer
            response_buffer_size *= 2;
            response_buffer = realloc(response_buffer, response_buffer_size);
        }
        memset(buffer, 0, BUFFER_SIZE);  
    }

    if (response < 0) {
        body = "<h1>Error : Gagal menerima respon server!</h1>";
    }

    close(sock_client);

    char *separator = strstr(response_buffer, "\r\n\r\n");
    if (separator != NULL) {
        *separator = '\0';
        header = response_buffer;
        body = separator + 4; 
        //printf("%s\n", header);
        //printf("%s\n", body);
        ResponseHeaders headers = parse_response_headers(header);
        //Block program untuk Decrypt Data
        if (headers.encrypted != NULL && strcmp(headers.encrypted, "yes") == 0) {
            const char *key = "1234567890123456789012345678901234567890";
            char *decrypt_body = call_decrypt(body, key, headers.content_length);

            if (decrypt_body != NULL) { 
                strcpy(body, decrypt_body);
                free(decrypt_body);
            } else {
                // Tangani kesalahan dekripsi jika diperlukan
                body = "<h1>Error : Proses Dekrip GAGAL!</h1>";
            }
        }
        free_response_headers(&headers);
    }
    free(response_buffer);
    return strdup(body);
}
