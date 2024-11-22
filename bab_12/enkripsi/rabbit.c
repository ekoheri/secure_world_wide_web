#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WORD_SIZE 32  // 32-bit word
#define KEY_SIZE 16   // Key size is 128 bits (16 bytes)
#define STATE_SIZE 8  // Rabbit state consists of 8 words (32 bytes)

typedef uint32_t word32;

// Rabbit state structure
typedef struct {
    word32 x[8];  // State (8 words)
    word32 c[8];  // Counter (8 words)
} rabbit_state_t;

// Rotate left (circular shift)
word32 rotate_left(word32 value, int shift) {
    return (value << shift) | (value >> (WORD_SIZE - shift));
}

// Key scheduling (initializes the Rabbit state)
void rabbit_key_schedule(rabbit_state_t *state, const char *key) {
    int i;
    word32 *x = state->x;
    word32 *c = state->c;

    // Copy key into state->x and initialize counters
    for (i = 0; i < 8; i++) {
        x[i] = 0; // Initialize x[i] to zero for now
        c[i] = 0; // Initialize counters to zero
    }
    
    // Copy key in chunks of 4 bytes to state->x (ensuring we stay within bounds)
    memcpy(x, key, KEY_SIZE > sizeof(state->x) ? sizeof(state->x) : KEY_SIZE);
    
    // Scramble the state with initial counter
    for (i = 0; i < 8; i++) {
        x[i] += c[i];
    }
}

// Generate next 32 bits of keystream
word32 rabbit_keystream_word(rabbit_state_t *state) {
    word32 t = 0;
    int i;

    // Update state by rotating words and applying transformations
    for (i = 0; i < 8; i++) {
        state->x[i] = rotate_left(state->x[i], 16);
        state->c[i] += state->x[i];
        t ^= state->x[i];
    }

    return t;
}

// Encryption function
char* encrypt(const char *plaintext, const char *key, long length) {
    rabbit_state_t state;
    rabbit_key_schedule(&state, key);

    char *ciphertext = malloc(length * 2 + 1);  // Allocate space for hex result
    if (!ciphertext) return NULL; // Check allocation success
    memset(ciphertext, 0, length * 2 + 1); // Initialize to avoid undefined behavior

    for (long i = 0; i < length; i++) {
        word32 keystream = rabbit_keystream_word(&state);  // Get 32-bit keystream word
        snprintf(ciphertext + i * 2, 3, "%02x", plaintext[i] ^ (keystream & 0xFF));  // Write hex directly to position
    }

    return ciphertext;
}

// Decryption function
char* decrypt(const char *ciphertext_hex, const char *key, long length) {
    rabbit_state_t state;
    rabbit_key_schedule(&state, key);

    long text_length = length;
    char *plaintext = malloc(text_length + 1);
    if (!plaintext) return NULL; // Check allocation success

    for (long i = 0; i < text_length; i++) {
        // Get the ciphertext byte from the hex representation
        char hex[3] = { ciphertext_hex[i * 2], ciphertext_hex[i * 2 + 1], '\0' };
        word32 ciphertext_byte = strtol(hex, NULL, 16);

        word32 keystream = rabbit_keystream_word(&state);  // Get 32-bit keystream word
        plaintext[i] = ciphertext_byte ^ (keystream & 0xFF);  // XOR with the least significant byte of keystream
    }

    plaintext[text_length] = '\0';  // Null terminate the string
    return plaintext;
}
