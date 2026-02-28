/* ============================================================================
 * Rosetta Refactored - NEON/SIMD Instruction Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 NEON/SIMD instruction
 * translation to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_NEON_H
#define ROSETTA_REFACTORED_NEON_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include "rosetta_neon_arith.h"
#include "rosetta_neon_logic.h"
#include "rosetta_neon_shift.h"
#include "rosetta_neon_compare.h"
#include "rosetta_neon_loadstore.h"
#include <stdint.h>

/* ============================================================================
 * Vector Register Types
 * ============================================================================ */

/* Vector128 is defined in rosetta_refactored.h */

/* Vector element sizes */
typedef enum {
    VECTOR_8B  = 0,   /* 8 x 8-bit elements */
    VECTOR_16B = 0,   /* 16 x 8-bit elements (Q) */
    VECTOR_4H  = 1,   /* 4 x 16-bit elements */
    VECTOR_8H  = 1,   /* 8 x 16-bit elements (Q) */
    VECTOR_2S  = 2,   /* 2 x 32-bit elements */
    VECTOR_4S  = 2,   /* 4 x 32-bit elements (Q) */
    VECTOR_1D  = 3,   /* 1 x 64-bit element */
    VECTOR_2D  = 3    /* 2 x 64-bit elements (Q) */
} VectorSize;

/* ============================================================================
 * Vector Arithmetic - Integer
 * ============================================================================ */

/**
 * translate_neon_add - Translate ARM64 ADD (vector)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sub - Translate ARM64 SUB (vector)
 */
int translate_neon_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mul - Translate ARM64 MUL (vector)
 */
int translate_neon_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mla - Translate ARM64 MLA (vector) - Multiply Accumulate
 */
int translate_neon_mla(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mls - Translate ARM64 MLS (vector) - Multiply Subtract
 */
int translate_neon_mls(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_padd - Translate ARM64 ADDP (pairwise add)
 */
int translate_neon_padd(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Logical Operations
 * ============================================================================ */

/**
 * translate_neon_and - Translate ARM64 AND (vector)
 */
int translate_neon_and(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_orr - Translate ARM64 ORR (vector)
 */
int translate_neon_orr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_eor - Translate ARM64 EOR (vector)
 */
int translate_neon_eor(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_bic - Translate ARM64 BIC (vector)
 */
int translate_neon_bic(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_orn - Translate ARM64 ORN (vector) - OR NOT
 */
int translate_neon_orn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Shift Operations
 * ============================================================================ */

/**
 * translate_neon_shl - Translate ARM64 SHL (shift left)
 */
int translate_neon_shl(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_shr - Translate ARM64 SHR/USHR (logical shift right)
 */
int translate_neon_shr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sshr - Translate ARM64 SSHR (arithmetic shift right)
 */
int translate_neon_sshr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sli - Translate ARM64 SLI (shift left insert)
 */
int translate_neon_sli(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sri - Translate ARM64 SRI (shift right insert)
 */
int translate_neon_sri(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Compare Operations
 * ============================================================================ */

/**
 * translate_neon_cmge - Translate ARM64 CMGE (signed >= compare)
 */
int translate_neon_cmge(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmgt - Translate ARM64 CMGT (signed > compare)
 */
int translate_neon_cmgt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmeq - Translate ARM64 CMEQ (equal compare)
 */
int translate_neon_cmeq(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmle - Translate ARM64 CMLE (signed <= compare)
 */
int translate_neon_cmle(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmlt - Translate ARM64 CMLT (signed < compare)
 */
int translate_neon_cmlt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Move/Duplicate Operations
 * ============================================================================ */

/**
 * translate_neon_dup - Translate ARM64 DUP (duplicate element)
 */
int translate_neon_dup(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mov - Translate ARM64 MOV (vector register move)
 */
int translate_neon_mov(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mov_element - Translate ARM64 MOV (element to GPR)
 */
int translate_neon_mov_element(uint32_t encoding, code_buf_t *code_buf,
                               Vector128 *vec_regs, uint64_t *x_regs);

/* ============================================================================
 * Vector Extract/Permute Operations
 * ============================================================================ */

/**
 * translate_neon_ext - Translate ARM64 EXT (extract)
 */
int translate_neon_ext(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_rev64 - Translate ARM64 REV64 (reverse elements)
 */
int translate_neon_rev64(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_rev32 - Translate ARM64 REV32 (reverse elements)
 */
int translate_neon_rev32(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_rev16 - Translate ARM64 REV16 (reverse elements)
 */
int translate_neon_rev16(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Load/Store Operations
 * ============================================================================ */

/**
 * translate_neon_ld1 - Translate ARM64 LD1 (load single structure)
 */
int translate_neon_ld1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st1 - Translate ARM64 ST1 (store single structure)
 */
int translate_neon_st1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld2 - Translate ARM64 LD2 (load two structures)
 */
int translate_neon_ld2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st2 - Translate ARM64 ST2 (store two structures)
 */
int translate_neon_st2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ldr - Translate ARM64 LDR (vector) - load vector register
 */
int translate_neon_ldr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/**
 * translate_neon_str - Translate ARM64 STR (vector) - store vector register
 */
int translate_neon_str(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/* ============================================================================
 * Vector Convert Operations
 * ============================================================================ */

/**
 * translate_neon_fcvtns - Translate ARM64 FCVTN/S (float to signed int)
 */
int translate_neon_fcvtns(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fcvtnu - Translate ARM64 FCVTN/U (float to unsigned int)
 */
int translate_neon_fcvtnu(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_scvtf - Translate ARM64 SCVTF (signed int to float)
 */
int translate_neon_scvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_ucvtf - Translate ARM64 UCVTF (unsigned int to float)
 */
int translate_neon_ucvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Floating Point Operations
 * ============================================================================ */

/**
 * translate_neon_fadd - Translate ARM64 FADD (vector)
 */
int translate_neon_fadd(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fsub - Translate ARM64 FSUB (vector)
 */
int translate_neon_fsub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fmul - Translate ARM64 FMUL (vector)
 */
int translate_neon_fmul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fdiv - Translate ARM64 FDIV (vector)
 */
int translate_neon_fdiv(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fmax - Translate ARM64 FMAX (vector)
 */
int translate_neon_fmax(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fmin - Translate ARM64 FMIN (vector)
 */
int translate_neon_fmin(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * NEON Reduction Operations (Helpers)
 * ============================================================================ */

/**
 * has_zero_byte - Check if any byte in a 64-bit value is zero
 * @x: 64-bit value to check
 * Returns: Non-zero if any byte is zero, 0 otherwise
 *
 * Uses SWAR technique: (x - 0x01010101...) & ~x & 0x80808080...
 */
uint64_t has_zero_byte(uint64_t x);

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * translate_neon_dispatch - Dispatch NEON instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state
 * Returns: 0 on success, -1 if not a NEON instruction
 */
int translate_neon_dispatch(uint32_t encoding, code_buf_t *code_buf,
                            Vector128 *vec_regs, uint64_t *x_regs);

#endif /* ROSETTA_REFACTORED_NEON_H */
