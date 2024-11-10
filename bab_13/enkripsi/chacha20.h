#ifndef CHACHA20_H
#define CHACHA20_H

#include <stdint.h>
#include <stddef.h>

// Fungsi untuk mengenkripsi plaintext (string) menggunakan ChaCha20
char* chacha20_encrypt(
    const char *plaintext, 
    const uint8_t *key, 
    const uint8_t *nonce, 
    uint32_t counter);

// Fungsi untuk mendekripsi ciphertext (string) menggunakan ChaCha20
char* chacha20_decrypt(
    const char *ciphertext, 
    const uint8_t *key, 
    const uint8_t *nonce, 
    uint32_t counter);

// Fungsi untuk mengonversi string hex ke bytes
void hex_to_bytes(const char* hex, uint8_t* bytes, size_t bytes_len);

// Fungsi untuk mengonversi bytes ke string hex
void bytes_to_hex(const uint8_t* bytes, size_t bytes_len, char* hex);

char *hex_to_utf8(const char *hex_str);

char *utf8_to_hex(const char *utf8_str);

#endif // CHACHA20_H
