CC = gcc

SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include
TAR_DIR = ../tars

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

tar: clean
	@if [ -z "$(version)" ]; then \
		echo "Please provide a version by running: make tar version=<version_number>"; \
		exit 1; \
	fi
	@mkdir -p $(TAR_DIR)
	@tar -czf $(TAR_DIR)/tynan_connor.assignment-$(version).tar.gz -C $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))) .
	@echo "Tarball created at $(TAR_DIR)/tynan_connor.assignment-$(version).tar.gz"
