#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Fungsi untuk mengonversi nilai byte ke heksadesimal
void to_hex(const char *input, char *output, long length) {
    for (long i = 0; i < length; i++) {
        sprintf(output + 2 * i, "%02x", (unsigned char)input[i]);
    }
    output[2 * length] = '\0';  // Null-terminate
}

// Fungsi untuk menghasilkan keystream HC-128
void hc128_key_stream(uint32_t *P, uint32_t *Q, char *keystream, long length) {
    int i = 0;
    while (i < length) {
        // Step 1: Generate 16 bytes keystream based on P and Q
        uint32_t tmp = P[i % 128] + Q[i % 128];
        keystream[i] = (char)(tmp & 0xFF); // Ambil byte pertama dari hasil penjumlahan

        i++;
    }
}

// Fungsi untuk mengenkripsi pesan dengan HC-128
char* encrypt(const char *plaintext, const char *key, long length) {
    uint32_t P[128], Q[128];

    // Inisialisasi P dan Q berdasarkan kunci
    for (int i = 0; i < 128; i++) {
        P[i] = i;  // Inisialisasi P dengan nilai yang sederhana
        Q[i] = i + 128;  // Inisialisasi Q
    }

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
    hc128_key_stream(P, Q, keystream, length);

    // XOR keystream dengan plaintext
    for (long i = 0; i < length; i++) {
        ciphertext[i] = plaintext[i] ^ keystream[i];
    }

    // Pastikan string ciphertext diakhiri dengan null terminator
    ciphertext[length] = '\0';

    // Konversi ke heksadesimal
    char *ciphertext_hex = (char *)malloc((2 * length + 1) * sizeof(char));
    if (ciphertext_hex == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }
    to_hex(ciphertext, ciphertext_hex, length);

    free(keystream);
    return ciphertext_hex;
}

// Fungsi untuk mendekripsi pesan dengan HC-128
char* decrypt(const char *ciphertext_hex, const char *key, long length) {
    uint32_t P[128], Q[128];

    // Inisialisasi P dan Q berdasarkan kunci
    for (int i = 0; i < 128; i++) {
        P[i] = i;  // Inisialisasi P dengan nilai yang sederhana
        Q[i] = i + 128;  // Inisialisasi Q
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
    hc128_key_stream(P, Q, keystream, length);

    // XOR ciphertext dengan keystream untuk mendapatkan plaintext
    for (long i = 0; i < length; i++) {
        plaintext[i] = ciphertext[i] ^ keystream[i];
    }

    // Pastikan string plaintext diakhiri dengan null terminator
    plaintext[length] = '\0';

    free(keystream);
    free(ciphertext);
    return plaintext;
}

/*int main() {
    const char *plaintext = "Hello, HC-128!";
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
}*/
