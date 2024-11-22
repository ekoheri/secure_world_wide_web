#ifndef ENCRYPTION_H
#define ENCRYPTION_H

char *call_encrypt (const char *plaintext, const char *key, long length);
char *call_decrypt (const char *ciphertext, const char *key, long length);

#endif