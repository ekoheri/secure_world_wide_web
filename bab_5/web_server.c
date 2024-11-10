#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define ip_address "127.0.0.1"
#define BUFFER_SIZE 1024

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

    printf("Server sedang berjalan.");
    printf("Tekan Ctrl+C untuk menghentikan.\n");
    printf("Akses URL  http://%s:%d\n", "127.0.0.1", PORT);
} //end start_server

void handle_client(int sock_client) {
    char *request;
    int request_size;

    // 1. Read Request : Membaca permintaan dari browser
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

    // Menampilkan permintaan dari browser
    printf("Request dari browser : \n%s\n", request);

    free(request);

    // 2. Send response ke browser
    // Membuat respons HTTP sederhana
    const char *response_header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";

    const char *response_body =
        "<!DOCTYPE html>"
        "<html>"
        "<head><title>Hello World</title></head>"
        "<body>"
        "<h1>Hello, World!</h1>"
        "<p>Ini C</p>"
        "</body>"
        "</html>";

    // Buffer untuk menyimpan gabungan header dan body
    char response[BUFFER_SIZE * 2]; 
    // Menggabungkan response_header dan response_body
    snprintf(response, sizeof(response), 
        "%s%s", response_header, response_body);

    // Mengirim respons ke browser
    if (send(sock_client, response, strlen(response), 0) < 0) {
        perror("Proses kirim data ke client gagal");
    }

    // 3. Menutup koneksi socket client
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

    // Server dijalankan
    start_server();
    run_server();
    return 0;
}
