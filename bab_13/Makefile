# Nama file output
TARGET = browser

# Daftar file sumber
SRCS = browser.c http.c external.c enkripsi/chacha20.c hash/sha256.c keygen/keygen.c

# C compiler
CC = gcc

# Flags untuk pkg-config
PKG_CFLAGS = $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.0 libxml-2.0)
PKG_LIBS = $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.0 libxml-2.0)

# Library tambahan
LIBS = -lcurl

# Aturan default: membangun aplikasi
$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(PKG_CFLAGS) $(PKG_LIBS) $(LIBS)

# Aturan untuk membersihkan file yang dihasilkan
clean:
	rm -f $(TARGET)
