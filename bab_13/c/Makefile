# Nama program yang akan dihasilkan
TARGET = web_server

# Daftar file sumber
SRCS = web_server.c http.c config.c log.c encryption.c

# Compiler dan opsi kompilasi
CC = gcc
CFLAGS = 

# Perintah untuk membuat target
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) -ldl

# Bersihkan file yang dihasilkan (opsional)
clean:
	rm -f $(TARGET)