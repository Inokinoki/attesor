# Rosetta 2 Binary Translator - API Documentation

## Overview

This document provides comprehensive API documentation for the Rosetta 2 binary translator, including all public interfaces, data structures, and usage patterns.

## Table of Contents

1. [Core Translation API](#core-translation-api)
2. [JIT Compilation API](#jit-compilation-api)
3. [Cache Management API](#cache-management-api)
4. [Decoder API](#decoder-api)
5. [Code Emission API](#code-emission-api)
6. [Syscall Translation API](#syscall-translation-api)
7. [Memory Management API](#memory-management-api)
8. [FP/SIMD Translation API](#fpsimd-translation-api)
9. [Data Structures](#data-structures)
10. [Usage Examples](#usage-examples)

---

## Core Translation API

### rosetta_translate.h

Main translation interface for converting x86_64 instructions to ARM64.

#### Translation Context

```c
typedef struct {
    // Guest state (x86_64)
    uint64_t guest_pc;           // Current x86_64 program counter
    uint64_t guest_registers[16]; // RAX-R15
    uint64_t guest_rsp;           // Stack pointer
    uint64_t guest_rflags;        // Flags register

    // Host state (ARM64)
    uint64_t host_registers[32]; // X0-X31, SP, PC
    uint32_t nzcv;               // ARM64 flags (NZCV)

    // Translation state
    void *translation_cache;     // Cache pointer
    uint64_t block_start;        // Start of current block
    uint32_t block_size;         // Size of translated block

    // Configuration
    uint32_t flags;              // Translation flags
    uint32_t opt_level;          // Optimization level (0-3)
} rosetta_context_t;
```

#### Core Translation Functions

##### `rosetta_translate_block()`

Translate a single x86_64 basic block to ARM64.

```c
/**
 * Translate a single x86_64 basic block to ARM64
 *
 * @param ctx    Translation context (must be initialized)
 * @param guest_pc Starting address of x86_64 block to translate
 * @param[out] host_code Pointer to translated ARM64 code
 * @param[out] code_size Size of translated code in bytes
 *
 * @return 0 on success, negative error code on failure
 *
 * Error codes:
 *   -1: Invalid context
 *   -2: Invalid guest PC
 *   -3: Translation failed
 *   -4: Code cache full
 *
 * Example:
 *   rosetta_context_t ctx;
 *   rosetta_init(&ctx);
 *
 *   void *host_code;
 *   uint32_t size;
 *   int ret = rosetta_translate_block(&ctx, 0x1000, &host_code, &size);
 *   if (ret == 0) {
 *       // Execute translated code
 *       ((void(*)(void))host_code)();
 *   }
 */
int rosetta_translate_block(rosetta_context_t *ctx,
                            uint64_t guest_pc,
                            void **host_code,
                            uint32_t *code_size);
```

##### `rosetta_translate_function()`

Translate an entire x86_64 function to ARM64.

```c
/**
 * Translate an entire x86_64 function to ARM64
 *
 * @param ctx    Translation context
 * @param guest_pc Starting address of function
 * @param[out] host_code Pointer to translated ARM64 code
 * @param[out] code_size Size of translated code
 *
 * @return 0 on success, negative error code on failure
 *
 * Note: This performs control flow analysis to identify
 * all basic blocks in the function and translates them
 * as a unit, enabling better optimizations.
 */
int rosetta_translate_function(rosetta_context_t *ctx,
                               uint64_t guest_pc,
                               void **host_code,
                               uint32_t *code_size);
```

##### `rosetta_init()`

Initialize the translation context.

```c
/**
 * Initialize Rosetta translation context
 *
 * @param ctx Context to initialize
 *
 * @return 0 on success, negative error code on failure
 *
 * Initializes:
 *   - Translation cache
 *   - Code cache
 *   - Register mappings
 *   - Decoder state
 *   - Emitter state
 */
int rosetta_init(rosetta_context_t *ctx);
```

##### `rosetta_cleanup()`

Clean up translation context.

```c
/**
 * Clean up Rosetta translation context
 *
 * @param ctx Context to clean up
 *
 * Releases:
 *   - Translation cache
 *   - Code cache
 *   - Allocated memory
 *
 * Note: Does NOT free the context structure itself.
 */
void rosetta_cleanup(rosetta_context_t *ctx);
```

---

## JIT Compilation API

### rosetta_jit.h

Just-in-time compilation interface for dynamic translation.

#### JIT Context

```c
typedef struct {
    // Code cache
    uint8_t *code_buffer;       // Allocated code buffer
    uint64_t code_size;         // Size of code buffer
    uint64_t code_offset;       // Current offset in buffer

    // Translation cache
    trans_cache_t *trans_cache; // Translation cache

    // Statistics
    uint64_t translations;      // Total translations
    uint64_t cache_hits;        // Cache hits
    uint64_t cache_misses;      // Cache misses

    // Configuration
    uint32_t cache_size;        // Translation cache size
    uint32_t buffer_size;       // Code buffer size
} jit_context_t;
```

#### JIT Functions

##### `jit_init()`

Initialize JIT context.

```c
/**
 * Initialize JIT compilation context
 *
 * @param ctx JIT context to initialize
 * @param cache_size Size of translation cache (entries)
 * @param buffer_size Size of code buffer (bytes)
 *
 * @return 0 on success, negative error code on failure
 *
 * Default sizes:
 *   cache_size: 4096 entries
 *   buffer_size: 8 MB
 */
int jit_init(jit_context_t *ctx,
            uint32_t cache_size,
            uint32_t buffer_size);
```

##### `jit_compile()`

JIT compile a single basic block.

```c
/**
 * JIT compile a basic block
 *
 * @param ctx JIT context
 * @param guest_pc Address of x86_64 block to compile
 * @param[out] host_code Compiled ARM64 code
 *
 * @return 0 on success, negative error code on failure
 *
 * Process:
 *   1. Check translation cache
 *   2. If miss: decode, translate, emit, cache
 *   3. Return cached or newly compiled code
 */
int jit_compile(jit_context_t *ctx,
               uint64_t guest_pc,
               void **host_code);
```

##### `jit_execute()`

Execute translated code.

```c
/**
 * Execute translated ARM64 code
 *
 * @param ctx JIT context
 * @param host_code Compiled ARM64 code to execute
 *
 * @return Return value from executed code
 *
 * Note: This is a wrapper that sets up the execution
 * environment and calls the translated code.
 */
uint64_t jit_execute(jit_context_t *ctx, void *host_code);
```

##### `jit_invalidate()`

Invalidate cached translations.

```c
/**
 * Invalidate cached translations
 *
 * @param ctx JIT context
 * @param guest_pc Starting address to invalidate
 *
 * Use cases:
 *   - Self-modifying code
 *   - Code unloading
 *   - Cache flush
 */
void jit_invalidate(jit_context_t *ctx, uint64_t guest_pc);
```

##### `jit_cleanup()`

Clean up JIT context.

```c
/**
 * Clean up JIT context
 *
 * @param ctx JIT context to clean up
 *
 * Releases:
 *   - Code buffer
 *   - Translation cache
 *   - Allocated memory
 */
void jit_cleanup(jit_context_t *ctx);
```

---

## Cache Management API

### rosetta_trans_cache.h

Translation cache for storing and retrieving translated blocks.

#### Cache Structures

```c
typedef struct {
    uint64_t guest_pc;     // Original x86_64 address
    void *host_addr;       // Translated ARM64 code
    uint32_t size;         // Size of translated block
    uint32_t hash;         // Hash of guest PC
    uint32_t flags;        // Block state flags
    uint32_t refcount;     // Reference count
} trans_cache_entry_t;

typedef struct {
    trans_cache_entry_t entries[REFACTORED_TRANSLATION_CACHE_SIZE];
    struct {
        uint8_t *buffer;   // Allocated code buffer
        uint64_t size;     // Size of buffer
        uint64_t offset;   // Current offset
    } code_cache;

    // Statistics
    uint64_t hits;
    uint64_t misses;
    uint64_t inserts;
    uint64_t flushes;
    uint64_t invalidations;
} trans_cache_t;
```

#### Cache Flags

```c
#define CACHE_FLAG_VALID    0x01  // Entry is valid
#define CACHE_FLAG_LINKED   0x02  // Block is chained
#define CACHE_FLAG_HOT      0x04  // Hot path (frequently executed)
#define CACHE_FLAG_READONLY 0x08  // Read-only (cannot be evicted)
```

#### Cache Functions

##### `trans_cache_init()`

Initialize translation cache.

```c
/**
 * Initialize translation cache
 *
 * @param cache Cache to initialize
 *
 * @return 0 on success, negative error code on failure
 */
int trans_cache_init(trans_cache_t *cache);
```

##### `trans_cache_lookup()`

Look up translated code in cache.

```c
/**
 * Look up translated code for a guest PC
 *
 * @param cache Translation cache
 * @param guest_pc x86_64 address to look up
 *
 * @return Pointer to ARM64 code, or NULL if not found
 *
 * Performance: ~5-10 cycles (hash + lookup + comparison)
 */
void *trans_cache_lookup(trans_cache_t *cache, uint64_t guest_pc);
```

##### `trans_cache_insert()`

Insert translated code into cache.

```c
/**
 * Insert translated code into cache
 *
 * @param cache Translation cache
 * @param guest_pc x86_64 address
 * @param host_addr ARM64 code address
 * @param size Size of translated code
 *
 * @return 0 on success, negative error code on failure
 *
 * Performance: ~20-30 cycles (hash + write + metadata)
 */
int trans_cache_insert(trans_cache_t *cache,
                      uint64_t guest_pc,
                      void *host_addr,
                      uint32_t size);
```

##### `trans_cache_invalidate()`

Invalidate a cache entry.

```c
/**
 * Invalidate a cache entry
 *
 * @param cache Translation cache
 * @param guest_pc Guest PC to invalidate
 *
 * Note: Marks entry as invalid but doesn't remove it.
 * It will be overwritten on next insert.
 */
void trans_cache_invalidate(trans_cache_t *cache, uint64_t guest_pc);
```

##### `trans_cache_flush()`

Flush entire cache.

```c
/**
 * Flush entire translation cache
 *
 * @param cache Translation cache to flush
 *
 * Note: Invalidates all entries but doesn't free memory.
 */
void trans_cache_flush(trans_cache_t *cache);
```

##### `trans_cache_get_stats()`

Get cache statistics.

```c
/**
 * Get cache performance statistics
 *
 * @param cache Translation cache
 * @param[out] stats Statistics structure
 *
 * Statistics:
 *   - hits: Number of cache hits
 *   - misses: Number of cache misses
 *   - hit_rate: hits / (hits + misses)
 */
typedef struct {
    uint64_t hits;
    uint64_t misses;
    double hit_rate;
} trans_cache_stats_t;

void trans_cache_get_stats(trans_cache_t *cache,
                           trans_cache_stats_t *stats);
```

---

## Decoder API

### rosetta_x86_decode.h

x86_64 instruction decoder.

#### Instruction Structure

```c
typedef struct {
    // Opcode
    uint8_t opcode;
    uint8_t prefix[4];
    uint8_t rex;
    uint8_t vex;
    uint8_t evex;

    // Operands
    x86_operand_t operands[4];
    uint8_t num_operands;

    // Instruction info
    uint8_t length;         // Instruction length
    uint64_t address;       // Instruction address

    // Flags
    uint32_t flags;         // Instruction flags
} x86_insn_t;
```

#### Decoder Functions

##### `x86_decode()`

Decode a single x86_64 instruction.

```c
/**
 * Decode x86_64 instruction
 *
 * @param data Pointer to instruction bytes
 * @param size Size of available data
 * @param address Instruction address
 * @param[out] insn Decoded instruction
 *
 * @return Instruction length, or 0 on error
 *
 * Supports:
 *   - All legacy instructions
 *   - SSE/AVX/AVX-512
 *   - VEX/EVEX encoding
 *   - Escape sequences
 */
uint8_t x86_decode(const uint8_t *data,
                  uint8_t size,
                  uint64_t address,
                  x86_insn_t *insn);
```

##### `x86_decode_length()`

Get instruction length without full decode.

```c
/**
 * Get x86_64 instruction length
 *
 * @param data Pointer to instruction bytes
 * @param size Size of available data
 *
 * @return Instruction length, or 0 on error
 *
 * Performance: Faster than full decode
 */
uint8_t x86_decode_length(const uint8_t *data, uint8_t size);
```

##### `x86_operand_type()`

Get operand type.

```c
/**
 * Get operand type information
 *
 * @param operand Operand to query
 *
 * @return Operand type (register, memory, immediate, etc.)
 */
x86_operand_type_t x86_operand_type(const x86_operand_t *operand);
```

---

## Code Emission API

### rosetta_arm64_emit.h

ARM64 code emitter.

#### Emitter Context

```c
typedef struct {
    uint8_t *buffer;         // Output buffer
    uint64_t size;           // Buffer size
    uint64_t offset;         // Current offset

    // Relocations
    struct {
        uint64_t offset;     // Offset in code
        uint64_t target;     // Target address
        uint32_t type;       // Relocation type
    } relocs[256];
    uint32_t num_relocs;
} arm64_emitter_t;
```

#### Emitter Functions

##### `arm64_emit_init()`

Initialize emitter.

```c
/**
 * Initialize ARM64 code emitter
 *
 * @param emit Emitter context
 * @param buffer Output buffer
 * @param size Buffer size
 *
 * @return 0 on success, negative error code on failure
 */
int arm64_emit_init(arm64_emitter_t *emit,
                   uint8_t *buffer,
                   uint64_t size);
```

##### `arm64_emit_add()`

Emit ADD instruction.

```c
/**
 * Emit ARM64 ADD instruction
 *
 * @param emit Emitter context
 * @param rd Destination register
 * @param rn Source register 1
 * @param rm Source register 2 (or immediate)
 * @param is_imm Whether rm is an immediate
 *
 * Supported variants:
 *   - ADD Xd, Xn, Xm (register)
 *   - ADD Xd, Xn, #imm (immediate)
 *   - ADD Wd, Wn, Wm (32-bit)
 *   - ADD Wd, Wn, #imm (32-bit immediate)
 */
void arm64_emit_add(arm64_emitter_t *emit,
                   uint8_t rd,
                   uint8_t rn,
                   uint64_t rm_or_imm,
                   bool is_imm);
```

##### `arm64_emit_sub()`

Emit SUB instruction.

```c
/**
 * Emit ARM64 SUB instruction
 *
 * @param emit Emitter context
 * @param rd Destination register
 * @param rn Source register 1
 * @param rm_or_imm Source register 2 or immediate
 * @param is_imm Whether rm is an immediate
 */
void arm64_emit_sub(arm64_emitter_t *emit,
                   uint8_t rd,
                   uint8_t rn,
                   uint64_t rm_or_imm,
                   bool is_imm);
```

##### `arm64_emit_branch()`

Emit branch instruction.

```c
/**
 * Emit ARM64 branch instruction
 *
 * @param emit Emitter context
 * @param type Branch type
 * @param target Target address (for relative branches)
 * @param reg Register (for register branches)
 *
 * Branch types:
 *   - B: Unconditional branch
 *   - B.cond: Conditional branch (EQ, NE, LT, GT, etc.)
 *   - BR: Branch to register
 *   - BL: Branch with link (call)
 *   - RET: Return
 */
void arm64_emit_branch(arm64_emitter_t *emit,
                      arm64_branch_type_t type,
                      uint64_t target,
                      uint8_t reg);
```

##### `arm64_emit_load_store()`

Emit load/store instruction.

```c
/**
 * Emit ARM64 load/store instruction
 *
 * @param emit Emitter context
 * @param type Load/store type
 * @param rt Destination/source register
 * @param rn Base register
 * @param offset Offset from base
 *
 * Load/store types:
 *   - LDR/STR: Load/Store register
 *   - LDP/STP: Load/Store pair
 *   - LDUR/STUR: Unscaled load/store
 */
void arm64_emit_load_store(arm64_emitter_t *emit,
                          arm64_mem_type_t type,
                          uint8_t rt,
                          uint8_t rn,
                          int64_t offset);
```

##### `arm64_emit_finalize()`

Finalize emitted code.

```c
/**
 * Finalize emitted code
 *
 * @param emit Emitter context
 *
 * Applies:
 *   - Relocations
 *   - Fixups
 *   - Patches
 *
 * @return 0 on success, negative error code on failure
 */
int arm64_emit_finalize(arm64_emitter_t *emit);
```

---

## Syscall Translation API

### rosetta_syscalls.h

System call translation interface.

#### Syscall Mapping

```c
typedef struct {
    uint32_t x86_num;     // x86_64 syscall number
    uint32_t arm_num;     // ARM64 syscall number
    uint8_t arg_remap[6]; // Argument remapping (0-5)
    uint32_t flags;       // Translation flags
} syscall_mapping_t;
```

#### Syscall Functions

##### `syscall_translate_number()`

Translate x86_64 syscall number to ARM64.

```c
/**
 * Translate x86_64 syscall number to ARM64
 *
 * @param x86_num x86_64 syscall number
 *
 * @return ARM64 syscall number, or -1 if unknown
 *
 * Supported syscalls:
 *   - File I/O: read, write, open, close, etc.
 *   - Memory: mmap, munmap, mprotect, brk
 *   - Process: getpid, fork, exit, etc.
 *   - Network: socket, connect, send, recv
 *   - Signal: rt_sigaction, rt_sigprocmask
 */
int32_t syscall_translate_number(uint32_t x86_num);
```

##### `syscall_translate_args()`

Translate syscall arguments.

```c
/**
 * Translate syscall arguments from x86_64 to ARM64
 *
 * @param x86_num x86_64 syscall number
 * @param x86_args x86_64 arguments
 * @param[out] arm_args ARM64 arguments
 *
 * @return Number of arguments, or negative error code
 */
int syscall_translate_args(uint32_t x86_num,
                          const uint64_t *x86_args,
                          uint64_t *arm_args);
```

##### `syscall_translate_result()`

Translate syscall result.

```c
/**
 * Translate syscall result from ARM64 to x86_64
 *
 * @param x86_num x86_64 syscall number
 * @param arm_result ARM64 result value
 *
 * @return x86_64 result value
 *
 * Note: Most syscalls have identical results, but some
 * (like mmap) may need adjustment.
 */
int64_t syscall_translate_result(uint32_t x86_num, int64_t arm_result);
```

---

## Memory Management API

### rosetta_memmgmt.h

Memory management for translated code.

#### Memory Functions

##### `mem_alloc_code()`

Allocate executable memory.

```c
/**
 * Allocate executable memory for translated code
 *
 * @param size Size to allocate
 *
 * @return Pointer to allocated memory, or NULL on failure
 *
 * Process:
 *   1. Allocate with PROT_READ | PROT_WRITE
 *   2. Emit code to memory
 *   3. Change to PROT_READ | PROT_EXEC
 *   4. Return pointer
 *
 * Note: Memory is page-aligned for mprotect efficiency.
 */
void *mem_alloc_code(uint64_t size);
```

##### `mem_free_code()`

Free executable memory.

```c
/**
 * Free executable memory
 *
 * @param ptr Memory to free
 * @param size Size of allocation
 *
 * Process:
 *   1. Change to PROT_READ | PROT_WRITE
 *   2. Free memory
 */
void mem_free_code(void *ptr, uint64_t size);
```

##### `mem_protect_code()`

Change memory protection.

```c
/**
 * Change memory protection for code
 *
 * @param ptr Memory pointer
 * @param size Size of region
 * @param prot New protection (PROT_READ, PROT_WRITE, PROT_EXEC)
 *
 * @return 0 on success, negative error code on failure
 */
int mem_protect_code(void *ptr, uint64_t size, int prot);
```

---

## FP/SIMD Translation API

### rosetta_simd.h

Floating-point and SIMD translation interface.

#### SIMD Functions

##### `simd_map_xmm_to_neon()`

Map XMM register to NEON.

```c
/**
 * Map x86_64 XMM register to ARM64 NEON register
 *
 * @param xmm_num XMM register number (0-15)
 * @param data_type Data type (scalar, vector, etc.)
 *
 * @return NEON register number and element type
 *
 * Mapping:
 *   - XMM0 (scalar double) → D0
 *   - XMM0 (scalar single) → S0
 *   - XMM0 (128-bit vector) → V0
 *   - YMM0 (256-bit) → V0 + V16
 *   - ZMM0 (512-bit) → V0 + V16 + V32 + V48
 */
neon_reg_t simd_map_xmm_to_neon(uint8_t xmm_num,
                               simd_data_type_t data_type);
```

##### `simd_translate_sse_to_neon()`

Translate SSE instruction to NEON.

```c
/**
 * Translate SSE instruction to NEON
 *
 * @param emit Emitter context
 * @param insn SSE instruction to translate
 *
 * @return 0 on success, negative error code on failure
 *
 * Supports:
 *   - SSE scalar (ADDSS, SUBSS, etc.)
 *   - SSE2 integer (PADDQ, PSUBQ, etc.)
 *   - SSE3/SSSE3 (ADDSUBPS, PSHUFB, etc.)
 *   - SSE4.x (PTEST, PMINSB, etc.)
 */
int simd_translate_sse_to_neon(arm64_emitter_t *emit,
                               const x86_insn_t *insn);
```

##### `simd_translate_avx_to_neon()`

Translate AVX instruction to NEON.

```c
/**
 * Translate AVX instruction to NEON
 *
 * @param emit Emitter context
 * @param insn AVX instruction to translate
 *
 * @return 0 on success, negative error code on failure
 *
 * Note: AVX (256-bit) requires 2 NEON instructions.
 *       AVX-512 (512-bit) requires 4 NEON instructions.
 */
int simd_translate_avx_to_neon(arm64_emitter_t *emit,
                               const x86_insn_t *insn);
```

---

## Data Structures

### Translation Block

```c
typedef struct {
    uint64_t guest_start;    // Start of x86_64 block
    uint64_t guest_end;      // End of x86_64 block
    void *host_code;         // Translated ARM64 code
    uint32_t code_size;      // Size of translated code
    uint32_t num_insns;      // Number of instructions

    // Block links
    struct trans_block *fallthrough;  // Fall-through target
    struct trans_block *taken;        // Branch target

    // Metadata
    uint32_t flags;          // Block flags
    uint32_t refcount;       // Reference count
    uint64_t exec_count;     // Execution count (for profiling)
} trans_block_t;
```

### Instruction Info

```c
typedef struct {
    const char *mnemonic;    // Instruction mnemonic
    const char *operand_str; // Operand string
    uint8_t length;          // Instruction length
    uint8_t category;        // Instruction category
    uint32_t flags;          // Instruction flags
} insn_info_t;
```

### Register Mapping

```c
typedef struct {
    uint8_t arm_reg;         // ARM64 register number
    uint8_t arm_size;        // ARM64 register size (32/64-bit)
    uint8_t arm_type;        // ARM64 register type
} reg_mapping_t;

// x86_64 to ARM64 register mapping
extern const reg_mapping_t x86_to_arm64_mapping[16];
```

---

## Usage Examples

### Example 1: Basic Translation

```c
#include "rosetta_translate.h"

int main() {
    // Initialize context
    rosetta_context_t ctx;
    if (rosetta_init(&ctx) != 0) {
        fprintf(stderr, "Failed to initialize Rosetta\n");
        return 1;
    }

    // Translate a basic block
    void *host_code;
    uint32_t code_size;
    int ret = rosetta_translate_block(&ctx, 0x1000,
                                      &host_code, &code_size);
    if (ret != 0) {
        fprintf(stderr, "Translation failed: %d\n", ret);
        rosetta_cleanup(&ctx);
        return 1;
    }

    // Execute translated code
    ((void(*)(void))host_code)();

    // Cleanup
    rosetta_cleanup(&ctx);
    return 0;
}
```

### Example 2: JIT Compilation with Caching

```c
#include "rosetta_jit.h"

int main() {
    // Initialize JIT
    jit_context_t jit;
    if (jit_init(&jit, 4096, 8 * 1024 * 1024) != 0) {
        fprintf(stderr, "Failed to initialize JIT\n");
        return 1;
    }

    // Compile and execute multiple times
    for (int i = 0; i < 1000; i++) {
        void *code;
        if (jit_compile(&jit, 0x1000, &code) == 0) {
            jit_execute(&jit, code);
        }
    }

    // Print statistics
    printf("Translations: %lu\n", jit.translations);
    printf("Cache hits: %lu\n", jit.cache_hits);
    printf("Cache misses: %lu\n", jit.cache_misses);
    printf("Hit rate: %.2f%%\n",
           100.0 * jit.cache_hits / (jit.cache_hits + jit.cache_misses));

    // Cleanup
    jit_cleanup(&jit);
    return 0;
}
```

### Example 3: Custom Cache Management

```c
#include "rosetta_trans_cache.h"

int main() {
    // Initialize cache
    trans_cache_t cache;
    trans_cache_init(&cache);

    // Lookup/insert pattern
    uint64_t guest_pc = 0x1000;
    void *host_code = trans_cache_lookup(&cache, guest_pc);

    if (!host_code) {
        // Cache miss - translate block
        host_code = translate_and_emit(guest_pc);
        trans_cache_insert(&cache, guest_pc,
                          host_code, code_size);
    }

    // Execute code
    ((void(*)(void))host_code)();

    // Print statistics
    trans_cache_stats_t stats;
    trans_cache_get_stats(&cache, &stats);
    printf("Hit rate: %.2f%%\n", stats.hit_rate * 100.0);

    // Cleanup
    trans_cache_cleanup(&cache);
    return 0;
}
```

### Example 4: SIMD Translation

```c
#include "rosetta_simd.h"
#include "rosetta_arm64_emit.h"

int translate_sse_instruction(uint8_t *x86_code,
                             uint8_t *arm64_buffer) {
    // Decode x86_64 instruction
    x86_insn_t insn;
    if (x86_decode(x86_code, 16, 0x1000, &insn) == 0) {
        return -1;  // Decode error
    }

    // Initialize emitter
    arm64_emitter_t emit;
    if (arm64_emit_init(&emit, arm64_buffer, 1024) != 0) {
        return -1;
    }

    // Translate SSE to NEON
    if (simd_translate_sse_to_neon(&emit, &insn) != 0) {
        return -1;
    }

    // Finalize code
    arm64_emit_finalize(&emit);
    return emit.offset;
}
```

---

## Error Handling

### Error Codes

```c
#define ROSETTA_SUCCESS           0
#define ROSETTA_ERROR_INVALID_CTX -1
#define ROSETTA_ERROR_INVALID_PC  -2
#define ROSETTA_ERROR_DECODE      -3
#define ROSETTA_ERROR_TRANSLATE   -4
#define ROSETTA_ERROR_EMIT        -5
#define ROSETTA_ERROR_CACHE_FULL  -6
#define ROSETTA_ERROR_NO_MEM      -7
```

### Error Reporting

```c
/**
 * Get last error message
 *
 * @return Error message string, or NULL if no error
 */
const char *rosetta_get_error(void);

/**
 * Set error message
 *
 * @param fmt Printf-style format string
 * @param ... Format arguments
 */
void rosetta_set_error(const char *fmt, ...);
```

---

## Performance Profiling

### Profiling Functions

##### `rosetta_profile_start()`

Start profiling.

```c
/**
 * Start performance profiling
 *
 * @param ctx Translation context
 *
 * Collects:
 *   - Translation time
 *   - Execution time
 *   - Cache statistics
 *   - Instruction counts
 */
void rosetta_profile_start(rosetta_context_t *ctx);
```

##### `rosetta_profile_stop()`

Stop profiling and get results.

```c
/**
 * Stop performance profiling
 *
 * @param ctx Translation context
 * @param[out] stats Profile statistics
 *
 * Statistics:
 *   - total_time: Total execution time
 *   - translate_time: Time spent translating
 *   - execute_time: Time spent executing
 *   - cache_hit_rate: Cache hit rate
 */
typedef struct {
    uint64_t total_time;
    uint64_t translate_time;
    uint64_t execute_time;
    double cache_hit_rate;
    uint64_t instructions_translated;
    uint64_t instructions_executed;
} profile_stats_t;

void rosetta_profile_stop(rosetta_context_t *ctx,
                         profile_stats_t *stats);
```

---

## Configuration Options

### Translation Flags

```c
#define ROSETTA_FLAG_OPTIMIZE      0x01  // Enable optimizations
#define ROSETTA_FLAG_DEBUG         0x02  // Debug mode
#define ROSETTA_FLAG_VERIFY        0x04  // Verify translations
#define ROSETTA_FLAG_PROFILE       0x08  // Enable profiling
#define ROSETTA_FLAG_CHAIN_BLOCKS  0x10  // Enable block chaining
#define ROSETTA_FLAG_CACHE_HOT     0x20  // Cache hot paths
```

### Optimization Levels

```c
#define ROSETTA_OPT_NONE    0  // No optimization
#define ROSETTA_OPT_BASIC   1  // Basic optimizations
#define ROSETTA_OPT_STANDARD 2  // Standard optimizations (default)
#define ROSETTA_OPT_AGGRESSIVE 3  // Aggressive optimizations
```

---

## Thread Safety

### Thread-Safety Guarantees

Most API functions are **not thread-safe** by default. For multi-threaded usage:

1. Use separate translation contexts per thread
2. Or use external synchronization

```c
// Thread-safe example with separate contexts
pthread_t threads[4];
rosetta_context_t contexts[4];

for (int i = 0; i < 4; i++) {
    rosetta_init(&contexts[i]);
    pthread_create(&threads[i], NULL,
                   thread_func, &contexts[i]);
}

for (int i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
    rosetta_cleanup(&contexts[i]);
}
```

---

## Best Practices

### 1. Always Initialize Context

```c
rosetta_context_t ctx;
if (rosetta_init(&ctx) != 0) {
    // Handle error
}
```

### 2. Check Return Values

```c
int ret = rosetta_translate_block(&ctx, pc, &code, &size);
if (ret != 0) {
    fprintf(stderr, "Translation error: %d\n", ret);
    // Handle error
}
```

### 3. Clean Up Resources

```c
// Always cleanup, even on error
rosetta_context_t ctx;
if (rosetta_init(&ctx) == 0) {
    // Use context
    rosetta_cleanup(&ctx);
}
```

### 4. Use Caching for Repeated Translations

```c
// Check cache first
void *code = trans_cache_lookup(&cache, pc);
if (!code) {
    // Cache miss - translate
    code = translate(pc);
    trans_cache_insert(&cache, pc, code, size);
}
```

### 5. Profile Before Optimizing

```c
rosetta_profile_start(&ctx);
// ... do work ...
rosetta_profile_stop(&ctx, &stats);
printf("Hit rate: %.2f%%\n", stats.cache_hit_rate * 100);
```

---

## See Also

- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [SIMD_FP_TRANSLATION.md](SIMD_FP_TRANSLATION.md) - SIMD/FP translation guide
- [CRYPTO_SIMD_TRANSLATION.md](CRYPTO_SIMD_TRANSLATION.md) - Cryptographic extensions
- [TESTING_GUIDE.md](TESTING_GUIDE.md) - Testing instructions

---

*Last updated: March 2026*
