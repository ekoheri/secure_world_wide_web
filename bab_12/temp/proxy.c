#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define PORT 8888
#define BUFFER_SIZE 4096

// Fungsi untuk memproses permintaan dari klien
void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    int server_sock;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char *host;
    char request[BUFFER_SIZE];
    int bytes_read;

    // Baca permintaan dari browser
    bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read < 0) {
        perror("Error reading from socket");
        close(client_sock);
        return;
    }
    buffer[bytes_read] = '\0';

    // Cetak permintaan untuk debugging
    printf("Request from client:\n%s\n", buffer);

    // Cari host dari permintaan GET
    host = strstr(buffer, "Host: ");
    if (host == NULL) {
        perror("No Host found in request");
        close(client_sock);
        return;
    }
    host += 6;  // Lompat ke awal nama host
    char *end_of_host = strstr(host, "\r\n");
    if (end_of_host != NULL) {
        *end_of_host = '\0';
    }

    // Cari alamat IP dari host
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "No such host: %s\n", host);
        close(client_sock);
        return;
    }

    // Buat socket untuk koneksi ke server tujuan
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket");
        close(client_sock);
        return;
    }

    // Atur alamat server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Hubungkan ke server tujuan
    if (connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(server_sock);
        close(client_sock);
        return;
    }

    // Teruskan permintaan dari browser ke server tujuan
    if (send(server_sock, buffer, bytes_read, 0) < 0) {
        perror("Error sending request to server");
        close(server_sock);
        close(client_sock);
        return;
    }

    // Baca respons dari server tujuan dan kirimkan kembali ke browser
    while ((bytes_read = recv(server_sock, buffer, sizeof(buffer), 0)) > 0) {
        if (send(client_sock, buffer, bytes_read, 0) < 0) {
            perror("Error sending response to client");
            close(server_sock);
            close(client_sock);
            return;
        }
    }

    // Tutup koneksi
    close(server_sock);
    close(client_sock);
}

int main() {
    int proxy_sock, client_sock;
    struct sockaddr_in proxy_addr, client_addr;
    socklen_t client_len;

    // Buat socket untuk proxy server
    proxy_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Setel alamat untuk proxy server
    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    proxy_addr.sin_port = htons(PORT);

    // Bind socket ke alamat dan port
    if (bind(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr)) < 0) {
        perror("Error binding socket");
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    // Mulai mendengarkan koneksi
    if (listen(proxy_sock, 5) < 0) {
        perror("Error listening on socket");
        close(proxy_sock);
        exit(EXIT_FAILURE);
    }

    printf("Proxy server listening on port %d...\n", PORT);

    // Loop untuk menerima dan memproses koneksi dari klien
    while (1) {
        client_len = sizeof(client_addr);
        client_sock = accept(proxy_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Error accepting connection");
            continue;
        }

        // Tangani klien di dalam fungsi terpisah
        handle_client(client_sock);
    }

    // Tutup socket proxy (tidak akan pernah terjadi karena loop tak terbatas)
    close(proxy_sock);
    return 0;
}
