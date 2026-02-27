#ifndef ROSETTA_TYPES_H
#define ROSETTA_TYPES_H

/* ============================================================================
 * Rosetta Binary Translator - Type Definitions
 * ============================================================================ */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * Basic Type Definitions
 * ============================================================================ */

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;

/* ============================================================================
 * Vector Types (128-bit NEON/XMM registers)
 * ============================================================================ */

typedef struct {
    u64 lo;
    u64 hi;
} v128_t;

typedef union {
    u8  u8[16];
    u16 u16[8];
    u32 u32[4];
    u64 u64[2];
    s8  s8[16];
    s16 s16[8];
    s32 s32[4];
    s64 s64[2];
    f32 f32[4];
    f64 f64[2];
} vec128_t;

/* ============================================================================
 * ARM64 Register File
 * ============================================================================ */

#define ARM64_NUM_GPRS  32  /* X0-X30, SP */
#define ARM64_NUM_VREGS 32  /* V0-V31 (SIMD/FP) */
#define ARM64_PSTATE    32  /* Processor state */

typedef struct {
    u64 x[32];          /* General-purpose registers X0-X30 */
    u64 sp;             /* Stack pointer (X31) */
    u64 pc;             /* Program counter */
    u64 pstate;         /* Processor state (NZCV flags) */

    /* SIMD/FP registers V0-V31 (each 128-bit) */
    vec128_t v[32];

    /* FP control registers */
    u32 fpsr;           /* FP Status Register */
    u32 fpcr;           /* FP Control Register */
} arm64_context_t;

/* ============================================================================
 * x86_64 Register Mapping
 * ============================================================================ */

#define X86_RAX     0
#define X86_RCX     1
#define X86_RDX     2
#define X86_RBX     3
#define X86_RSI     4
#define X86_RDI     5
#define X86_RBP     6
#define X86_RSP     7
#define X86_R8      8
#define X86_R9      9
#define X86_R10     10
#define X86_R11     11
#define X86_R12     12
#define X86_R13     13
#define X86_R14     14
#define X86_R15     15

#define X86_NUM_GPRS    16
#define X86_NUM_XMMS    16

typedef struct {
    u64 r[16];          /* General-purpose registers RAX-R15 */
    u64 rip;            /* Instruction pointer */
    u64 rflags;         /* Flags register */

    /* XMM registers (each 128-bit) */
    vec128_t xmm[16];
} x86_context_t;

/* ============================================================================
 * NZCV Flag Definitions (ARM64 PSTATE)
 * ============================================================================ */

#define NZCV_N_SHIFT    31  /* Negative flag */
#define NZCV_Z_SHIFT    30  /* Zero flag */
#define NZCV_C_SHIFT    29  /* Carry flag */
#define NZCV_V_SHIFT    28  /* Overflow flag */

#define NZCV_N          (1U << 31)
#define NZCV_Z          (1U << 30)
#define NZCV_C          (1U << 29)
#define NZCV_V          (1U << 28)

/* ============================================================================
 * ARM64 Condition Codes
 * ============================================================================ */

typedef enum {
    COND_EQ = 0,    /* Equal (Z==1) */
    COND_NE = 1,    /* Not equal (Z==0) */
    COND_CS = 2,    /* Carry set / Higher or same (C==1) */
    COND_CC = 3,    /* Carry clear (C==0) */
    COND_MI = 4,    /* Minus / Negative (N==1) */
    COND_PL = 5,    /* Plus / Positive (N==0) */
    COND_VS = 6,    /* Overflow set (V==1) */
    COND_VC = 7,    /* Overflow clear (V==0) */
    COND_HI = 8,    /* Unsigned higher (C==1 && Z==0) */
    COND_LS = 9,    /* Unsigned lower or same (C==0 || Z==1) */
    COND_GE = 10,   /* Signed >= (N==V) */
    COND_LT = 11,   /* Signed < (N!=V) */
    COND_GT = 12,   /* Signed > (Z==0 && N==V) */
    COND_LE = 13,   /* Signed <= (Z==1 || N!=V) */
    COND_AL = 14,   /* Always */
    COND_NV = 15    /* Never */
} arm64_cond_t;

