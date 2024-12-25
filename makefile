# Selenay Hur g221210088
#Emir Vardar g211210071
#Ipek Dural g211210026


# Compiler ve derleme bayrakları
CC = gcc
CFLAGS = -Wall -g
TARGET = shell

# Hedef dosya ve bağımlılıkları
all: $(TARGET)
	@echo "Program çalıştırılıyor..."
	@./$(TARGET)

$(TARGET): myshell.c myshell.h
	$(CC) $(CFLAGS) -o $(TARGET) myshell.c

# Temizlik komutu
clean:
	rm -f $(TARGET)

