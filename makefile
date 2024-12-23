# Compiler ve derleme bayrakları
CC = gcc
CFLAGS = -Wall -g
TARGET = shell

# Hedef dosya ve bağımlılıkları
all: $(TARGET)
	@echo "Program çalıştırılıyor..."
	@./$(TARGET) # Derlemeden sonra program otomatik olarak çalıştırılır

$(TARGET): shell.c
	$(CC) $(CFLAGS) -o $(TARGET) shell.c

# Temizlik komutu
clean:
	rm -f $(TARGET)

