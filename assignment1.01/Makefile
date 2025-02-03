CC = gcc

SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include
SRC = $(wildcard $(SRC_DIR)/*.c)
INCLUDES = $(wildcard $(INCLUDE_DIR)/*.h)
TARGET = $(BIN_DIR)/termune

all: $(TARGET)

$(TARGET): $(SRC) $(INCLUDES)
	mkdir -p $(BIN_DIR)
	$(CC) -Wall -Werror -g -I$(INCLUDE_DIR) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) -lm

clean:
	rm -rf $(BIN_DIR)
