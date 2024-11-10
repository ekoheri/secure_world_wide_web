#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024

const char* get_parameter(
    int argc, 
    char *argv[], 
    const char *arg_name) {
    
    size_t panjang_arg_name = strlen(arg_name);
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], arg_name, panjang_arg_name) == 0) {
            return argv[i] + panjang_arg_name;
        }
    }
    return NULL;
}

// Fungsi untuk menjalankan skrip PHP 
// dengan memasukkan variabel GET atau POST
char *run_php_script(
    const char *target, 
    const char *query_string, 
    const char *post_data) {

    char command[MAX_BUFFER];
    FILE *fp;

    // Menjalankan skrip PHP dengan GET dan POST
    snprintf(command, sizeof(command),
             "php -r 'parse_str(\"%s\", $_GET); "
             "parse_str(\"%s\", $_POST); "
             "include \"%s\";'",
             query_string, post_data, target);

    printf("<p>Eksekusi perintah : %s</p>\n", command);

    // Buffer untuk menyimpan seluruh respons
    char *response = (char *)malloc(RESPONSE_BUFFER);
    if (response == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    response[0] = '\0';  // Inisialisasi string kosong

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        free(response);
        exit(EXIT_FAILURE);
    }

    char result[MAX_BUFFER];
    int has_error = 0;  // Flag untuk menandakan ada error

    // Membaca output dari PHP dan menyusunnya ke dalam response buffer
    while (fgets(result, sizeof(result), fp) != NULL) {
        // Cek apakah ada pesan kesalahan dari PHP
        if (strstr(result, "Warning:") || strstr(result, "Notice:") || strstr(result, "Fatal error:")) {
            fprintf(stderr, "PHP Warning/Notice/Error: %s", result);
            has_error = 1;  // Tandai bahwa ada error
        } else {
            // Tambahkan hasil ke response buffer
            strncat(response, result, RESPONSE_BUFFER - strlen(response) - 1);
        }
    }

    if (pclose(fp) == -1) {
        perror("pclose");
        free(response);
        exit(EXIT_FAILURE);
    }

    // Jika ada error, tambahkan pesan error ke response
    if (has_error) {
        snprintf(response + strlen(response), RESPONSE_BUFFER - strlen(response) - 1,
                 "<p>Terjadi kesalahan dalam menjalankan skrip PHP.</p>\n");
    }

    return response;  // Mengembalikan respons lengkap
}

int main(int argc, char *argv[]) {
    char query_string[MAX_BUFFER] = {0};
    char post_data[MAX_BUFFER] = {0};
    const char *target = NULL;
    const char *method = NULL;
    const char *query_string_data = NULL;
    const char *post_data_data = NULL;

    target = get_parameter(argc, argv, "--target=");
    method = get_parameter(argc, argv, "--method=");
    query_string_data = 
        get_parameter(argc, argv, "--data_query_string=");
    post_data_data = get_parameter(argc, argv, "--data_post=");

    if (target != NULL && method != NULL) {
        if (query_string_data != NULL) {
            snprintf(query_string, 
                sizeof(query_string),"%s",query_string_data);
        }
        if (post_data_data != NULL) {
            snprintf(post_data, 
                sizeof(post_data),"%s",post_data_data);
        }

        if (strcmp(method, "POST") == 0) {
            run_php_script(target, "", post_data);
        } else if (strcmp(method, "GET") == 0) {
            run_php_script(target, query_string, "");
        } else if (strcmp(method, "BOTH") == 0) {
            run_php_script(target, query_string, post_data);
        } else {
            printf("<p>Tidak mendukung metode request: %s</p>\n", 
                method);
        }
    } else {
        printf("<p>Target atau method tidak spesifik</p>\n");
    }

    return 0;
}
