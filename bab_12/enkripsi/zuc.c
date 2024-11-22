#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ZUC constants and parameters
#define W 32        // Word size in bits
#define N 128       // Number of bits for state size
#define M 32        // Number of bits for each element
#define K 32        // Number of bits for key size

// Linear feedback shift register (LFSR) constants for ZUC
static uint32_t S[4]; // ZUC internal state (4 words, 32 bits each)

// Rotate left function
#define ROTL(x, y) (((x) << (y)) | ((x) >> (W - (y))))

// ZUC initialization function (key setup)
void zuc_key_setup(const uint8_t *key) {
    // Initialize state from key (simplified version)
    for (int i = 0; i < 4; i++) {
        S[i] = 0;
        for (int j = 0; j < 4; j++) {
            S[i] |= (uint32_t)key[i * 4 + j] << (8 * j);
        }
    }
}

// ZUC key stream generation
uint32_t zuc_generate_keystream() {
    // ZUC key stream generation logic (simplified)
    uint32_t keystream = 0;

    // Rotate state values and generate keystream (simple version)
    for (int i = 0; i < 4; i++) {
        S[i] = ROTL(S[i], 8); // Left rotate state value
        keystream ^= S[i];
    }
    return keystream;
}

// ZUC encryption function (simplified)
char *encrypt(const char *plaintext, const char *key, long length) {
    zuc_key_setup((const uint8_t *)key);
    
    uint8_t *input = (uint8_t *)malloc(length);
    memcpy(input, plaintext, length);

    uint8_t *ciphertext = (uint8_t *)malloc(length);

    for (long i = 0; i < length; i++) {
        uint32_t keystream = zuc_generate_keystream();
        ciphertext[i] = input[i] ^ (keystream & 0xFF);  // XOR with keystream (byte-by-byte)
    }

    // Convert ciphertext to hexadecimal
    char *hex_result = (char *)malloc(length * 2 + 1);
    for (long i = 0; i < length; i++) {
        sprintf(hex_result + i * 2, "%02X", ciphertext[i]);
    }
    hex_result[length * 2] = '\0';

    free(input);
    free(ciphertext);

    return hex_result;
}

// ZUC decryption function (simplified)
char *decrypt(const char *ciphertext_hex, const char *key, long length) {
    int byte_length = strlen(ciphertext_hex) / 2;
    uint8_t *ciphertext = (uint8_t *)malloc(byte_length);

    // Convert hexadecimal ciphertext to byte array
    for (int i = 0; i < byte_length; i++) {
        sscanf(ciphertext_hex + i * 2, "%2hhx", &ciphertext[i]);
    }

    zuc_key_setup((const uint8_t *)key);
    
    uint8_t *decrypted = (uint8_t *)malloc(byte_length);

    for (long i = 0; i < byte_length; i++) {
        uint32_t keystream = zuc_generate_keystream();
        decrypted[i] = ciphertext[i] ^ (keystream & 0xFF);  // XOR with keystream (byte-by-byte)
    }

    // Convert decrypted bytes to a string
    char *plaintext = (char *)malloc(byte_length + 1);
    memcpy(plaintext, decrypted, byte_length);
    plaintext[length] = '\0';

    free(ciphertext);
    free(decrypted);

    return plaintext;
}

//gcc -shared -o zuc -fPIC enkripsi/zuc.c

/*int main() {
    char plaintext[] = "Hello, world!";
    char key[16] = "thisisasecretkey";  // 16-byte key

    long length = strlen(plaintext);

    // Enkripsi
    char *encrypted_hex = zuc_encrypt(plaintext, key, length);
    printf("Encrypted (hex): %s\n", encrypted_hex);

    // Dekripsi
    char *decrypted = zuc_decrypt(encrypted_hex, key, length);
    printf("Decrypted: %s\n", decrypted);

    free(encrypted_hex);
    free(decrypted);

    return 0;
}
*/