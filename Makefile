# Makefile for ACP (Autonomous Command Protocol) Library
# Supports Linux, macOS, Windows (MinGW)

# Project configuration
PROJECT_NAME = acp
VERSION = 0.3.0
MAJOR_VERSION = 0
MINOR_VERSION = 3

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS = -DACP_VERSION_MAJOR=$(MAJOR_VERSION) -DACP_VERSION_MINOR=$(MINOR_VERSION)

# No heap allocation by default (can be overridden)
ifndef ACP_ENABLE_HEAP
    CPPFLAGS += -DACP_NO_HEAP=1
endif

# Platform detection and compiler-specific optimizations
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
    PLATFORM = linux
    SHARED_LIB_EXT = so
    SHARED_FLAGS = -fPIC -fvisibility=hidden
    CFLAGS += -fstack-protector-strong
    LDFLAGS += -Wl,-z,relro,-z,now
endif
ifeq ($(UNAME_S), Darwin)
    PLATFORM = macos
    SHARED_LIB_EXT = dylib
    SHARED_FLAGS = -fPIC -fvisibility=hidden
    CFLAGS += -fstack-protector-strong -mmacosx-version-min=10.9
endif
ifeq ($(OS), Windows_NT)
    PLATFORM = windows
    SHARED_LIB_EXT = dll
    SHARED_FLAGS = 
    LDFLAGS += -lws2_32
    CPPFLAGS += -D_WIN32_WINNT=0x0600
    # MinGW-specific optimizations
    ifeq ($(CC), gcc)
        CFLAGS += -fstack-protector-strong
    endif
endif

# Compiler detection and optimization
ifeq ($(CC), clang)
    CFLAGS += -Weverything -Wno-padded -Wno-packed
endif
ifeq ($(CC), gcc)
    CFLAGS += -Wformat=2 -Wformat-security
endif

# Directories
SRC_DIR = .
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
TEST_DIR = tests
EXAMPLE_DIR = examples
DOC_DIR = docs

# Source files
CORE_SOURCES = acp.c acp_framer.c acp_cobs.c acp_crypto.c acp_session.c acp_nvs.c acp_crc16.c acp_constants.c

# Platform-specific sources
ifeq ($(PLATFORM), windows)
    PLATFORM_SOURCES = acp_platform_windows.c
else
    PLATFORM_SOURCES = acp_platform_posix.c
endif

SOURCES = $(CORE_SOURCES) $(PLATFORM_SOURCES)

# Object files
OBJECTS = $(addprefix $(OBJ_DIR)/, $(SOURCES:.c=.o))

# Library targets
STATIC_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).$(SHARED_LIB_EXT)

