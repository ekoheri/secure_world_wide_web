#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define LIBRARY_FOLDER "/home/eko/socket_programming/bab_12/"
#define CIPHERTEXT_FILE "/home/eko/socket_programming/bab_12/ciphertext.txt"

char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(file_size + 1);
    if (content == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(content, 1, file_size, file);
    content[file_size] = '\0';
    fclose(file);
    return content;
}

int write_to_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return -1;
    }
    fprintf(file, "%s", data);
    fclose(file);
    return 0;
}

int main() {
    void *handle;
    char* (*fungsi_encrypt)(const char*, const char*, long);
    char* (*fungsi_decrypt)(const char*, const char*, long);
    char *error;

    char library_path[256];

    // Meminta pengguna memilih jenis enkripsi
    char pilihan[10];
    printf("Pilih jenis enkripsi : ");
    scanf("%9s", pilihan);

    snprintf(library_path, sizeof(library_path), "%s%s", LIBRARY_FOLDER, pilihan);

    handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    fungsi_encrypt = dlsym(handle, "encrypt");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error finding function encrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    fungsi_decrypt = dlsym(handle, "decrypt");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error finding function decrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    const char *key = "1234567890123456789012345678901234567890";
    const char *file = "/home/eko/socket_programming/bab_7/index.html";
  
    const char *plaintext = read_file(file);
    if (!plaintext) {
        dlclose(handle);
        return 1;
    }

    int len = strlen(plaintext);
    printf("Panjang : %d\n", len);

    char *ciphertext = fungsi_encrypt(plaintext, key, len);
    if (!ciphertext) {
        printf("Encryption failed!\n");
        free((void *)plaintext);
        dlclose(handle);
        return 1;
    }

    // printf("Encrypted text (hex): \n%s\n", ciphertext);

    // Simpan ciphertext ke file
    if (write_to_file(CIPHERTEXT_FILE, ciphertext) != 0) {
        printf("Failed to write ciphertext to file!\n");
        free(ciphertext);
        free((void *)plaintext);
        dlclose(handle);
        return 1;
    }
    printf("Ciphertext has been written to %s\n", CIPHERTEXT_FILE);

    free((void *)plaintext);
    free(ciphertext);

    // Baca ciphertext dari file untuk proses dekripsi
    char *file_ciphertext = read_file(CIPHERTEXT_FILE);
    if (!file_ciphertext) {
        dlclose(handle);
        return 1;
    }

    char *decrypted_text = fungsi_decrypt(file_ciphertext, key, len);
    if (!decrypted_text) {
        printf("Decryption failed!\n");
        free(file_ciphertext);
        dlclose(handle);
        return 1;
    }

    printf("Decrypted text: \n%s\n", decrypted_text);

    free(file_ciphertext);
    free(decrypted_text);

    dlclose(handle);
    return 0;
}
