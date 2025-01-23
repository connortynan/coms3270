# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

# Directories
SRC_DIR = .
BUILD_DIR = build

# Source and target
SRC = $(SRC_DIR)/wordsearchgen.c
TARGET = $(BUILD_DIR)/wordsearchgen

# Build target
all: $(TARGET)

# Rule to compile and link
$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean
