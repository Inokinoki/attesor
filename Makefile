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
    rosetta_jit_emit.c \
    rosetta_jit_emit_simd.c \
    rosetta_arm64_emit.c

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
    rosetta_fp_translate.c \
    rosetta_fp_helpers.c

# NEON/SIMD translation (new modular components)
REFACTORED_NEON_SRCS = \
    rosetta_trans_neon.c

# System instruction translation (new modular components)
REFACTORED_SYSTEM_SRCS = \
    rosetta_trans_system.c

# Cryptographic extensions (new modular components)
REFACTORED_CRYPTO_SRCS = \
    rosetta_crypto.c

# SIMD string utilities (new modular components)
REFACTORED_STRING_SIMD_SRCS = \
    rosetta_string_simd.c

# Syscall implementation (new modular components)
REFACTORED_SYSCALLS_IMPL_SRCS = \
    rosetta_syscalls_impl.c

# Memory utilities (new modular components)
REFACTORED_MEMORY_UTILS_SRCS = \
    rosetta_memory_utils.c

# String utilities (new modular components)
REFACTORED_STRING_UTILS_SRCS = \
    rosetta_string_utils.c

# Translation helpers (new modular components)
REFACTORED_TRANS_HELPERS_SRCS = \
    rosetta_trans_helpers.c

# Translation implementation modules (full functional translation)
REFACTORED_TRANS_IMPL_SRCS = \
    rosetta_translate_alu_impl.c \
    rosetta_translate_memory_impl.c \
    rosetta_translate_branch_impl.c \
    rosetta_translate_special_impl.c

# Translation dispatch module
REFACTORED_TRANS_DISPATCH_SRCS = \
    rosetta_trans_dispatch.c

# SIMD memory helpers module
REFACTORED_SIMD_MEM_HELPERS_SRCS = \
    rosetta_simd_mem_helpers.c

# New JIT core module (header only - functions in rosetta_codegen.c)
REFACTORED_JIT_CORE_SRCS =

# New x86 instruction decoding module
REFACTORED_X86_INSNS_SRCS = \
    rosetta_x86_insns.c

# Refactored utilities (from rosetta_refactored.c)
REFACTORED_UTIL_SRCS = \
    rosetta_refactored_vector.c \
    rosetta_refactored_helpers.c

# Runtime and entry point
REFACTORED_RUNTIME_SRCS = \
    rosetta_runtime.c

REFACTORED_SRCS = $(REFACTORED_CORE_SRCS) $(REFACTORED_TRANS_SRCS) $(REFACTORED_FP_SRCS) $(REFACTORED_NEON_SRCS) $(REFACTORED_SYSTEM_SRCS) $(REFACTORED_CRYPTO_SRCS) $(REFACTORED_STRING_SIMD_SRCS) $(REFACTORED_SYSCALLS_IMPL_SRCS) $(REFACTORED_MEMORY_UTILS_SRCS) $(REFACTORED_STRING_UTILS_SRCS) $(REFACTORED_TRANS_HELPERS_SRCS) $(REFACTORED_TRANS_IMPL_SRCS) $(REFACTORED_TRANS_DISPATCH_SRCS) $(REFACTORED_SIMD_MEM_HELPERS_SRCS) $(REFACTORED_JIT_CORE_SRCS) $(REFACTORED_X86_INSNS_SRCS) $(REFACTORED_UTIL_SRCS) $(REFACTORED_RUNTIME_SRCS)

# All source files (include REFACTORED_SRCS which contains NEON, System, Crypto, String SIMD modules)
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
    rosetta_jit_emit.h \
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
    rosetta_refactored_vector.h \
    rosetta_refactored_helpers.h \
    rosetta_jit_emit.h \
    rosetta_jit_emit_simd.h \
    rosetta_trans_alu.h \
    rosetta_trans_mem.h \
    rosetta_trans_branch.h \
    rosetta_trans_bit.h \
    rosetta_trans_string.h \
    rosetta_trans_special.h \
    rosetta_fp_translate.h \
    rosetta_fp_helpers.h \
    rosetta_runtime.h \
    rosetta_x86_predicates.h \
    rosetta_arm64_decode_helpers.h \
    rosetta_x86_insn.h \
    rosetta_trans_neon.h \
    rosetta_trans_system.h \
    rosetta_syscalls_impl.h \
    rosetta_crypto.h \
    rosetta_string_simd.h \
    rosetta_memory_utils.h \
    rosetta_string_utils.h \
    rosetta_trans_helpers.h \
    rosetta_translate_alu_impl.h \
    rosetta_translate_memory_impl.h \
    rosetta_translate_branch_impl.h \
    rosetta_translate_special_impl.h \
    rosetta_jit_core.h \
    rosetta_x86_insns.h \
    rosetta_arm64_insns.h \
    rosetta_trans_dispatch.h

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
