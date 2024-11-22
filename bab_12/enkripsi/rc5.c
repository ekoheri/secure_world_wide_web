#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define W 32               // Word size in bits
#define R 12               // Number of rounds
#define KEY_SIZE 16        // Key size in bytes
#define T 2 * (R + 1)      // Table size for subkeys
#define P32 0xB7E15163     // Magic constant P for RC5
#define Q32 0x9E3779B9     // Magic constant Q for RC5

uint32_t S[T];             // Subkey array

// Fungsi untuk memutar ke kiri
#define ROTL(x, y) (((x) << (y)) | ((x) >> (W - (y))))
// Fungsi untuk memutar ke kanan
#define ROTR(x, y) (((x) >> (y)) | ((x) << (W - (y))))

// Fungsi untuk melakukan inisialisasi subkey berdasarkan kunci
void rc5_key_setup(const uint8_t *key) {
    uint32_t L[KEY_SIZE / 4];
    for (int i = 0; i < KEY_SIZE / 4; i++) {
        L[i] = 0;
        for (int j = 0; j < 4; j++) {
            L[i] |= (uint32_t)key[i * 4 + j] << (8 * j);
        }
    }

    S[0] = P32;
    for (int i = 1; i < T; i++) {
        S[i] = S[i - 1] + Q32;
    }

    uint32_t i = 0, j = 0, A = 0, B_local = 0;
    for (int k = 0; k < 3 * T; k++) {
        A = S[i] = ROTL(S[i] + A + B_local, 3);
        B_local = L[j] = ROTL(L[j] + A + B_local, (A + B_local) % W);
        i = (i + 1) % T;
        j = (j + 1) % (KEY_SIZE / 4);
    }
}

// Fungsi enkripsi RC5
void rc5_encrypt(uint32_t *pt) {
    uint32_t A = pt[0] + S[0];
    uint32_t B_local = pt[1] + S[1];
    for (int i = 1; i <= R; i++) {
        A = ROTL(A ^ B_local, B_local % W) + S[2 * i];
        B_local = ROTL(B_local ^ A, A % W) + S[2 * i + 1];
    }
    pt[0] = A;
    pt[1] = B_local;
}

// Fungsi dekripsi RC5
void rc5_decrypt(uint32_t *ct) {
    uint32_t B_local = ct[1];
    uint32_t A = ct[0];
    for (int i = R; i > 0; i--) {
        B_local = ROTR(B_local - S[2 * i + 1], A % W) ^ A;
        A = ROTR(A - S[2 * i], B_local % W) ^ B_local;
    }
    ct[1] = B_local - S[1];
    ct[0] = A - S[0];
}

// Fungsi untuk mengonversi byte array ke heksadesimal
char *bytes_to_hex(const uint8_t *bytes, int length) {
    char *hex = (char *)malloc(length * 2 + 1);
    for (int i = 0; i < length; i++) {
        sprintf(hex + i * 2, "%02X", bytes[i]);
    }
    hex[length * 2] = '\0';
    return hex;
}

// Fungsi untuk mengonversi heksadesimal ke byte array
uint8_t *hex_to_bytes(const char *hex, int *out_length) {
    *out_length = strlen(hex) / 2;
    uint8_t *bytes = (uint8_t *)malloc(*out_length);
    for (int i = 0; i < *out_length; i++) {
        sscanf(hex + i * 2, "%2hhx", &bytes[i]);
    }
    return bytes;
}

// Fungsi enkripsi RC5 untuk plaintext input
char *encrypt(const char *plaintext, const char *key, long length) {
    rc5_key_setup((const uint8_t *)key);

    long round_length = ((length  + 7) / 8) * 8;
    uint8_t *input = (uint8_t *)malloc(round_length);
    memcpy(input, plaintext, round_length);

    // Proses enkripsi blok
    for (int i = 0; i < round_length; i += 8) {
        rc5_encrypt((uint32_t *)(input + i));
    }

    // Konversi hasil enkripsi ke heksadesimal
    char *hex_result = bytes_to_hex(input, round_length);
    free(input);
    return hex_result;
}

// Fungsi dekripsi RC5 untuk ciphertext input dalam heksadesimal
char *decrypt(const char *hex_ciphertext, const char *key, long length) {
    int byte_length;
    uint8_t *ciphertext = hex_to_bytes(hex_ciphertext, &byte_length);
    
    rc5_key_setup((const uint8_t *)key);

    // Proses dekripsi blok
    for (int i = 0; i < byte_length; i += 8) {
        rc5_decrypt((uint32_t *)(ciphertext + i));
    }

    // Hasil dekripsi diubah menjadi string
    char *plaintext = (char *)malloc(byte_length + 1);
    memcpy(plaintext, ciphertext, byte_length);
    plaintext[length] = '\0';

    free(ciphertext);
    return plaintext;
}

/*int main() {
    char plaintext[] = "Hello, world!";
    char key[KEY_SIZE] = "thisisasecretkey"; // 16-byte key

    int length = strlen(plaintext);
    char *encrypted_hex = rc5_encrypt_text(plaintext, key, length);
    printf("Encrypted (hex): %s\n", encrypted_hex);

    char *decrypted = rc5_decrypt_text(encrypted_hex, key, length);
    printf("Decrypted: %s\n", decrypted);

    free(encrypted_hex);
    free(decrypted);
    return 0;
}
*/