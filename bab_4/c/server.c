#include <stdio.h>      // library console I/O, perror
#include <stdlib.h>     // library exit
#include <string.h>     // library fungsi string
#include <unistd.h>     // library open, close
#include <sys/socket.h> // library socket APIs
#include <arpa/inet.h>  // library sockaddr_in

#define PORT 8080
#define ip_address "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    //Program Socket Server
    //Definisi variable
    int sock_server, sock_client, request, opt = 1;
    struct sockaddr_in serv_addr;
    int addrlen = sizeof(serv_addr);
    char buffer[BUFFER_SIZE] = {0};

    //1. Inisialisasi socket
    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Inisialisasi socket server gagal");
        exit(EXIT_FAILURE);
    }

    //2. Set Socket option
    if (setsockopt(sock_server, 
        SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
        &opt, sizeof(opt))) {
        
        perror("setsockopt gagal");
        exit(EXIT_FAILURE);
    }

    //3. Bind IP Address & Port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    serv_addr.sin_port = htons(PORT);
    if (bind(sock_server, (struct sockaddr *)&serv_addr, 
        sizeof(serv_addr)) < 0) {

        perror("proses bind gagal");
        exit(EXIT_FAILURE);
    }

    //4. Listen
    if (listen(sock_server, 3) < 0) {
        perror("proses listen gagal");
        exit(EXIT_FAILURE);
    }

    printf("Server siap IP : %s Port : %d\n", ip_address, PORT);

    //5. Accept
    if ((sock_client = accept(sock_server, 
        (struct sockaddr *)&serv_addr, 
        (socklen_t *)&addrlen)) < 0) {

        perror("proses accept gagal");
        exit(EXIT_FAILURE);
    }

    while (1) {
        //6. Read Request
        memset(buffer, 0, BUFFER_SIZE);
        request = read(sock_client, buffer, BUFFER_SIZE);
        // menampilkan data yang dikirim ke client
        if (request > 0) {
            printf("Client: %s\n", buffer);
        }
        // menyiapkan ketikkan yang akan dikirim ke client
        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; 

        if (strcmp(buffer, "exit") == 0) {
            printf("Mengakiri chat...\n");
            break;
        }

        //7. Send Response
        send(sock_client, buffer, strlen(buffer), 0);
    }
    //8. Close Socket Client 
    close(sock_client);
    //9. Close Socket Server
    close(sock_server);
    return 0;
}

