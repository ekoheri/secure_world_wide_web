#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Fungsi untuk mengonversi char ke hex
void char_to_hex(unsigned char byte, char *hex) {
    sprintf(hex, "%02x", byte);
}

// Fungsi untuk mengonversi string hex ke char
unsigned char hex_to_char(const char *hex) {
    unsigned int byte;
    sscanf(hex, "%2x", &byte);
    return (unsigned char) byte;
}

// Fungsi untuk mengenkripsi pesan dengan Vigenère Cipher dengan UTF-8 dan hasil heksadesimal
char* encrypt(const char *plaintext, const char *key, long length) {
    int i = 0, j = 0, keyLen = strlen(key);

    // Alokasi memori untuk ciphertext dalam bentuk heksadesimal
    char *ciphertext = (char *)malloc((length * 2 + 1) * sizeof(char)); // 2 char per byte untuk hex
    if (ciphertext == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    while (i < length) {
        unsigned char ch = plaintext[i];
        unsigned char keyChar = key[j % keyLen];

        // Geser berdasarkan karakter kunci dan tetap dalam rentang UTF-8
        unsigned char encryptedChar = (ch + keyChar) % 256;

        // Konversi hasil enkripsi ke hex
        char hex[3];
        char_to_hex(encryptedChar, hex);
        ciphertext[i * 2] = hex[0];
        ciphertext[i * 2 + 1] = hex[1];

        i++;
        j++;
    }

    // Pastikan string ciphertext diakhiri dengan null terminator
    ciphertext[length * 2] = '\0';
    return ciphertext;
}

// Fungsi untuk mendekripsi pesan dengan Vigenère Cipher dari hex
char* decrypt(const char *ciphertext, const char *key, long length) {
    int i = 0, j = 0, keyLen = strlen(key);

    // Alokasi memori untuk plaintext
    char *plaintext = (char *)malloc((length + 1) * sizeof(char));
    if (plaintext == NULL) {
        fprintf(stderr, "Error: tidak dapat mengalokasikan memori.\n");
        exit(1);
    }

    while (i < length) {
        // Ambil dua karakter dari ciphertext hex
        char hex[3] = {ciphertext[i * 2], ciphertext[i * 2 + 1]};
        unsigned char encryptedChar = hex_to_char(hex);

        unsigned char keyChar = key[j % keyLen];

        // Geser mundur berdasarkan karakter kunci dan tetap dalam rentang UTF-8
        unsigned char decryptedChar = (encryptedChar - keyChar + 256) % 256;

        plaintext[i] = decryptedChar;

        i++;
        j++;
    }

    // Pastikan string plaintext diakhiri dengan null terminator
    plaintext[i] = '\0';
    return plaintext;
}

/*int main() {
    // Sample key and plaintext
    const char key[] = "thisisaverysecretkey!!";
    const char plaintext[] = "This is a test message for Vigenère encryption and decryption!";
    long length = strlen(plaintext);

    // Encryption
    char *ciphertext = encrypt(plaintext, key, length);
    printf("Ciphertext (hex): %s\n", ciphertext);

    // Decryption
    char *decrypted_text = decrypt(ciphertext, key, length);
    printf("Decrypted text: %s\n", decrypted_text);

    // Clean up
    free(ciphertext);
    free(decrypted_text);

    return 0;
}
*/