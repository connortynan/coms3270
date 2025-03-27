CC = gcc

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
GEN_DIR = $(BUILD_DIR)/art
ART_DIR = art
TAR_DIR = ../tars

SRC = $(shell find $(SRC_DIR) -type f -name "*.c")
INCLUDES = $(shell find $(SRC_DIR) -type f -name "*.h")
TARGET = $(BIN_DIR)/termune

CFLAGS = -Wall -g -I$(SRC_DIR) -I$(GEN_DIR)

ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG_DEV_FLAGS
endif

ifneq ($(LOG_FILE), 0)
	CFLAGS += -DLOG_FILE=$(LOG_FILE)
endif

TXT_FILES := $(wildcard $(ART_DIR)/*.txt)
INC_FILES := $(patsubst $(ART_DIR)/%.txt,$(GEN_DIR)/%.inc,$(TXT_FILES))

all: $(INC_FILES) $(TARGET)

# Compile target
$(TARGET): $(SRC) $(INCLUDES) $(INC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) -lm -lncursesw

# Generate .inc files from .txt files using the art generator
$(GEN_DIR)/%.inc: $(ART_DIR)/%.txt $(ART_DIR)/art_inc_generator.py
	@mkdir -p $(dir $@)
	python3 $(ART_DIR)/art_inc_generator.py "$<" "$@"

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

# Tar rule
tar: clean
	@if [ -z "$(version)" ]; then
