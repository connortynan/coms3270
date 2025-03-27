CC = gcc
PKG_CONFIG := pkg-config

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
GEN_DIR = $(BUILD_DIR)/art
ART_DIR = art
TAR_DIR = ../tars

SRC = $(shell find $(SRC_DIR) -type f -name "*.c")
INCLUDES = $(shell find $(SRC_DIR) -type f -name "*.h")
TARGET = $(BIN_DIR)/termune

BASE_CFLAGS = -Wall -g -I$(SRC_DIR) -I$(GEN_DIR)
BASE_LDFLAGS = -lm

ifeq ($(DEBUG), 1)
	BASE_CFLAGS += -DDEBUG_DEV_FLAGS
endif

ifneq ($(LOG_FILE), 0)
	BASE_CFLAGS += -DLOG_FILE=$(LOG_FILE)
endif

TXT_FILES := $(wildcard $(ART_DIR)/*.txt)
INC_FILES := $(patsubst $(ART_DIR)/%.txt,$(GEN_DIR)/%.inc,$(TXT_FILES))

all: $(TARGET)

$(TARGET): $(SRC) $(INCLUDES) $(INC_FILES)
	@mkdir -p $(BIN_DIR)
	@if $(PKG_CONFIG) --exists ncursesw; then \
		echo "[Makefile] ncursesw found: defining USE_WNCURSES"; \
		CFLAGS="$(BASE_CFLAGS) -DUSE_WNCURSES"; \
		LDFLAGS="$(BASE_LDFLAGS) -lncursesw"; \
	else \
		echo "[Makefile] ncursesw NOT found: building without USE_WNCURSES"; \
		CFLAGS="$(BASE_CFLAGS)"; \
		LDFLAGS="$(BASE_LDFLAGS) -lncurses"; \
	fi; \
	CMD="$(CC) $$CFLAGS $(SRC) -o $(TARGET) $$LDFLAGS"; \
	echo "$$CMD"; \
	$$CMD

# Generate .inc files from .txt using the art generator
$(GEN_DIR)/%.inc: $(ART_DIR)/%.txt $(ART_DIR)/art_inc_generator.py
	@mkdir -p $(dir $@)
	python3 $(ART_DIR)/art_inc_generator.py "$<" "$@"

clean:
	rm -rf $(BUILD_DIR)

tar: clean
	@if [ -z "$(version)" ]; then \
		echo "Please provide a version by running: make tar version=<version_number>"; \
		exit 1; \
	fi
	mkdir -p $(TAR_DIR)
	mkdir -p $(TAR_DIR)/tynan_connor.assignment-$(version)
	cp -r $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/* $(TAR_DIR)/tynan_connor.assignment-$(version)
	tar -czf $(TAR_DIR)/tynan_connor.assignment-$(version).tar.gz -C $(TAR_DIR) tynan_connor.assignment-$(version)
	rm -rf $(TAR_DIR)/tynan_connor.assignment-$(version)
