/* ============================================================================
 * Rosetta Translator - Core Type Definitions
 * ============================================================================
 *
 * This file defines the core data structures used throughout the Rosetta
 * binary translation layer. It builds on rosetta_refactored.h with additional
 * types needed for the refactored modules.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_TYPES_H
#define ROSETTA_REFACTORED_TYPES_H

/* Include rosetta_refactored.h which defines all common types */
#include "rosetta_refactored.h"

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
 * ARM64 Condition Codes - only if not already defined in rosetta_types.h
 * ============================================================================ */

#ifndef ROSETTA_TYPES_H
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
#endif

#endif /* ROSETTA_REFACTORED_TYPES_H */
