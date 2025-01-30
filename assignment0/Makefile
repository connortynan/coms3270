CC = gcc
CFLAGS = -Wall -Werror -g
LDFLAGS =

BIN_DIR = bin
SRC = wordsearchgen.c
TARGET = $(BIN_DIR)/wordsearchgen

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