# Test targets
TEST_SOURCES = $(wildcard $(TEST_DIR)/*_test.c)
TEST_BINARIES = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BIN_DIR)/%)

# Example targets
EXAMPLE_SOURCES = $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_BINARIES = $(EXAMPLE_SOURCES:$(EXAMPLE_DIR)/%.c=$(BIN_DIR)/%)

# Default target
.PHONY: all
all: static shared examples tests

# Static library
.PHONY: static
static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS) | $(LIB_DIR)
	ar rcs $@ $^

# Shared library (not for Windows in this release)
.PHONY: shared  
shared: $(SHARED_LIB)

$(SHARED_LIB): $(OBJECTS) | $(LIB_DIR)
ifeq ($(PLATFORM), windows)
	@echo "Shared library build deferred for Windows - static library only in v0.3"
else ifeq ($(PLATFORM), linux)
	$(CC) -shared $(SHARED_FLAGS) -Wl,-soname,lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(MAJOR_VERSION) \
		-Wl,--version-script=acp.map -o $@.$(VERSION) $^ $(LDFLAGS)
	ln -sf lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(VERSION) $(LIB_DIR)/lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(MAJOR_VERSION)
	ln -sf lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(MAJOR_VERSION) $(LIB_DIR)/lib$(PROJECT_NAME).$(SHARED_LIB_EXT)
else ifeq ($(PLATFORM), macos)
	$(CC) -shared $(SHARED_FLAGS) -install_name @rpath/lib$(PROJECT_NAME).$(MAJOR_VERSION).$(SHARED_LIB_EXT) \
		-compatibility_version $(MAJOR_VERSION).0 -current_version $(VERSION) -o $@.$(VERSION) $^ $(LDFLAGS)
	ln -sf lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(VERSION) $(LIB_DIR)/lib$(PROJECT_NAME).$(MAJOR_VERSION).$(SHARED_LIB_EXT)
	ln -sf lib$(PROJECT_NAME).$(MAJOR_VERSION).$(SHARED_LIB_EXT) $(LIB_DIR)/lib$(PROJECT_NAME).$(SHARED_LIB_EXT)
endif

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SHARED_FLAGS) -c $< -o $@

# Examples
.PHONY: examples
examples: $(EXAMPLE_BINARIES)

$(BIN_DIR)/%: $(EXAMPLE_DIR)/%.c $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(SRC_DIR) $< -L$(LIB_DIR) -l$(PROJECT_NAME) -o $@

# Tests
.PHONY: tests
tests: $(TEST_BINARIES)

$(BIN_DIR)/%_test: $(TEST_DIR)/%_test.c $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(SRC_DIR) $< -L$(LIB_DIR) -l$(PROJECT_NAME) -o $@

# Run all tests
.PHONY: check
check: tests
	@for test in $(TEST_BINARIES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done

# Documentation
.PHONY: docs
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		cd $(DOC_DIR) && doxygen Doxyfile; \
	else \
		echo "Doxygen not found. Install doxygen to generate documentation."; \
	fi

# Create directories
$(BUILD_DIR) $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean targets
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DOC_DIR)/html $(DOC_DIR)/latex

.PHONY: distclean
distclean: clean
	rm -f config.log config.status

# Install (optional)
PREFIX ?= /usr/local
.PHONY: install
install: static shared pkg-config
	install -d $(PREFIX)/include $(PREFIX)/lib $(PREFIX)/lib/pkgconfig
	install -m 644 acp_*.h $(PREFIX)/include/
	install -m 644 $(STATIC_LIB) $(PREFIX)/lib/
	install -m 644 $(BUILD_DIR)/acp.pc $(PREFIX)/lib/pkgconfig/
ifneq ($(PLATFORM), windows)
	install -m 755 $(LIB_DIR)/lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(VERSION) $(PREFIX)/lib/
	ln -sf lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(VERSION) $(PREFIX)/lib/lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(MAJOR_VERSION)
	ln -sf lib$(PROJECT_NAME).$(SHARED_LIB_EXT).$(MAJOR_VERSION) $(PREFIX)/lib/lib$(PROJECT_NAME).$(SHARED_LIB_EXT)
endif

# Generate pkg-config file
.PHONY: pkg-config
pkg-config: | $(BUILD_DIR)
	@echo "Generating pkg-config file..."
	@echo "prefix=$(PREFIX)" > $(BUILD_DIR)/acp.pc
	@echo "exec_prefix=\$${prefix}" >> $(BUILD_DIR)/acp.pc
	@echo "libdir=\$${prefix}/lib" >> $(BUILD_DIR)/acp.pc
	@echo "includedir=\$${prefix}/include" >> $(BUILD_DIR)/acp.pc
	@echo "" >> $(BUILD_DIR)/acp.pc
	@echo "Name: $(PROJECT_NAME)" >> $(BUILD_DIR)/acp.pc
	@echo "Description: Autonomous Command Protocol - Portable C99 framing library" >> $(BUILD_DIR)/acp.pc
	@echo "Version: $(VERSION)" >> $(BUILD_DIR)/acp.pc
	@echo "URL: https://github.com/pzanna/acp" >> $(BUILD_DIR)/acp.pc
	@echo "" >> $(BUILD_DIR)/acp.pc
	@echo "Libs: -L\$${libdir} -l$(PROJECT_NAME)" >> $(BUILD_DIR)/acp.pc
ifeq ($(PLATFORM), windows)
	@echo "Libs.private: -lws2_32" >> $(BUILD_DIR)/acp.pc
else
	@echo "Libs.private:" >> $(BUILD_DIR)/acp.pc
endif
	@echo "Cflags: -I\$${includedir}" >> $(BUILD_DIR)/acp.pc

# Help target
.PHONY: help
help:
	@echo "ACP Library Build System"
	@echo "Available targets:"
	@echo "  all       - Build static lib, shared lib (non-Windows), examples, and tests"
	@echo "  static    - Build static library only"
	@echo "  shared    - Build shared library (Linux/macOS only)"
	@echo "  examples  - Build example programs"
	@echo "  tests     - Build test programs"
	@echo "  check     - Run all tests"
	@echo "  docs      - Generate documentation with Doxygen"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install libraries and headers"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  ACP_ENABLE_HEAP=1  - Enable heap allocation features (default: disabled)"
	@echo "  PREFIX=/path       - Install prefix (default: /usr/local)"