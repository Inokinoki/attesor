/* ============================================================================
 * Rosetta Refactored - NEON Shift Operations Header
 * ============================================================================
 *
 * This header declares functions for NEON/SIMD shift instruction
 * translation.
 * ============================================================================ */

#ifndef ROSETTA_NEON_SHIFT_H
#define ROSETTA_NEON_SHIFT_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"
#include <stdint.h>

/* ============================================================================
 * Vector Shift Left Operations
 * ============================================================================ */

/**
 * translate_neon_shl - Translate ARM64 SHL (vector) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_shl(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sli - Translate ARM64 SLI (shift left insert) instruction
 * SLI Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_sli(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Shift Right Operations
 * ============================================================================ */

/**
 * translate_neon_shr - Translate ARM64 SHR/USHR (vector) instruction
 * USHR Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_shr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sshr - Translate ARM64 SSHR (arithmetic shift right) instruction
 * SSHR Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_sshr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_sri - Translate ARM64 SRI (shift right insert) instruction
 * SRI Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_sri(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Shift Right Narrow (with rounding)
 * ============================================================================ */

/**
 * translate_neon_shrn - Translate ARM64 SHRN (shift right narrow) instruction
 * SHRN Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_shrn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_rshrn - Translate ARM64 RSHRN (rounding shift right narrow) instruction
 * RSHRN Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_rshrn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Shift Left Long
 * ============================================================================ */

/**
 * translate_neon_shll - Translate ARM64 SHLL (shift left long) instruction
 * SHLL Vd.<T>, Vn.<T>, #imm
 */
int translate_neon_shll(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Extract
 * ============================================================================ */

/**
 * translate_neon_ext - Translate ARM64 EXT (vector) instruction
 * EXT Vd.16B, Vn.16B, Vm.16B, #imm
 */
int translate_neon_ext(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

#endif /* ROSETTA_NEON_SHIFT_H */
