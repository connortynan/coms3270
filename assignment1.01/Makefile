CC = gcc
CFLAGS = -Wall -Werror -g -Iinclude
LDFLAGS =

SRC_DIR = src
BIN_DIR = bin
SRC = $(wildcard $(SRC_DIR)/*.c)
TARGET = $(BIN_DIR)/termune

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
