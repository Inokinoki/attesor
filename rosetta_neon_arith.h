/* ============================================================================
 * Rosetta Refactored - NEON Arithmetic Operations Header
 * ============================================================================
 *
 * This header declares functions for NEON/SIMD arithmetic instruction
 * translation.
 * ============================================================================ */

#ifndef ROSETTA_NEON_ARITH_H
#define ROSETTA_NEON_ARITH_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"
#include <stdint.h>

/* ============================================================================
 * Vector Integer Arithmetic
 * ============================================================================ */

/**
 * translate_neon_add - Translate ARM64 ADD (vector) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sub - Translate ARM64 SUB (vector) instruction
 */
int translate_neon_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Integer Multiplication
 * ============================================================================ */

/**
 * translate_neon_mul - Translate ARM64 MUL (vector) instruction
 */
int translate_neon_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mla - Translate ARM64 MLA (vector) instruction
 * MLA Vd.<T>, Vn.<T>, Vm.<T>  ; Vd = Vd + Vn * Vm
 */
int translate_neon_mla(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_mls - Translate ARM64 MLS (vector) instruction
 * MLS Vd.<T>, Vn.<T>, Vm.<T>  ; Vd = Vd - Vn * Vm
 */
int translate_neon_mls(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Widening Multiplication
 * ============================================================================ */

/**
 * translate_neon_smull - Translate ARM64 SMULL (signed widening multiply long)
 */
int translate_neon_smull(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_umull - Translate ARM64 UMULL (unsigned widening multiply long)
 */
int translate_neon_umull(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_smulh - Translate ARM64 SMULH (signed multiply high)
 */
int translate_neon_smulh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_umulh - Translate ARM64 UMULH (unsigned multiply high)
 */
int translate_neon_umulh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Saturating/Add Long Operations
 * ============================================================================ */

/**
 * translate_neon_saddl - Translate ARM64 SADDL (signed add long)
 */
int translate_neon_saddl(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sadalp - Translate ARM64 SADALP (signed saturating add long pairwise)
 */
int translate_neon_sadalp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

#endif /* ROSETTA_NEON_ARITH_H */
