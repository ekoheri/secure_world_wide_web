#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define P_SIZE 1024
#define Q_SIZE 1024

// Deklarasi array untuk P dan Q
uint32_t P[P_SIZE];
uint32_t Q[Q_SIZE];

// Fungsi untuk menginisialisasi array P dan Q
void hc256_init(uint8_t *key) {
    // Inisialisasi array P dan Q menggunakan key
    // Ini hanya contoh inisialisasi dan bisa disesuaikan dengan algoritma HC-256
    for (int i = 0; i < P_SIZE; i++) {
        P[i] = key[i % 32]; // Menggunakan key untuk mengisi P
    }
    for (int i = 0; i < Q_SIZE; i++) {
        Q[i] = key[i % 32]; // Menggunakan key untuk mengisi Q
    }
}

// Fungsi untuk menghitung keystream pada HC-256
uint32_t hc256_generate_keystream(uint32_t *counter) {
    uint32_t i = *counter % (P_SIZE + Q_SIZE); // Menghitung posisi keystream
    
    uint32_t keystream = 0;
    if (*counter < P_SIZE) {
        // Operasi pada array P
        keystream = P[i];
    } else {
        // Operasi pada array Q
        keystream = Q[i - P_SIZE];
    }

    // Increment counter dan pastikan tetap dalam batas yang valid
    *counter = (*counter + 1) % (P_SIZE + Q_SIZE);
    
    return keystream;
}

// Fungsi enkripsi
char *encrypt(const char *plaintext, const char *key, long length) {
    uint8_t key_padded[32] = {0};  
    strncpy((char *)key_padded, key, 32);
    hc256_init(key_padded);  // Inisialisasi HC-256 dengan kunci

    uint32_t counter = 0;  // Counter untuk keystream
    uint8_t *encrypted_bytes = (uint8_t *)malloc(length);
    
    if (!encrypted_bytes) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        uint8_t keystream_byte = hc256_generate_keystream(&counter) & 0xFF;
        encrypted_bytes[i] = plaintext[i] ^ keystream_byte;
    }

    // Mengonversi hasil enkripsi ke bentuk hex
    char *output = (char *)malloc(2 * length + 1);
    if (!output) {
        free(encrypted_bytes);
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", encrypted_bytes[i]);
    }

    free(encrypted_bytes);
    return output;
}

// Fungsi untuk mendekripsi data
char *decrypt(const char *ciphertext, const char *key, long length) {
    uint8_t key_padded[32] = {0};
    strncpy((char *)key_padded, key, 32);
    hc256_init(key_padded);  // Inisialisasi HC-256 dengan kunci

    uint32_t counter = 0;  // Counter untuk keystream

    uint8_t *encrypted_bytes = (uint8_t *)malloc(length);
    if (!encrypted_bytes) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Mengonversi ciphertext dari hex ke bytes
    for (int i = 0; i < length; i++) {
        sscanf(ciphertext + (i * 2), "%02hhx", &encrypted_bytes[i]);
    }

    char *output = (char *)malloc(length + 1);
    if (!output) {
        free(encrypted_bytes);
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        uint8_t keystream_byte = hc256_generate_keystream(&counter) & 0xFF;
        output[i] = encrypted_bytes[i] ^ keystream_byte;
    }

    output[length] = '\0';
    free(encrypted_bytes);
    return output;
}