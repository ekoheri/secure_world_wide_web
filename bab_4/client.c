#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock_client, response;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE] = {0};

    // 1. Inisialisasi socket client
    if (argc < 2) {
        fprintf(stderr,"IP address server tidak diisi!\n");
        exit(EXIT_FAILURE);
    }

    // Cek IP Address server apakah aktif atau tidak.
    if ((server = gethostbyname(argv[1])) == NULL) {
        perror("Server tidak aktif");
        exit(EXIT_FAILURE);
    }

    if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Inisialisasi socket client gagal");
        exit(EXIT_FAILURE);
    }

    //2. Connect
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memcpy(&serv_addr.sin_addr.s_addr, 
        server->h_addr, 
        server->h_length);

    if (connect(sock_client, 
        (struct sockaddr *)&serv_addr, 
        sizeof(serv_addr)) < 0) {

        perror("Koneksi ke server gagal");
        exit(EXIT_FAILURE);
    }

    while (1) {
        //3. Send Request ke server
        printf("Client: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(sock_client, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            printf("Mengakhiri chat...\n");
            break;
        }

        //4. Read Response dari server
        memset(buffer, 0, BUFFER_SIZE);
        response = read(sock_client, buffer, BUFFER_SIZE);
        if (response > 0) {
            printf("Server: %s\n", buffer);
        }
    }

    //5. Close Socket Client
    close(sock_client);
    return 0;
}
