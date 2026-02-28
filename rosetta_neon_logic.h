/* ============================================================================
 * Rosetta Refactored - NEON Logical Operations Header
 * ============================================================================
 *
 * This header declares functions for NEON/SIMD logical instruction
 * translation.
 * ============================================================================ */

#ifndef ROSETTA_NEON_LOGIC_H
#define ROSETTA_NEON_LOGIC_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"
#include <stdint.h>

/* ============================================================================
 * Vector Logical Operations
 * ============================================================================ */

/**
 * translate_neon_and - Translate ARM64 AND (vector) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_and(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_orr - Translate ARM64 ORR (vector) instruction
 */
int translate_neon_orr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_eor - Translate ARM64 EOR (vector) instruction
 */
int translate_neon_eor(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_bic - Translate ARM64 BIC (vector) instruction
 * BIC Vd.16B, Vn.16B, Vm.16B  ; Vd = Vn AND NOT Vm
 */
int translate_neon_bic(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_orn - Translate ARM64 ORN (vector) instruction
 * ORN Vd.16B, Vn.16B, Vm.16B  ; Vd = Vn OR NOT Vm
 */
int translate_neon_orn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_andn - Translate ARM64 ANDN (vector) instruction
 * ANDN Vd.16B, Vn.16B, Vm.16B  ; Vd = NOT Vn AND Vm
 */
int translate_neon_andn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_bics - Translate ARM64 BICS (vector, sets flags)
 */
int translate_neon_bics(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Negate/Complement
 * ============================================================================ */

/**
 * translate_neon_mvn - Translate ARM64 MVN (vector) instruction
 */
int translate_neon_mvn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_not - Translate ARM64 NOT (vector) instruction
 */
int translate_neon_not(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Bitwise Select
 * ============================================================================ */

/**
 * translate_neon_bsl - Translate ARM64 BSL (bitwise select) instruction
 * BSL Vd.16B, Vn.16B, Vm.16B  ; Vd = (Vn AND Vd) OR (Vm AND NOT Vd)
 */
int translate_neon_bsl(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_bif - Translate ARM64 BIF (bitwise insert if false) instruction
 */
int translate_neon_bif(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_bit - Translate ARM64 BIT (bitwise insert if true) instruction
 */
int translate_neon_bit(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

#endif /* ROSETTA_NEON_LOGIC_H */
