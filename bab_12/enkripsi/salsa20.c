#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Fungsi rotasi kiri (rotate left)
#define ROTL32(x, y) (((x) << (y)) | ((x) >> (32 - (y))))

// Konstanta Salsa20 (pecah menjadi blok 4x4 untuk mempermudah proses)
void salsa20_block(uint32_t *state) {
    uint32_t x[16];
    memcpy(x, state, 16 * sizeof(uint32_t));

    for (int i = 0; i < 20; i += 2) {
        // Odd round
        x[ 4] ^= ROTL32(x[ 0] + x[12], 7);
        x[ 8] ^= ROTL32(x[ 4] + x[ 0], 9);
        x[12] ^= ROTL32(x[ 8] + x[ 4], 13);
        x[ 0] ^= ROTL32(x[12] + x[ 8], 18);

        x[ 9] ^= ROTL32(x[ 5] + x[ 1], 7);
        x[13] ^= ROTL32(x[ 9] + x[ 5], 9);
        x[ 1] ^= ROTL32(x[13] + x[ 9], 13);
        x[ 5] ^= ROTL32(x[ 1] + x[13], 18);

        x[14] ^= ROTL32(x[10] + x[ 6], 7);
        x[ 2] ^= ROTL32(x[14] + x[10], 9);
        x[ 6] ^= ROTL32(x[ 2] + x[14], 13);
        x[10] ^= ROTL32(x[ 6] + x[ 2], 18);

        x[ 3] ^= ROTL32(x[15] + x[11], 7);
        x[ 7] ^= ROTL32(x[ 3] + x[15], 9);
        x[11] ^= ROTL32(x[ 7] + x[ 3], 13);
        x[15] ^= ROTL32(x[11] + x[ 7], 18);

        // Even round
        x[ 4] ^= ROTL32(x[ 0] + x[12], 7);
        x[ 8] ^= ROTL32(x[ 4] + x[ 0], 9);
        x[12] ^= ROTL32(x[ 8] + x[ 4], 13);
        x[ 0] ^= ROTL32(x[12] + x[ 8], 18);

        x[ 9] ^= ROTL32(x[ 5] + x[ 1], 7);
        x[13] ^= ROTL32(x[ 9] + x[ 5], 9);
        x[ 1] ^= ROTL32(x[13] + x[ 9], 13);
        x[ 5] ^= ROTL32(x[ 1] + x[13], 18);

        x[14] ^= ROTL32(x[10] + x[ 6], 7);
        x[ 2] ^= ROTL32(x[14] + x[10], 9);
        x[ 6] ^= ROTL32(x[ 2] + x[14], 13);
        x[10] ^= ROTL32(x[ 6] + x[ 2], 18);

        x[ 3] ^= ROTL32(x[15] + x[11], 7);
        x[ 7] ^= ROTL32(x[ 3] + x[15], 9);
        x[11] ^= ROTL32(x[ 7] + x[ 3], 13);
        x[15] ^= ROTL32(x[11] + x[ 7], 18);
    }

    for (int i = 0; i < 16; i++) {
        state[i] += x[i];
    }
}

// Fungsi untuk mengonversi ke heksadesimal
void to_hex(char *buffer, long length, char *output) {
    for (long i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", (unsigned char)buffer[i]);
    }
    output[length * 2] = '\0';  // Menambahkan null terminator
}

// Fungsi untuk mengonversi heksadesimal kembali ke karakter
void hex_to_char(const char *hex, char *output, long length) {
    for (long i = 0; i < length; i++) {
        sscanf(hex + i * 2, "%2hhx", &output[i]);
    }
}

// Fungsi enkripsi Salsa20
char *encrypt(char *plaintext, char *key, long length) {
    uint32_t state[16];
    uint32_t keystream[16];
    int i;
    
    // Menyiapkan state
    state[0] = 0x61707865;
    state[1] = 0x3320646e;
    state[2] = 0x79622d36;
    state[3] = 0x3f0f6d7a;

    // Key
    for (i = 0; i < 8; i++) {
        state[4 + i] = ((uint32_t *)key)[i];
    }

    // Block counter
    state[12] = 0;
    state[13] = 0;
    state[14] = 0;
    state[15] = 0;

    // Menyiapkan output ciphertext
    char *ciphertext = (char *)malloc(length);
    
    for (long j = 0; j < length; j += 64) {
        // Salin state ke keystream
        memcpy(keystream, state, sizeof(state));
        
        // Enkripsi dengan Salsa20
        salsa20_block(keystream);

        // XOR dengan plaintext untuk menghasilkan ciphertext
        for (i = 0; i < 64 && (j + i) < length; i++) {
            ciphertext[j + i] = plaintext[j + i] ^ ((char *)keystream)[i];
        }

        // Update counter
        state[8]++;
        if (state[8] == 0) {
            state[9]++;
        }
    }

    // Menyimpan ciphertext dalam bentuk hexadecimal
    char *hex_output = (char *)malloc(length * 2 + 1);
    to_hex(ciphertext, length, hex_output);

    // Bebaskan memori ciphertext setelah konversi ke hex
    free(ciphertext);

    return hex_output;
}

// Fungsi dekripsi Salsa20
char *decrypt(const char *hex_ciphertext, char *key, long length) {
    // Mengonversi ciphertext dari hex ke char
    char *ciphertext = (char *)malloc(length);
    hex_to_char(hex_ciphertext, ciphertext, length);

    uint32_t state[16];
    uint32_t keystream[16];
    int i;
    
    // Menyiapkan state
    state[0] = 0x61707865;
    state[1] = 0x3320646e;
    state[2] = 0x79622d36;
    state[3] = 0x3f0f6d7a;

    // Key
    for (i = 0; i < 8; i++) {
        state[4 + i] = ((uint32_t *)key)[i];
    }

    // Block counter
    state[12] = 0;
    state[13] = 0;
    state[14] = 0;
    state[15] = 0;

    // Menyiapkan output plaintext
    char *plaintext = (char *)malloc(length);
    
    for (long j = 0; j < length; j += 64) {
        // Salin state ke keystream
        memcpy(keystream, state, sizeof(state));
        
        // Enkripsi dengan Salsa20 (yang juga digunakan untuk dekripsi)
        salsa20_block(keystream);

        // XOR dengan ciphertext untuk menghasilkan plaintext
        for (i = 0; i < 64 && (j + i) < length; i++) {
            plaintext[j + i] = ciphertext[j + i] ^ ((char *)keystream)[i];
        }

        // Update counter
        state[8]++;
        if (state[8] == 0) {
            state[9]++;
        }
    }

    // Bebaskan memori ciphertext setelah dekripsi
    free(ciphertext);

    return plaintext;
}

/*int main() {
    char key[32] = "thisisaverysecretkey!!";
    char plaintext[] = "This is a test message for Salsa20 encryption and decryption!";
    long length = strlen(plaintext);

    // Enkripsi
    char *ciphertext_hex = encrypt_salsa20(plaintext, key, length);
    printf("Ciphertext (hex): %s\n", ciphertext_hex);

    // Dekripsi
    char *decrypted_text = decrypt_salsa20(ciphertext_hex, key, length);
    printf("Decrypted text: %s\n", decrypted_text);

    // Bebaskan memori
    free(ciphertext_hex);
    free(decrypted_text);

    return 0;
}*/
