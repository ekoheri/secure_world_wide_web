#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "encryption.h"
#include "config.h"

char *call_encrypt (const char *plaintext, const char *key, long length) {
    void *handle;
    char * (*encrypt_function)(const char*, const char*, long);
    char *error;

    char library_path[256];

    snprintf(library_path, sizeof(library_path), "%s%s", config.encrypt_directory, config.encryption_type);

    handle = dlopen(library_path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "Error tidak dapat membuka library enkripsi: %s\n", error);
        exit(EXIT_FAILURE);
    }

    dlerror();

    encrypt_function = (char *(*)(const char*, const char*, long)) dlsym(handle, "encrypt");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error tidak dapat menemukan fungsi encrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    char *ciphertext = encrypt_function(plaintext, key, length);
    if (ciphertext == NULL) {
        fprintf(stderr, "Error pada proses dekripsi\n");
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    dlclose(handle);

    return ciphertext;
}

char *call_decrypt(const char *ciphertext, const char *key, long length) {
    void *handle;
    char * (*decrypt_function)(const char*, const char*, long);
    char *error;
    char *plaintext;
    char library_path[256];

    snprintf(library_path, sizeof(library_path), "%s%s", config.encrypt_directory, config.encryption_type);

    //printf("Library :%s\n", library_path);
    handle = dlopen(library_path,  RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "Error tidak dapat membuka library enkripsi: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror(); // Kosongkan error sebelumnya
    // Ambil simbol untuk fungsi decrypt dengan casting void**
    decrypt_function = (char *(*)(const char*, const char*, long)) dlsym(handle, "decrypt");
    //printf("Address of decrypt_function: %p\n", (void *)decrypt_function);

    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "Error tidak dapat menemukan fungsi decrypt: %s\n", error);
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Panggil fungsi decrypt
    plaintext = decrypt_function(ciphertext, key, length);
    if (plaintext == NULL) {
        fprintf(stderr, "Error pada proses dekripsi\n");
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Menutup handle sebelum return
    dlclose(handle);

    return plaintext;
}