#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define PORT 80
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 8192

void handle_url(
    char *url, 
    char *hostname, 
    char *path, 
    int *port) {
    //Program handle_url
    char *http_prefix = "http://";
    if (strncmp(url, http_prefix, strlen(http_prefix)) == 0) {
         url += strlen(http_prefix);  // Menghapus 'http://'
    }
    // Mencari posisi dari ':', '/' dan menentukannya
    char *colon_pos = strchr(url, ':'); // Mencari ':'
    char *slash_pos = strchr(url, '/'); // Mencari '/'

    // Inisialisasi path dan port default
    strcpy(path, "/");
    *port = PORT;

    if (colon_pos != NULL && (
        slash_pos == NULL || colon_pos < slash_pos)
        ) {
        // Jika ':' muncul sebelum '/', ada port
        strncpy(hostname, url, colon_pos - url); // Ambil hostname
        hostname[colon_pos - url] = '\0';        // Null-terminate hostname

        // Ambil port setelah ':'
        *port = atoi(colon_pos + 1);

        // Ambil path jika ada
        if (slash_pos != NULL) {
            strcpy(path, slash_pos);  // Ambil path dari '/'
        } else {
            strcpy(path, "/");  // Jika tidak ada path, set ke '/'
        }
    } else if (slash_pos != NULL) {
        // Jika tidak ada port tapi ada path
        strncpy(hostname, url, slash_pos - url);  // Ambil hostname
        hostname[slash_pos - url] = '\0';         // Null-terminate hostname
        strcpy(path, slash_pos);                  // Ambil path
    } else {
        // Jika tidak ada port dan path
        strcpy(hostname, url);  // Hanya hostname
    }
} //end handle_url

char * client_request(char *url) {
    int sock_client, response;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE] = {0};
    char response_buffer[RESPONSE_SIZE] = {0};
    //char url[BUFFER_SIZE] = {0};
    char hostname[BUFFER_SIZE] = {0};
    char path[BUFFER_SIZE] = {0};
    int port = 0;
    int total_bytes_received = 0;

    // Menangani URL untuk mendapatkan hostname, path, dan port
    handle_url(url, hostname, path, &port);

    // Mendapatkan alamat server berdasarkan hostname
    if ((server = gethostbyname(hostname)) == NULL) {
        fprintf(stderr, "Hostname tidak valid\n");
        exit(EXIT_FAILURE);
    }

    // 1. Inisialisasi socket
    if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Inisialisasi socket client gagal");
        exit(EXIT_FAILURE);
    }

    // Mengisi informasi server (IP Address & port)
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, 
        server->h_addr, 
        server->h_length);

    // 2. Connect ke server
    if (connect(sock_client, 
        (struct sockaddr *)&serv_addr, 
        sizeof(serv_addr)) < 0) {

        perror("Koneksi ke server gagal");
        exit(EXIT_FAILURE);
    }

    // Membuat permintaan HTTP GET
    snprintf(buffer, BUFFER_SIZE, 
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "User-Agent: EkoBrowser/1.0\r\n"
        "\r\n", path, hostname);

    // 3. Send request ke server
    if (send(sock_client, 
            buffer, 
            strlen(buffer), 0) < 0) {

        perror("Gagal mengirim permintaan");
        close(sock_client);
        exit(EXIT_FAILURE);
    }

    // 4. Read Response dari server
    memset(buffer, 0, BUFFER_SIZE);
    while ((response = read(sock_client, buffer, BUFFER_SIZE - 1)
            ) > 0) {
        buffer[response] = '\0';
        
        // Menyimpan respons ke buffer lengkap
        if (total_bytes_received + response < RESPONSE_SIZE - 1) {
            strcat(response_buffer, buffer);
            total_bytes_received += response;
        } else {
            fprintf(stderr, "Buffer terlalu kecil\n");
            break;
        }
        // Reset buffer untuk pembacaan berikutnya
        memset(buffer, 0, BUFFER_SIZE);  
    } //end while

    if (response < 0) {
        perror("Gagal menerima respons");
    }

    close(sock_client);
    return strdup(response_buffer); // Mengembalikan respons
} //end client_request

void parse_tag_html (
    const char *response, 
    const char *tag_open, 
    const char *tag_close, 
    char *tag_name) {
    
    const char *start, *end;

    start = response;
    while ((start = strstr(start, tag_open)) != NULL) {
        start += strlen(tag_open);
        end = strstr(start, tag_close);
        if (end == NULL) break;
        printf("%s: %.*s\n", tag_name, (int)(end - start), start);
        start = end + strlen(tag_close);
    }
}//end parse_tag_html

void display_html() {
    const char *h1_open = "<h1>";
    const char *h1_close = "</h1>";
    const char *p_open = "<p>";
    const char *p_close = "</p>";
    const char *a_open = "<a>";
    const char *a_close = "</a>";

    char url[BUFFER_SIZE] = {0};

    // Meminta pengguna memasukkan URL
    printf("Masukkan URL: ");
    fgets(url, BUFFER_SIZE, stdin);
    url[strcspn(url, "\n")] = 0; 

    // Mengirim permintaan dan mendapatkan respons
    char *response = client_request(url);
    printf("Display HTML:\n");
    parse_tag_html(response, h1_open, h1_close, "H1");
    parse_tag_html(response, p_open, p_close, "P");
    parse_tag_html(response, a_open, a_close, "Hyperlink");
} //end display_html

int main() {
    display_html();
    return 0;
}