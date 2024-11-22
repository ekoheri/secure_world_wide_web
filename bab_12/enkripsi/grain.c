#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define GRAIN_KEY_SIZE 16      // Ukuran kunci 16 byte (128-bit)
#define GRAIN_LFSR_SIZE 80     // Ukuran LFSR (sesuai desain Grain)
#define GRAIN_NFSR_SIZE 80     // Ukuran NFSR

typedef struct {
    uint8_t lfsr[GRAIN_LFSR_SIZE];
    uint8_t nfsr[GRAIN_NFSR_SIZE];
} GrainState;

// Inisialisasi LFSR dan NFSR dari kunci
void grain_init(GrainState *state, const uint8_t *key) {
    for (int i = 0; i < GRAIN_LFSR_SIZE; i++) {
        state->lfsr[i] = (i < GRAIN_KEY_SIZE * 8) ? (key[i / 8] >> (i % 8)) & 1 : 1;
    }
    for (int i = 0; i < GRAIN_NFSR_SIZE; i++) {
        state->nfsr[i] = (i < GRAIN_KEY_SIZE * 8) ? (key[i / 8] >> (i % 8)) & 1 : 0;
    }
}

// Fungsi untuk memperbarui LFSR
uint8_t lfsr_step(GrainState *state) {
    uint8_t feedback = state->lfsr[0] ^ state->lfsr[13] ^ state->lfsr[23] ^ state->lfsr[38] ^ state->lfsr[51] ^ state->lfsr[62];
    memmove(&state->lfsr[0], &state->lfsr[1], GRAIN_LFSR_SIZE - 1);
    state->lfsr[GRAIN_LFSR_SIZE - 1] = feedback;
    return feedback;
}

// Fungsi untuk memperbarui NFSR
uint8_t nfsr_step(GrainState *state) {
    uint8_t feedback = state->nfsr[0] ^ state->nfsr[26] ^ state->nfsr[56] ^ state->nfsr[67] ^ (state->nfsr[3] & state->nfsr[63]);
    memmove(&state->nfsr[0], &state->nfsr[1], GRAIN_NFSR_SIZE - 1);
    state->nfsr[GRAIN_NFSR_SIZE - 1] = feedback;
    return feedback;
}

// Menghasilkan satu byte keystream dari LFSR dan NFSR
uint8_t grain_keystream_byte(GrainState *state) {
    uint8_t lfsr_out = lfsr_step(state);
    uint8_t nfsr_out = nfsr_step(state);
    return lfsr_out ^ nfsr_out;
}

// Fungsi enkripsi dengan Grain stream cipher, hasil dalam heksadesimal
char *encrypt(const char *plaintext, const char *key, long length) {
    GrainState state;
    uint8_t key_bytes[GRAIN_KEY_SIZE] = {0};
    strncpy((char *)key_bytes, key, GRAIN_KEY_SIZE);
    grain_init(&state, key_bytes);

    // Alokasi memori untuk ciphertext heksadesimal
    char *hex_ciphertext = (char *)malloc(length * 2 + 1);
    if (!hex_ciphertext) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Enkripsi dan konversi ke heksadesimal
    for (int i = 0; i < length; i++) {
        uint8_t keystream_byte = grain_keystream_byte(&state);
        uint8_t encrypted_byte = plaintext[i] ^ keystream_byte;
        sprintf(hex_ciphertext + (i * 2), "%02X", encrypted_byte);
    }
    hex_ciphertext[length * 2] = '\0';

    return hex_ciphertext;
}

// Fungsi konversi heksadesimal ke byte array
uint8_t *hex_to_bytes(const char *hex, int *out_length) {
    int hex_length = strlen(hex);
    *out_length = hex_length / 2;
    uint8_t *bytes = (uint8_t *)malloc(*out_length);
    if (!bytes) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (int i = 0; i < *out_length; i++) {
        sscanf(hex + (i * 2), "%2hhx", &bytes[i]);
    }
    return bytes;
}

// Fungsi dekripsi dengan Grain stream cipher, membaca ciphertext dari heksadesimal
char *decrypt(const char *hex_ciphertext, const char *key, long length) {
    int byte_length;
    uint8_t *ciphertext_bytes = hex_to_bytes(hex_ciphertext, &byte_length);
    if (!ciphertext_bytes) return NULL;

    GrainState state;
    uint8_t key_bytes[GRAIN_KEY_SIZE] = {0};
    strncpy((char *)key_bytes, key, GRAIN_KEY_SIZE);
    grain_init(&state, key_bytes);

    // Alokasi memori untuk plaintext
    char *plaintext = (char *)malloc(byte_length + 1);
    if (!plaintext) {
        perror("Failed to allocate memory");
        free(ciphertext_bytes);
        return NULL;
    }

    // Dekripsi
    for (int i = 0; i < byte_length; i++) {
        uint8_t keystream_byte = grain_keystream_byte(&state);
        plaintext[i] = ciphertext_bytes[i] ^ keystream_byte;
    }
    plaintext[length] = '\0';

    free(ciphertext_bytes);
    return plaintext;
}

// gcc -shared -o grain -fPIC enkripsi/grain.c

/*int main() {
    char plaintext[] = "Hello, world!";
    char key[20] = "mysecretkey12345aaaa";  // Kunci 16 karakter
    int length = strlen(plaintext);

    printf("Plaintext: %s\n", plaintext);

    // Enkripsi
    char *encrypted_hex = grain_encrypt(plaintext, key, length);
    printf("Encrypted (hex): %s\n", encrypted_hex);

    // Dekripsi
    char *decrypted = grain_decrypt(encrypted_hex, key);
    printf("Decrypted: %s\n", decrypted);

    // Bebaskan memori
    free(encrypted_hex);
    free(decrypted);

    return 0;
}
*/