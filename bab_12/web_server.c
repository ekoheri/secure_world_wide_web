#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "http.h"

#define PORT 8080
#define ip_address "127.0.0.1"

#define MAX_EVENTS 64

int sock_server;
struct sockaddr_in serv_addr;
int addrlen = 0;

int epoll_fd;
struct epoll_event event, events[MAX_EVENTS];

volatile sig_atomic_t keep_running = 1; // Variabel untuk menghentikan daemon

void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void start_server() {
    int opt = 1;
    addrlen = sizeof(serv_addr);

    // 1. Inisialisasi socket server
    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Inisialisasi socket server gagal");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(sock_server);

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
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    serv_addr.sin_port = htons(PORT);

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

     // Create epoll instance
    epoll_fd = epoll_create1(0);
    event.data.fd = sock_server;
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_server, &event);

    printf("Server sedang berjalan.");
    printf("Tekan Ctrl+C untuk menghentikan.\n");
    printf("Akses URL  http://%s:%d\n", ip_address, PORT);
}

void handle_client(int sock_client) {
    char *request;
    char *response = NULL;
    int request_size;
    int response_size = 0;
    char method[16], uri[256], query_string[512], 
        post_data[512], http_version[16];

    request = (char *)malloc(BUFFER_SIZE * sizeof(char));
    if (!request) {
        perror("Gagal mengalokasikan memory untuk request");
        close(sock_client);
        return;
    }

    request_size = read(sock_client, request, BUFFER_SIZE - 1);
    if (request_size < 0) {
        perror("Proses baca request dari client gagal");
        close(sock_client);
        free(request);
        return;
    }

    // Tambahkan null terminator pada akhir request
    request[request_size] = '\0';
    RequestHeader req_header = parse_request_line(request);
    free(request);

    response = handle_method(&response_size, req_header);

    if (response != NULL) {
        if (send(sock_client, response, response_size, 0) < 0) {
            perror("Proses kirim data ke client gagal");
        }
        free(response);
    } else {
        perror("Response data is NULL\n");
    }

    close(sock_client);
}

void run_server() {
    if (sock_server > 0) {
        while (keep_running) {
            int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            for (int i = 0; i < num_fds; i++) {
                if (events[i].data.fd == sock_server) {
                    // Accept new connection
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
                    
                    set_nonblocking(sock_client);

                    // Add the new client socket to epoll
                    event.data.fd = sock_client;
                    event.events = EPOLLIN;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_client, &event);
                } else {
                    // Handle client request
                    handle_client(events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                }
            }
        }
    }
}

void stop_server(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    close(sock_server);
    close(epoll_fd);
    keep_running = 0;
    printf("\nServer telah dihentikan.\n");

    exit(0);
  }
}

void set_daemon() {
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE); // Fork gagal
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Proses parent keluar
    }

    // Set umask
    umask(0);

    // Buat sesi baru
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Ganti direktori kerja
    chdir("/");

    // Tutup file deskriptor standar
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

int main() {
    // Jika dihentikan dgn perintah kill
    //signal(SIGTERM, stop_server);
    // Jika ditekan Ctrl+c maka server dihentikan
    signal(SIGINT, stop_server);

    //set_daemon();

    // Server dijalankan
    start_server();
    run_server();
    return 0;
}

//compile : gcc web_server.c http.c enkripsi/chacha20.c keygen/keygen.c hash/sha256.c -o web_server
