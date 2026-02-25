# Rosetta Binary Translator - Modular Build Makefile
#
# This Makefile builds the modularized Rosetta translation layer
# Architecture: x86_64 -> ARM64 binary translation
#

CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
CFLAGS += -O2 -g
CFLAGS += -std=c11
CFLAGS += -D_GNU_SOURCE

# macOS-specific flags
ifeq ($(shell uname -s), Darwin)
    CFLAGS += -D_DARWIN_C_SOURCE
endif

# Linux-specific flags
ifeq ($(shell uname -s), Linux)
    CFLAGS += -D_LINUX_C_SOURCE
endif

# ============================================================================
# Source files for modular build - Original modular structure
# ============================================================================

# Core library - x86_64 decoding and codegen
CORE_SRCS = \
    rosetta_utils.c \
    rosetta_hash.c \
    rosetta_vector.c \
    rosetta_cache.c \
    rosetta_transcache.c \
    rosetta_context.c \
    rosetta_memmgmt.c \
    rosetta_x86_decode.c \
    rosetta_codegen.c

# Translation modules
TRANSLATE_SRCS = \
    rosetta_translate_alu.c \
    rosetta_translate_memory.c \
    rosetta_translate_branch.c \
    rosetta_translate_bit.c \
    rosetta_translate_string.c \
    rosetta_translate_special.c \
    rosetta_translate_block.c \
    rosetta_translate_dispatch.c \
    rosetta_translate.c \
    rosetta_jit.c

# System and SIMD modules
SYSTEM_SRCS = \
    rosetta_simd.c \
    rosetta_simd_mem.c \
    rosetta_syscalls.c

# ============================================================================
# Refactored modular structure (rosetta_refactored.c split into modules)
# ============================================================================

# JIT code emitter
REFACTORED_CORE_SRCS = \
    rosetta_jit_emit.c

# Translation modules
REFACTORED_TRANS_SRCS = \
    rosetta_trans_alu.c \
    rosetta_trans_mem.c \
    rosetta_trans_branch.c \
    rosetta_trans_bit.c \
    rosetta_trans_string.c \
    rosetta_trans_special.c

# Floating point and NEON translation
REFACTORED_FP_SRCS = \
    rosetta_fp_translate.c

REFACTORED_SRCS = $(REFACTORED_CORE_SRCS) $(REFACTORED_TRANS_SRCS) $(REFACTORED_FP_SRCS)

# All source files
MODULAR_SRCS = $(CORE_SRCS) $(TRANSLATE_SRCS) $(SYSTEM_SRCS) $(REFACTORED_SRCS)

# Object files
MODULAR_OBJS = $(MODULAR_SRCS:.c=.o)

# Header files
HEADERS = \
    rosetta.h \
    rosetta_types.h \
    rosetta_x86_decode.h \
    rosetta_arm64_decode.h \
    rosetta_arm64_emit.h \
    rosetta_codegen.h \
    rosetta_translate.h \
    rosetta_translate_alu.h \
    rosetta_translate_memory.h \
    rosetta_translate_branch.h \
    rosetta_translate_bit.h \
    rosetta_translate_string.h \
    rosetta_translate_special.h \
    rosetta_translate_block.h \
    rosetta_translate_dispatch.h \
    rosetta_jit.h \
    rosetta_context.h \
    rosetta_simd.h \
    rosetta_simd_mem.h \
    rosetta_memmgmt.h \
    rosetta_cache.h \
    rosetta_transcache.h \
    rosetta_hash.h \
    rosetta_vector.h \
    rosetta_syscalls.h \
    rosetta_utils.h \
    rosetta_function_map.h \
    rosetta_refactored_types.h \
    rosetta_jit_emit.h \
    rosetta_trans_alu.h \
    rosetta_trans_mem.h \
    rosetta_trans_branch.h \
    rosetta_trans_bit.h \
    rosetta_trans_string.h \
    rosetta_trans_special.h \
    rosetta_fp_translate.h

# Main targets
all: librosetta.a test_jit test_translate

# Static library
librosetta.a: $(MODULAR_OBJS)
	ar rcs $@ $^

# Individual object compilation
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Test executables (build separately - each has its own main)
test_jit: test_jit.c librosetta.a
	$(CC) $(CFLAGS) -Wno-macro-redefined -o $@ test_jit.c -L. -lrosetta

test_translate: test_translate.c librosetta.a
	$(CC) $(CFLAGS) -Wno-macro-redefined -o $@ test_translate.c -L. -lrosetta

# Phony test target runs both tests
test: test_jit test_translate
	./test_jit
	./test_translate

# Clean build artifacts
clean:
	rm -f $(MODULAR_OBJS) librosetta.a test_jit test_translate

# Phony targets
.PHONY: all clean test install

# Install target (optional)
install: librosetta.a $(HEADERS)
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include/rosetta
	cp librosetta.a /usr/local/lib/
	cp $(HEADERS) /usr/local/include/rosetta/

# Help target
help:
	@echo "Rosetta Binary Translator Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build static library and test (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Install library and headers to /usr/local"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Source files: $(MODULAR_SRCS)"
