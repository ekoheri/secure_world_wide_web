#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#include "http.h"
#include "config.h"
#include "log.h"

extern Config config;

int sock_server;
struct sockaddr_in serv_addr;
int addrlen = 0;

void start_server() {
    int opt = 1;
    addrlen = sizeof(serv_addr);

    // 1. Inisialisasi socket server
    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        write_log("Inisialisasi socket server gagal %d", sock_server);
        exit(EXIT_FAILURE);
    }

    // 2. Mengatur opsi socket
    if (setsockopt(sock_server, 
        SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
        &opt, 
        sizeof(opt))) {

        write_log("setsockopt gagal");
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

        write_log("Proses bind gagal");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    // 4. Listen untuk mendengarkan koneksi masuk
    if (listen(sock_server, 3) < 0) {
        write_log("proses listen gagal");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    write_log("Web Server sedang berjalan");
}

void handle_client(int sock_client) {
    char *request;
    char *response = NULL;
    int request_size;
    int response_size = 0;
    char method[16] = {0}, uri[256] = {0}, http_version[16] = {0};
    int  request_buffer_size = 4096;

    request = (char *)malloc(request_buffer_size * sizeof(char));
    if (!request) {
        write_log("Gagal mengalokasikan memory untuk request");
        close(sock_client);
        return;
    }

    request_size = read(sock_client, request, request_buffer_size - 1);
    if (request_size < 0) {
        write_log("Proses baca request dari client gagal");
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
            write_log("Proses kirim data ke client gagal");
        }
        free(response);
    } else {
        write_log("Response data ke browser NULL");
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
            write_log("Ada koneksi dari %s", client_ip);

            // Panggil fungsi handle_client
            handle_client(sock_client);
        }
    }
}

void stop_server(int signal) {
  if (signal == SIGINT)
  {
    close(sock_server);
    write_log("Web Server telah dihentikan.");

    exit(0);
  }
}

void set_daemon() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // Parent process exit
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid() failed");
        exit(EXIT_FAILURE);
    }

    // Mengubah working directory ke root
    if (chdir("/") < 0) {
        perror("chdir() failed");
        exit(EXIT_FAILURE);
    }

    // Alihkan STDIN, STDOUT, dan STDERR ke /dev/null
    int devnull = open("/dev/null", O_RDWR);
    if (devnull < 0) {
        perror("open /dev/null failed");
        exit(EXIT_FAILURE);
    }
    dup2(devnull, STDIN_FILENO);
    dup2(devnull, STDOUT_FILENO);
    dup2(devnull, STDERR_FILENO);
    // Tutup devnull nilainya tidak 0, 1, atau 2
    if (devnull > 2) close(devnull);
}

int main() {
    // Jika dihentikan dgn perintah kill
    signal(SIGTERM, stop_server);
    // Jika ditekan Ctrl+c maka server dihentikan
    signal(SIGINT, stop_server);

    load_config("server.conf");

    // Buat Web Server menjadi daemon
    set_daemon();
    
    // Buat direktori log
    create_log_directory();
    
    // Server dijalankan
    start_server();
    run_server();
    return 0;
}

// compile : gcc -o web_server web_server.c http.c config.c log.c
// run : ./web_server &

// Kill :
// ps aux | grep web_server | grep -v grep
// kill -9 <Nomor PID>