#ifndef VIGENERE_H
#define VIGENERE_H

char* encrypt(const char *plaintext, const char *key, long length);
char* decrypt(const char *ciphertext, const char *key, long length);
#endif