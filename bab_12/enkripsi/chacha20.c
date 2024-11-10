#include "chacha20.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CHACHA20_BLOCK_SIZE 64
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

void chacha20_block(uint32_t output[16], const uint32_t input[16]) {
    uint32_t state[16];
    memcpy(state, input, 16 * sizeof(uint32_t));

    for (int i = 0; i < 20; i += 2) {
        state[0] += state[4]; state[12] = ROTATE_LEFT(state[12] ^ state[0], 16);
        state[8] += state[12]; state[4] = ROTATE_LEFT(state[4] ^ state[8], 12);
        state[0] += state[4]; state[12] = ROTATE_LEFT(state[12] ^ state[0], 8);
        state[8] += state[12]; state[4] = ROTATE_LEFT(state[4] ^ state[8], 7);

        state[1] += state[5]; state[13] = ROTATE_LEFT(state[13] ^ state[1], 16);
        state[9] += state[13]; state[5] = ROTATE_LEFT(state[5] ^ state[9], 12);
        state[1] += state[5]; state[13] = ROTATE_LEFT(state[13] ^ state[1], 8);
        state[9] += state[13]; state[5] = ROTATE_LEFT(state[5] ^ state[9], 7);

        state[2] += state[6]; state[14] = ROTATE_LEFT(state[14] ^ state[2], 16);
        state[10] += state[14]; state[6] = ROTATE_LEFT(state[6] ^ state[10], 12);
        state[2] += state[6]; state[14] = ROTATE_LEFT(state[14] ^ state[2], 8);
        state[10] += state[14]; state[6] = ROTATE_LEFT(state[6] ^ state[10], 7);

        state[3] += state[7]; state[15] = ROTATE_LEFT(state[15] ^ state[3], 16);
        state[11] += state[15]; state[7] = ROTATE_LEFT(state[7] ^ state[11], 12);
        state[3] += state[7]; state[15] = ROTATE_LEFT(state[15] ^ state[3], 8);
        state[11] += state[15]; state[7] = ROTATE_LEFT(state[7] ^ state[11], 7);

        state[0] += state[5]; state[15] = ROTATE_LEFT(state[15] ^ state[0], 16);
        state[10] += state[15]; state[5] = ROTATE_LEFT(state[5] ^ state[10], 12);
        state[0] += state[5]; state[15] = ROTATE_LEFT(state[15] ^ state[0], 8);
        state[10] += state[15]; state[5] = ROTATE_LEFT(state[5] ^ state[10], 7);

        state[1] += state[6]; state[12] = ROTATE_LEFT(state[12] ^ state[1], 16);
        state[11] += state[12]; state[6] = ROTATE_LEFT(state[6] ^ state[11], 12);
        state[1] += state[6]; state[12] = ROTATE_LEFT(state[12] ^ state[1], 8);
        state[11] += state[12]; state[6] = ROTATE_LEFT(state[6] ^ state[11], 7);

        state[2] += state[7]; state[13] = ROTATE_LEFT(state[13] ^ state[2], 16);
        state[8] += state[13]; state[7] = ROTATE_LEFT(state[7] ^ state[8], 12);
        state[2] += state[7]; state[13] = ROTATE_LEFT(state[13] ^ state[2], 8);
        state[8] += state[13]; state[7] = ROTATE_LEFT(state[7] ^ state[8], 7);

        state[3] += state[4]; state[14] = ROTATE_LEFT(state[14] ^ state[3], 16);
        state[9] += state[14]; state[4] = ROTATE_LEFT(state[4] ^ state[9], 12);
        state[3] += state[4]; state[14] = ROTATE_LEFT(state[14] ^ state[3], 8);
        state[9] += state[14]; state[4] = ROTATE_LEFT(state[4] ^ state[9], 7);
    }

    for (int i = 0; i < 16; i++) {
        output[i] = state[i] + input[i];
    }
}

