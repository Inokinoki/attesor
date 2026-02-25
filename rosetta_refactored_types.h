/* ============================================================================
 * Rosetta Translator - Core Type Definitions
 * ============================================================================
 *
 * This file defines the core data structures used throughout the Rosetta
 * binary translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_TYPES_H
#define ROSETTA_REFACTORED_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>

/* ============================================================================
 * Basic Type Definitions
 * ============================================================================ */

typedef uint8_t   byte;
typedef uint16_t  word;
typedef uint32_t  dword;
typedef uint64_t  qword;
typedef int64_t   longlong;
typedef uint64_t  ulonglong;

/* ============================================================================
 * ARM64 Register Context Structures
 * ============================================================================ */

/**
 * Vector128 - 128-bit NEON/FP register
 * Used for SIMD operations and floating-point data
 */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} Vector128;

/**
 * FPContext - ARM64 Floating Point Control
 */
typedef struct {
    uint32_t fpcr;  /* Floating Point Control Register */
    uint32_t fpsr;  /* Floating Point Status Register */
} FPContext;

/**
 * GPRContext - ARM64 General Purpose Registers
 */
typedef struct {
    uint64_t x[30];  /* X0-X29 */
    uint64_t lr;     /* X30 - Link Register */
    uint64_t sp;     /* Stack Pointer */
    uint64_t pc;     /* Program Counter */
    uint64_t nzcv;   /* Condition flags (N, Z, C, V) */
} GPRContext;

/**
 * VectorContext - ARM64 Vector Registers (V0-V31)
 */
typedef struct {
    Vector128 v[32];
} VectorContext;

/**
 * CPUContext - Full ARM64 CPU Context
 */
typedef struct {
    GPRContext    gpr;
    VectorContext vec;
    FPContext     fp;
} CPUContext;

/* ============================================================================
 * Rosetta State Structures
 * ============================================================================ */

/**
 * TranslationBlock - Represents a translated code block
 */
typedef struct {
    uint64_t guest_pc;      /* Guest (ARM64) PC */
    uint64_t host_pc;       /* Host (x86_64) PC */
    uint32_t size;          /* Block size in bytes */
    uint32_t flags;         /* Block flags */
} TranslationBlock;

/**
 * TranslationCacheEntry - Cache entry for translated blocks
 */
typedef struct {
    uint64_t guest_addr;
    uint64_t host_addr;
    uint32_t hash;
    uint32_t refcount;
} TranslationCacheEntry;

/**
 * MemoryMapping - Guest-Host memory mapping
 */
typedef struct {
    uint64_t guest_base;
    uint64_t host_base;
    uint64_t size;
    uint32_t prot;          /* Protection flags */
    uint32_t flags;         /* Mapping flags */
} MemoryMapping;

/**
 * ThreadState - Per-thread translation state
 */
typedef struct {
    CPUContext    cpu;
    void         *tls_base;
    uint32_t      thread_id;
    uint32_t      flags;
    uint64_t      syscall_nr;
    int64_t       syscall_result;
} ThreadState;

/* ============================================================================
 * Function Pointer Types
 * ============================================================================ */

typedef void (*translate_func_t)(ThreadState *);
typedef int (*syscall_handler_t)(ThreadState *, int nr);
typedef void *(*memory_map_func_t)(uint64_t guest, uint64_t size);
typedef int (*memory_unmap_func_t)(uint64_t guest, uint64_t size);

/* ============================================================================
 * Code Buffer for JIT Emission
 * ============================================================================ */

typedef struct {
    uint8_t *buffer;        /* Pointer to code buffer */
    size_t   size;          /* Total buffer size */
    size_t   offset;        /* Current write offset */
    int      error;         /* Error flag */
} CodeBuffer;

/* ============================================================================
 * Translation Result
 * ============================================================================ */

typedef struct {
    void    *code;               /* Pointer to translated code */
    size_t   size;               /* Size of translated code in bytes */
    uint64_t guest_pc;           /* Guest PC of block start */
    uint64_t host_pc;            /* Host PC of translated code */
    int      insn_count;         /* Number of instructions translated */
    bool     ends_with_branch;   /* Block ends with branch/return */
    bool     is_cached;          /* Block is in translation cache */
} TranslationResult;

/* ============================================================================
 * ARM64 Condition Codes
 * ============================================================================ */

typedef enum {
    COND_EQ = 0x0,  /* Equal - Z */
    COND_NE = 0x1,  /* Not equal - !Z */
    COND_CS = 0x2,  /* Carry set - C */
    COND_CC = 0x3,  /* Carry clear - !C */
    COND_MI = 0x4,  /* Minus - N */
    COND_PL = 0x5,  /* Plus - !N */
    COND_VS = 0x6,  /* Overflow - V */
    COND_VC = 0x7,  /* No overflow - !V */
    COND_HI = 0x8,  /* Unsigned higher - C && !Z */
    COND_LS = 0x9,  /* Unsigned lower or same - !C || Z */
    COND_LT = 0xA,  /* Signed less than - N != V */
    COND_GE = 0xB,  /* Signed greater or equal - N == V */
    COND_LE = 0xC,  /* Signed less or equal - Z || N != V */
    COND_GT = 0xD,  /* Signed greater than - !Z && N == V */
    COND_AL = 0xE,  /* Always - unconditional */
    COND_NV = 0xF   /* Never - always false */
} arm64_cond_t;

/* ============================================================================
 * NEON/SIMD Helper Macros
 * ============================================================================ */

/* Extract byte from vector */
#define VGET_BYTE(v, i) (((uint8_t*)(v))[(i)])

/* Set byte in vector */
#define VSET_BYTE(v, i, b) (((uint8_t*)(v))[(i)] = (b))

/* NEON UMINV - Unsigned Minimum Across Vector */
static inline uint8_t neon_uminv(const uint8_t vec[16]) {
    uint8_t min = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] < min) min = vec[i];
    }
    return min;
}

/* NEON UMAXV - Unsigned Maximum Across Vector */
static inline uint8_t neon_umaxv(const uint8_t vec[16]) {
    uint8_t max = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] > max) max = vec[i];
    }
    return max;
}

#endif /* ROSETTA_REFACTORED_TYPES_H */
