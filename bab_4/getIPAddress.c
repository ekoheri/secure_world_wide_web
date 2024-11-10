#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void get_ip_address(char *ip_buffer, size_t buffer_size) {
    int sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_udp < 0) {
        perror("Socket UDP gagal dibuat");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dummy_addr;
    memset(&dummy_addr, 0, sizeof(dummy_addr));
    dummy_addr.sin_family = AF_INET;
    dummy_addr.sin_port = htons(8080);  // port dummy
    inet_pton(AF_INET, "8.8.8.8", &dummy_addr.sin_addr);  // DNS server Google

    // Melakukan koneksi dummy untuk mendapatkan IP aktif dari antarmuka jaringan
    connect(sock_udp, (struct sockaddr *)&dummy_addr, sizeof(dummy_addr));

    // Mendapatkan IP address dari socket yang terhubung
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    getsockname(sock_udp, (struct sockaddr *)&local_addr, &local_addr_len);

    // Tutup socket UDP
    close(sock_udp);

    // Mengonversi alamat IP ke format string dan menyimpannya di buffer
    inet_ntop(AF_INET, &local_addr.sin_addr, ip_buffer, buffer_size);
}


int main() {
    char ip_str[INET_ADDRSTRLEN];

    // Panggil fungsi untuk mendapatkan IP address aktif
    get_ip_address(ip_str, sizeof(ip_str));

    printf("IP Address aktif: %s\n", ip_str);
    return 0;
}