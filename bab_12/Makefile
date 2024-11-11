# Nama program yang akan dihasilkan
TARGET = web_server

# Daftar file sumber
SRCS = web_server.c http.c enkripsi/chacha20.c keygen/keygen.c hash/sha256.c

# Compiler dan opsi kompilasi
CC = gcc
CFLAGS = 

# Perintah untuk membuat target
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET)

# Bersihkan file yang dihasilkan (opsional)
clean:
	rm -f $(TARGET)
