/* ============================================================================
 * Rosetta Refactored - NEON Compare Operations Header
 * ============================================================================
 *
 * This header declares functions for NEON/SIMD compare instruction
 * translation.
 *
 * Supported instructions:
 * - CMGE, CMGT (signed greater than/equal, greater than)
 * - CMLE, CMLT (signed less than/equal, less than)
 * - CMEQ (equal compare)
 * - CMHI, CMHS (unsigned compare)
 * - FCMEQ, FCMGE, FCMGT (floating point compare)
 * ============================================================================ */

#ifndef ROSETTA_NEON_COMPARE_H
#define ROSETTA_NEON_COMPARE_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"
#include <stdint.h>

/* ============================================================================
 * Vector Integer Compare Operations
 * ============================================================================ */

/**
 * translate_neon_cmge - Translate ARM64 CMGE (vector) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_cmge(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmgt - Translate ARM64 CMGT (signed greater than compare)
 * CMGT Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmgt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmeq - Translate ARM64 CMEQ (vector) instruction
 * CMEQ Vd.<T>, Vn.<T>, Vm.<T>  ; Equal compare
 */
int translate_neon_cmeq(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmle - Translate ARM64 CMLE (signed <= compare)
 * CMLE Vd.<T>, Vn.<T>, #0  ; Compare with zero
 */
int translate_neon_cmle(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmlt - Translate ARM64 CMLT (signed < compare)
 * CMLT Vd.<T>, Vn.<T>, #0  ; Compare less than zero
 */
int translate_neon_cmlt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmhi - Translate ARM64 CMHI (unsigned higher compare)
 * CMHI Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmhi(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmhs - Translate ARM64 CMHS (unsigned higher or same compare)
 * CMHS Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmhs(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Vector Compare with Zero
 * ============================================================================ */

/**
 * translate_neon_cmgez - Translate ARM64 CMGEZ (compare >= zero)
 * CMGE Vd.<T>, Vn.<T>, #0
 */
int translate_neon_cmgez(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_cmeq_zero - Translate ARM64 CMEQ with zero
 * CMEQ Vd.<T>, Vn.<T>, #0
 */
int translate_neon_cmeq_zero(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * Floating Point Compare (Vector)
 * ============================================================================ */

/**
 * translate_neon_fcmge - Translate ARM64 FCMGE (FP compare greater than or equal)
 * FCMGE Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmge(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fcmgt - Translate ARM64 FCMGT (FP compare greater than)
 * FCMGT Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmgt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_neon_fcmeq - Translate ARM64 FCMEQ (FP compare equal)
 * FCMEQ Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmeq(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

#endif /* ROSETTA_NEON_COMPARE_H */