void bytes_to_hex(const uint8_t* bytes, size_t bytes_len, char* hex) {
    for (size_t i = 0; i < bytes_len; i++) {
        sprintf(hex + i * 2, "%02x", bytes[i]);
    }
    hex[bytes_len * 2] = '\0';
}

void hex_to_bytes(const char* hex, uint8_t* bytes, size_t bytes_len) {
    for (size_t i = 0; i < bytes_len; i++) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

char *hex_to_utf8(const char *hex_str) {
    int len = strlen(hex_str);
    char *utf8_str = malloc((len / 2) + 1);
    if (utf8_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < len; i += 2) {
        char hex_byte[3] = {hex_str[i], hex_str[i + 1], '\0'};
        utf8_str[i / 2] = (char)strtol(hex_byte, NULL, 16);
    }
    utf8_str[len / 2] = '\0';

    return utf8_str;
}

char *utf8_to_hex(const char *utf8_str) {
    int len = strlen(utf8_str);
    // Alokasi memori untuk buffer heksadesimal
    char *hex_str = malloc((len * 2) + 1);  // +1 untuk null terminator
    if (hex_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", (unsigned char)utf8_str[i]);
    }
    hex_str[len * 2] = '\0';  // Null-terminate string

    return hex_str;
}

char* chacha20_encrypt(
    const char *plaintext, 
    const uint8_t *key, 
    const uint8_t *nonce, 
    uint32_t counter) {

    uint32_t len = strlen(plaintext);
    uint8_t *ciphertext_bytes = malloc(len);
    if (!ciphertext_bytes) {
        return NULL;
    }

    uint32_t block[16] = {0};
    memcpy(block, key, 32);
    memcpy(block + 8, nonce, 8);
    block[12] = counter;

    uint32_t output_block[16];
    
    for (uint32_t i = 0; i < len; i++) {
        if (i % CHACHA20_BLOCK_SIZE == 0) {
            chacha20_block(output_block, block);
            block[12]++;
        }
        ciphertext_bytes[i] = plaintext[i] ^ ((uint8_t*)output_block)[i % CHACHA20_BLOCK_SIZE];
    }

    char *ciphertext_hex = malloc(len * 2 + 1);
    if (!ciphertext_hex) {
        free(ciphertext_bytes);
        return NULL;
    }
    bytes_to_hex(ciphertext_bytes, len, ciphertext_hex);

    free(ciphertext_bytes);
    return ciphertext_hex;
}

char* chacha20_decrypt(
    const char *ciphertext, 
    const uint8_t *key, 
    const uint8_t *nonce, 
    uint32_t counter) {

    uint32_t len = strlen(ciphertext) / 2;
    uint8_t *ciphertext_bytes = malloc(len);
    if (!ciphertext_bytes) {
        return NULL;
    }
    hex_to_bytes(ciphertext, ciphertext_bytes, len);

    uint8_t *plaintext_bytes = malloc(len);
    if (!plaintext_bytes) {
        free(ciphertext_bytes);
        return NULL;
    }

    uint32_t block[16] = {0};
    memcpy(block, key, 32);
    memcpy(block + 8, nonce, 8);
    block[12] = counter;

    uint32_t output_block[16];

    for (uint32_t i = 0; i < len; i++) {
        if (i % CHACHA20_BLOCK_SIZE == 0) {
            chacha20_block(output_block, block);
            block[12]++;
        }
        plaintext_bytes[i] = ciphertext_bytes[i] ^ ((uint8_t*)output_block)[i % CHACHA20_BLOCK_SIZE];
    }

    char *plaintext = malloc(len + 1);
    if (!plaintext) {
        free(ciphertext_bytes);
        free(plaintext_bytes);
        return NULL;
    }
    memcpy(plaintext, plaintext_bytes, len);
    plaintext[len] = '\0';

    free(ciphertext_bytes);
    free(plaintext_bytes);
    return plaintext;
}
