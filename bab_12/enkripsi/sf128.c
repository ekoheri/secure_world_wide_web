#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define SF128_STATE_SIZE 128
#define SF128_KEY_SIZE 128

// Fungsi untuk mengonversi nilai byte ke heksadesimal
void to_hex(const char *input, char *output, long length) {
    for (long i = 0; i < length; i++) {
        sprintf(output + 2 * i, "%02x", (unsigned char)input[i]);
    }
    output[2 * length] = '\0';  // Null-terminate
}

// Fungsi untuk menghasilkan keystream SF-128
// Fungsi pembaruan state SF-128 (ini adalah variasi lebih kompleks)
void sf128_key_stream(uint32_t *state, char *keystream, long length) {
    // SF-128 menggunakan dua buah register X dan Y untuk pembaruan state
    uint32_t X[8], Y[8];
    for (int i = 0; i < 8; i++) {
        X[i] = state[i];  // Inisialisasi dengan state yang diberikan
        Y[i] = state[i + 8];
    }

    int i = 0;
    while (i < length) {
        // Kombinasi X dan Y dengan operasi bitwise untuk menghasilkan keystream
        uint32_t z = X[0] ^ Y[0];  // Contoh operasi XOR antara X dan Y
        uint32_t shift_val = (z >> 1) | (z << (32 - 1));  // Rotate 1 bit
        
        // Update state berdasarkan rotasi dan XOR
        for (int j = 0; j < 7; j++) {
            X[j] = X[j + 1];
            Y[j] = Y[j + 1];
        }
        X[7] = shift_val;
        Y[7] = z;

        // Hasilkan keystream byte dari nilai X dan Y
        keystream[i] = (char)(X[0] & 0xFF);  // Ambil byte pertama dari register X

        i++;
    }
}

// Fungsi untuk mengenkripsi pesan dengan SF-128
char* encrypt(const char *plaintext, const char *key, long length) {
    uint32_t state[16];

    // Inisialisasi state dari kunci
    for (int i = 0; i < 16; i++) {
        state[i] = key[i % strlen(key)];  // Ambil nilai kunci untuk state
    }

    // Alokasi memori untuk ciphertext
    char *ciphertext = (char *)malloc((length + 1) * sizeof(char));
    if (ciphertext == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    char *keystream = (char *)malloc(length * sizeof(char));
    if (keystream == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    // Hasilkan keystream
    sf128_key_stream(state, keystream, length);

    // XOR keystream dengan plaintext untuk menghasilkan ciphertext
    for (long i = 0; i < length; i++) {
        ciphertext[i] = plaintext[i] ^ keystream[i];
    }

    // Pastikan string ciphertext diakhiri dengan null terminator
    ciphertext[length] = '\0';

    // Konversi ciphertext ke heksadesimal
    char *ciphertext_hex = (char *)malloc((2 * length + 1) * sizeof(char));
    if (ciphertext_hex == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }
    to_hex(ciphertext, ciphertext_hex, length);

    free(keystream);
    return ciphertext_hex;
}

// Fungsi untuk mendekripsi pesan dengan SF-128
char* decrypt(const char *ciphertext_hex, const char *key, long length) {
    uint32_t state[16];

    // Inisialisasi state dari kunci
    for (int i = 0; i < 16; i++) {
        state[i] = key[i % strlen(key)];  // Ambil nilai kunci untuk state
    }

    // Alokasikan memori untuk ciphertext dalam bentuk byte
    char *ciphertext = (char *)malloc(length * sizeof(char));
    if (ciphertext == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    // Konversi heksadesimal ke byte array
    for (long i = 0; i < length; i++) {
        sscanf(ciphertext_hex + 2 * i, "%2hhx", &ciphertext[i]);
    }

    char *plaintext = (char *)malloc((length + 1) * sizeof(char));
    if (plaintext == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    char *keystream = (char *)malloc(length * sizeof(char));
    if (keystream == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    // Hasilkan keystream
    sf128_key_stream(state, keystream, length);

    // XOR ciphertext dengan keystream untuk menghasilkan plaintext
    for (long i = 0; i < length; i++) {
        plaintext[i] = ciphertext[i] ^ keystream[i];
    }

    // Pastikan string plaintext diakhiri dengan null terminator
    plaintext[length] = '\0';

    free(keystream);
    free(ciphertext);
    return plaintext;
}

/*
int main() {
    const char *plaintext = "Hello, SF-128!";
    const char *key = "SuperSecretKey";

    long length = strlen(plaintext);

    // Enkripsi
    char *ciphertext_hex = encrypt(plaintext, key, length);
    printf("Ciphertext (hex): %s\n", ciphertext_hex);

    // Dekripsi
    char *decrypted_text = decrypt(ciphertext_hex, key, length);
    printf("Decrypted text: %s\n", decrypted_text);

    // Bebaskan memori
    free(ciphertext_hex);
    free(decrypted_text);

    return 0;
}
*/