/* ============================================================================
 * Translation Cache Entry
 * ============================================================================ */

#define TRANSLATION_CACHE_BITS  12      /* 4096 entries */
#define TRANSLATION_CACHE_SIZE  (1U << TRANSLATION_CACHE_BITS)
#define TRANSLATION_CACHE_MASK  (TRANSLATION_CACHE_SIZE - 1)

typedef struct translation_entry {
    u64 guest_pc;                   /* Guest ARM64 PC */
    u64 host_addr;                  /* Host x86_64 address */
    u32 block_size;                 /* Size of translated block */
    u32 flags;                      /* Block flags */
    struct translation_entry *next; /* Next entry (chaining) */
} translation_entry_t;

#define BLOCK_FLAG_VALID    0x01
#define BLOCK_FLAG_HOT      0x02
#define BLOCK_FLAG_LINKED   0x04

/* ============================================================================
 * Code Buffer for JIT Emission
 * ============================================================================ */

#define CODE_BUFFER_DEFAULT_SIZE  65536   /* 64KB default */

typedef struct code_buffer {
    u8  *buffer;                    /* Code buffer */
    u32 size;                       /* Total buffer size */
    u32 offset;                     /* Current write offset */
    bool error;                     /* Error flag */
    bool owns_buffer;               /* Whether we own the buffer */
} code_buffer_t;

/* ============================================================================
 * Thread State
 * ============================================================================ */

/* Use RosettaThreadState to avoid conflict with macOS mach/thread_status.h */
typedef struct RosettaThreadState {
    /* Guest ARM64 state */
    arm64_context_t guest;

    /* Host x86_64 state */
    x86_context_t host;

    /* Translation state */
    u64 current_pc;                 /* Current guest PC */
    void *current_block;            /* Current translated block */

    /* Syscall state */
    s64 syscall_nr;                 /* Syscall number */
    s64 syscall_result;             /* Syscall result */
    s64 syscall_errno;              /* Syscall errno */

    /* Signal state */
    u32 pending_signals;            /* Pending signals */
    void *signal_trampoline;        /* Signal trampoline address */

    /* Memory management */
    void *guest_base;               /* Guest memory base */
    size_t guest_size;              /* Guest memory size */
} RosettaThreadState;

/* Alias for ThreadState (used in syscall module) */
typedef RosettaThreadState ThreadState;

/* ============================================================================
 * Memory Protection Flags
 * ============================================================================ */

#ifndef PROT_NONE
#define PROT_NONE   0x0
#endif
#ifndef PROT_READ
#define PROT_READ   0x1
#endif
#ifndef PROT_WRITE
#define PROT_WRITE  0x2
#endif
#ifndef PROT_EXEC
#define PROT_EXEC   0x4
#endif

/* ============================================================================
 * Syscall Numbers (ARM64 Linux)
 * ============================================================================ */

#define __ARM64_NR_read         63
#define __ARM64_NR_write        64
#define __ARM64_NR_open        1024   /* Actually openat */
#define __ARM64_NR_close        57
#define __ARM64_NR_mmap        222
#define __ARM64_NR_munmap      215
#define __ARM64_NR_exit        93
#define __ARM64_NR_exit_group  94

/* ============================================================================
 * Error Codes
 * ============================================================================ */

#define ROSETTA_OK              0
#define ROSETTA_ERR_NOMEM      -1
#define ROSETTA_ERR_INVAL      -2
#define ROSETTA_ERR_NOTIMPL    -3
#define ROSETTA_ERR_FAULT      -4
#define ROSETTA_ERR_SYSCALL    -5

/* ============================================================================
 * Macro Utilities
 * ============================================================================ */

#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define ALIGN_UP(x, align)      (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align)    ((x) & ~((align) - 1))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

/* Bit field extraction */
#define BITS(val, hi, lo)       (((val) >> (lo)) & ((1U << ((hi) - (lo) + 1)) - 1))
#define BIT(val, bit)           (((val) >> (bit)) & 1)

/* Sign extension - cast to u64 before shifting to avoid shift count overflow */
#define SIGN_EXT(val, bits)     (((s64)((u64)(val) << (64 - (bits)))) >> (64 - (bits)))

#endif /* ROSETTA_TYPES_H */
