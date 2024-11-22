// main.c
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define LIBRARY_FOLDER "/home/eko/socket_programming/bab_12/"

char* read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Menentukan ukuran file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Mengalokasikan memori untuk menyimpan isi file
    char *content = (char *)malloc(file_size + 1);
    if (content == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Membaca isi file ke dalam buffer
    fread(content, 1, file_size, file);
    content[file_size] = '\0';  // Menambahkan null terminator

    fclose(file);
    return content;
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

    // Tentukan library yang akan dimuat berdasarkan pilihan
    snprintf(library_path, sizeof(library_path), "%s%s", LIBRARY_FOLDER, pilihan);

    // Memuat shared library sesuai pilihan
    handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Mendapatkan alamat fungsi encrypt dari library
    fungsi_encrypt = dlsym(handle, "encrypt");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error finding function encrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Mendapatkan alamat fungsi decrypt dari library
    fungsi_decrypt = dlsym(handle, "decrypt");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error finding function decrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Data yang akan dienkripsi
    const char *key = "1234567890123456789012345678901234567890";  // Kunci 40 byte
    const char *file = "/home/eko/socket_programming/bab_12/ciphertext.txt";
  
    const char *ciphertext = read_file(file);

    long len = 3166;
    // Dekripsi
    char *decrypted_text = fungsi_decrypt(ciphertext, key, len);
    if (!decrypted_text) {
        printf("Decryption failed!\n");
        dlclose(handle);
        return 1;
    }

    printf("Decrypted text: \n%s\n", decrypted_text);

    // Membersihkan memori
    free(decrypted_text);

    // Menutup handle library
    dlclose(handle);
    return 0;
}

// gcc test_enkripsi.c -o test_enkripsi -ldl
