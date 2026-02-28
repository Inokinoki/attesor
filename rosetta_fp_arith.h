/* ============================================================================
 * Rosetta Refactored - FP Arithmetic Operations Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 floating-point arithmetic
 * instruction translation to x86_64.
 *
 * Includes: FADD, FSUB, FMUL, FDIV, FMAX, FMIN, FSQRT, FABS, FNEG, etc.
 * ============================================================================ */

#ifndef ROSETTA_FP_ARITH_H
#define ROSETTA_FP_ARITH_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include <stdint.h>

/* ============================================================================
 * FP Arithmetic Operations - Basic
 * ============================================================================ */

/**
 * translate_fp_add - Translate ARM64 FADD (floating-point add)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_fp_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_sub - Translate ARM64 FSUB (floating-point subtract)
 */
int translate_fp_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_mul - Translate ARM64 FMUL (floating-point multiply)
 */
int translate_fp_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_div - Translate ARM64 FDIV (floating-point divide)
 */
int translate_fp_div(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_sqrt - Translate ARM64 FSQRT (floating-point square root)
 */
int translate_fp_sqrt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Arithmetic Operations - Min/Max
 * ============================================================================ */

/**
 * translate_fp_max - Translate ARM64 FMAX (floating-point maximum)
 */
int translate_fp_max(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_min - Translate ARM64 FMIN (floating-point minimum)
 */
int translate_fp_min(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_maxnm - Translate ARM64 FMAXNM (maximum number)
 */
int translate_fp_maxnm(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_minnm - Translate ARM64 FMINNM (minimum number)
 */
int translate_fp_minnm(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Arithmetic Operations - Unary
 * ============================================================================ */

/**
 * translate_fp_abs - Translate ARM64 FABS (floating-point absolute)
 */
int translate_fp_abs(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_neg - Translate ARM64 FNEG (floating-point negate)
 */
int translate_fp_neg(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Arithmetic Operations - Fused Multiply-Add
 * ============================================================================ */

/**
 * translate_fp_fma - Translate ARM64 FMADD (fused multiply-add)
 * FMADD Vd, Vn, Vm, Va  => Vd = Va + (Vn * Vm)
 */
int translate_fp_fma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fms - Translate ARM64 FMSUB (fused multiply-subtract)
 * FMSUB Vd, Vn, Vm, Va  => Vd = Va - (Vn * Vm)
 */
int translate_fp_fms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fnma - Translate ARM64 FNMADD (negated fused multiply-add)
 * FNMADD Vd, Vn, Vm, Va  => Vd = -(Va + (Vn * Vm))
 */
int translate_fp_fnma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fnms - Translate ARM64 FNMSUB (negated fused multiply-subtract)
 * FNMSUB Vd, Vn, Vm, Va  => Vd = -(Va - (Vn * Vm))
 */
int translate_fp_fnms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Arithmetic Dispatch
 * ============================================================================ */

/**
 * translate_fp_arith_dispatch - Dispatch FP arithmetic instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 if not an FP arithmetic instruction
 */
int translate_fp_arith_dispatch(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

#endif /* ROSETTA_FP_ARITH_H */
