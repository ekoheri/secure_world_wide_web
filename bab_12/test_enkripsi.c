#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "enkripsi/chacha20.h"
#include "hash/sha256.h"
#include "keygen/otp.h"

int main() {
    char *otp = generate_otp();
    printf("OTP : %s\n", otp);

    SHA256_CTX ctx;
    uint8_t hash[SHA256_BLOCK_SIZE];
    char *key_hex;
    
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)otp, strlen(otp));
    sha256_final(&ctx, hash);

    key_hex = sha256_to_hex(hash);
    if (key_hex != NULL) {
        printf("SHA-256 hash: %s\n", key_hex);
    } else {
        fprintf(stderr, "Failed to convert hash to hex\n");
    }

    char nonce_hex[17];
    char plaintext[256];

    uint8_t key[32];
    uint8_t nonce[8];

    uint32_t counter = 1;

    hex_to_bytes(key_hex, key, 32);
    memcpy(key_hex, nonce_hex, 16);
    nonce_hex[16] = '\0';
    hex_to_bytes(nonce_hex, nonce, 8);

    printf("Masukkan plaintext: ");
    fgets(plaintext, sizeof(plaintext), stdin);
    plaintext[strcspn(plaintext, "\n")] = 0;  // Hapus newline
    uint32_t plaintext_len = strlen(plaintext);

    // Enkripsi
    char *ciphertext_hex = chacha20_encrypt(plaintext, key_hex, nonce_hex, 1);
    if (ciphertext_hex == NULL) {
        fprintf(stderr, "Gagal melakukan enkripsi\n");
        return 1;
    }

    printf("Ciphertext (hex): %s\n", ciphertext_hex);

    // Konversi ciphertext hex ke ASCII
    char *ciphertext_utf8 = hex_to_utf8(ciphertext_hex);
    if (ciphertext_utf8 == NULL) {
        free(ciphertext_hex);
        return 1;
    }

    printf("Ciphertext (UTF-8): %s\n", ciphertext_utf8);

    // Konversi kembali dari ASCII ke Hex
    char *reconverted_hex = utf8_to_hex(ciphertext_utf8);
    if (reconverted_hex == NULL) {
        free(ciphertext_hex);
        free(ciphertext_utf8);
        return 1;
    }

    printf("Reconverted Hex from UTF-8: %s\n", reconverted_hex);

    // Dekripsi kembali
    char *decrypted_text = chacha20_decrypt(reconverted_hex, key_hex, nonce_hex, 1);
    if (decrypted_text == NULL) {
        fprintf(stderr, "Gagal melakukan dekripsi\n");
        free(ciphertext_hex);
        free(ciphertext_utf8);
        free(reconverted_hex);
        return 1;
    }

    printf("Decrypted text: %s\n", decrypted_text);

    // Bersihkan memori yang telah dialokasikan
    free(otp);
    free(key_hex);
    free(ciphertext_hex);
    free(ciphertext_utf8);
    free(reconverted_hex);
    free(decrypted_text);

    return 0;
}

//compile : gcc -o test_enkripsi test_enkripsi.c enkripsi/chacha20.c hash/sha256.c keygen/otp.c