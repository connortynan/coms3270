CC = gcc

SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include
SRC = $(wildcard $(SRC_DIR)/*.c)
INCLUDES = $(shell find $(INCLUDE_DIR) -type f -name "*.h")
TARGET = $(BIN_DIR)/termune

CFLAGS = -Wall -g -I$(INCLUDE_DIR)

ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG_DEV_FLAGS
endif

all: $(TARGET)

$(TARGET): $(SRC) $(INCLUDES)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) -lm

clean:
	rm -rf $(BIN_DIR)
