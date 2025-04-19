# Compiler and flags
CXX := g++
PKG_CONFIG := pkg-config

# Directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
GEN_DIR := $(BUILD_DIR)/art
ART_DIR := art
TAR_DIR := ../tars

# Detect ncursesw and apply defines and libs
HAVE_WNCURSES := $(shell $(PKG_CONFIG) --exists ncursesw && echo 1)

ifeq ($(HAVE_WNCURSES), 1)
    NCURSES_FLAGS := -DUSE_WNCURSES
    NCURSES_LIBS := -lncursesw
    $(info [Makefile] ncursesw found: defining USE_WNCURSES)
else
    NCURSES_FLAGS :=
    NCURSES_LIBS := -lncurses
    $(info [Makefile] ncursesw NOT found: building without USE_WNCURSES)
endif

# Source and target
SRC := $(shell find $(SRC_DIR) -name "*.cpp")
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP := $(OBJ:.o=.d)
TARGET := $(BIN_DIR)/termune

# Art assets
TXT_FILES := $(wildcard $(ART_DIR)/*.txt)
INC_FILES := $(patsubst $(ART_DIR)/%.txt,$(GEN_DIR)/%.inc,$(TXT_FILES))

# Flags
CXXFLAGS := -Wall -g -std=c++17 -MMD -MP -I$(SRC_DIR) -I$(BUILD_DIR) $(NCURSES_FLAGS)
LDFLAGS := -lm $(NCURSES_LIBS)

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DDEBUG_DEV_FLAGS
endif

ifneq ($(LOG_FILE),)
	CXXFLAGS += -DLOG_FILE=$(LOG_FILE)
endif

.PHONY: all clean tar

all: $(TARGET)

# Final binary
$(TARGET): $(OBJ) $(INC_FILES)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile .cpp -> .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generate art .inc files
$(GEN_DIR)/%.inc: $(ART_DIR)/%.txt $(ART_DIR)/art_inc_generator.py
	@mkdir -p $(dir $@)
	python3 $(ART_DIR)/art_inc_generator.py "$<" "$@"

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Tarball
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

# Include generated .d dependency files
-include $(DEP)
