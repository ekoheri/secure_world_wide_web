#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "http.h"
#include "config.h"

extern Config config;

int sock_server;
struct sockaddr_in serv_addr;
int addrlen = 0;

void start_server() {
    int opt = 1;
    addrlen = sizeof(serv_addr);

    // 1. Inisialisasi socket server
    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Inisialisasi socket server gagal");
        exit(EXIT_FAILURE);
    }

    // 2. Mengatur opsi socket
    if (setsockopt(sock_server, 
        SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
        &opt, 
        sizeof(opt))) {

        perror("setsockopt gagal");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    // 3. Bind IP Address dengan port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(config.server_name);
    serv_addr.sin_port = htons(config.server_port);

    if (bind(sock_server, 
        (struct sockaddr *)&serv_addr, 
        sizeof(serv_addr)) < 0) {

        perror("proses bind gagal");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    // 4. Listen untuk mendengarkan koneksi masuk
    if (listen(sock_server, 3) < 0) {
        perror("proses listen gagal");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    printf("Server sedang berjalan.");
    printf("Tekan Ctrl+C untuk menghentikan.\n");
    printf("Akses URL  http://%s:%d\n", config.server_name, config.server_port);
}

void handle_client(int sock_client) {
    char *request;
    char *response = NULL;
    int request_size;
    int response_size = 0;
    char method[16] = {0}, uri[256] = {0}, http_version[16] = {0};

    request = (char *)malloc(config.request_buffer_size * sizeof(char));
    if (!request) {
        perror("Gagal mengalokasikan memory untuk request");
        close(sock_client);
        return;
    }

    request_size = read(sock_client, request, config.request_buffer_size - 1);
    if (request_size < 0) {
        perror("Proses baca request dari client gagal");
        close(sock_client);
        free(request);
        return;
    }

     // Tambahkan null terminator pada akhir request
    request[request_size] = '\0';
    RequestHeader req_header = parse_request_line(request);
    //printf("%s\n", request);
    free(request);

    response = handle_method(&response_size, req_header);

    if (response != NULL) {
        if (send(sock_client, response, response_size, 0) < 0) {
            perror("Proses kirim data ke client gagal");
        }
        free(response);
    } else {
        printf("Response data is NULL\n");
    }

    close(sock_client);
}

void run_server() {
    if (sock_server > 0) {
        while (1) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int sock_client = accept(
                sock_server, 
                (struct sockaddr *)&client_addr, &addr_len
            );
            
            if (sock_client < 0) {
                perror("Proses accept gagal");
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), 
                client_ip, INET_ADDRSTRLEN);
            printf("Proses accept dari %s berhasil\n", client_ip);

            // Panggil fungsi handle_client
            handle_client(sock_client);
        }
    }
}

void stop_server(int signal) {
  if (signal == SIGINT)
  {
    close(sock_server);
    printf("\nServer telah dihentikan.\n");

    exit(0);
  }
}

int main() {
    // Jika ditekan Ctrl+c maka server dihentikan
    signal(SIGINT, stop_server);

    load_config("server.conf");
    
    // Server dijalankan
    start_server();
    run_server();
    return 0;
}