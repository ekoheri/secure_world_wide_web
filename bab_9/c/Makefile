# Nama program yang akan dihasilkan
TARGET = cgi

# Daftar file sumber
SRCS = cgi.c http.c config.c log.c

# Compiler dan opsi kompilasi
CC = gcc
CFLAGS = 

# Perintah untuk membuat target
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET)

# Bersihkan file yang dihasilkan (opsional)
clean:
	rm -f $(TARGET)