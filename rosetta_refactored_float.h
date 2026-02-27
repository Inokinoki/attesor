/* ============================================================================
 * Rosetta Refactored - Floating-Point Instruction Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 floating-point instruction
 * translation to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_FLOAT_H
#define ROSETTA_REFACTORED_FLOAT_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include <stdint.h>

/* Use Vector128 for FP registers (shared with NEON) */
typedef Vector128 FPRegister;

/* FP types */
typedef enum {
    FP_SINGLE = 0,    /* 32-bit single precision */
    FP_DOUBLE = 1,    /* 64-bit double precision */
    FP_HALF = 2       /* 16-bit half precision */
} FPType;

/* ============================================================================
 * FP Arithmetic Operations
 * ============================================================================ */

/**
 * translate_fp_add - Translate ARM64 FADD
 */
int translate_fp_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_sub - Translate ARM64 FSUB
 */
int translate_fp_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_mul - Translate ARM64 FMUL
 */
int translate_fp_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_div - Translate ARM64 FDIV
 */
int translate_fp_div(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_max - Translate ARM64 FMAX
 */
int translate_fp_max(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_min - Translate ARM64 FMIN
 */
int translate_fp_min(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_sqrt - Translate ARM64 FSQRT
 */
int translate_fp_sqrt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_abs - Translate ARM64 FABS
 */
int translate_fp_abs(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_neg - Translate ARM64 FNEG
 */
int translate_fp_neg(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fma - Translate ARM64 FMADD (fused multiply-add)
 */
int translate_fp_fma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fms - Translate ARM64 FMSUB (fused multiply-subtract)
 */
int translate_fp_fms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fnma - Translate ARM64 FNMADD (negated fused multiply-add)
 */
int translate_fp_fnma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fnms - Translate ARM64 FNMSUB (negated fused multiply-subtract)
 */
int translate_fp_fnms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Compare Operations
 * ============================================================================ */

/**
 * translate_fp_cmp - Translate ARM64 FCMP
 */
int translate_fp_cmp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs, uint32_t *pstate);

/**
 * translate_fp_cmpe - Translate ARM64 FCMPE (with exception on NaN)
 */
int translate_fp_cmpe(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                      uint64_t *x_regs, uint32_t *pstate);

/* ============================================================================
 * FP Convert Operations
 * ============================================================================ */

/**
 * translate_fp_scvtf - Translate ARM64 SCVTF (signed int to float)
 */
int translate_fp_scvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_ucvtf - Translate ARM64 UCVTF (unsigned int to float)
 */
int translate_fp_ucvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtns - Translate ARM64 FCVTNS (float to signed int, nearest)
 */
int translate_fp_fcvtns(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtnu - Translate ARM64 FCVTNU (float to unsigned int, nearest)
 */
int translate_fp_fcvtnu(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtps - Translate ARM64 FCVTPS (float to signed int, positive)
 */
int translate_fp_fcvtps(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtpu - Translate ARM64 FCVTPU (float to unsigned int, positive)
 */
int translate_fp_fcvtpu(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtn - Translate ARM64 FCVTN (narrow float)
 */
int translate_fp_fcvtn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtxn - Translate ARM64 FCVTXN (narrow with exponent boost)
 */
int translate_fp_fcvtxn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Move Operations
 * ============================================================================ */

/**
 * translate_fp_mov - Translate ARM64 FMOV (register or immediate)
 */
int translate_fp_mov(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_mov_gpr - Translate ARM64 FMOV (GPR to FP or FP to GPR)
 */
int translate_fp_mov_gpr(uint32_t encoding, code_buf_t *code_buf,
                         Vector128 *vec_regs, uint64_t *x_regs);

/* ============================================================================
 * FP Load/Store Operations
 * ============================================================================ */

/**
 * translate_fp_ldr - Translate ARM64 LDR (vector)
 */
int translate_fp_ldr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs);

/**
 * translate_fp_str - Translate ARM64 STR (vector)
 */
int translate_fp_str(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs);

/**
 * translate_fp_ldur - Translate ARM64 LDUR (vector, unscaled)
 */
int translate_fp_ldur(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                      uint64_t *x_regs);

/**
 * translate_fp_stur - Translate ARM64 STUR (vector, unscaled)
 */
int translate_fp_stur(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                      uint64_t *x_regs);

/**
 * translate_fp_ldp - Translate ARM64 LDP (FP, load pair)
 */
int translate_fp_ldp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs);

/**
 * translate_fp_stp - Translate ARM64 STP (FP, store pair)
 */
int translate_fp_stp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs);

/* ============================================================================
 * FP Round/Special Operations
 * ============================================================================ */

/**
 * translate_fp_rint - Translate ARM64 FRINT (round to integer)
 */
int translate_fp_rint(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rinta - Translate ARM64 FRINTA (round to odd)
 */
int translate_fp_rinta(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rintn - Translate ARM64 FRINTN (round to nearest even)
 */
int translate_fp_rintn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rintp - Translate ARM64 FRINTP (round to positive)
 */
int translate_fp_rintp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rintm - Translate ARM64 FRINTM (round to negative)
 */
int translate_fp_rintm(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rintz - Translate ARM64 FRINTZ (round to zero)
 */
int translate_fp_rintz(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_rintx - Translate ARM64 FRINTX (round to exact integer)
 */
int translate_fp_rintx(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Conditional Select
 * ============================================================================ */

/**
 * translate_fp_fcsel - Translate ARM64 FCSEL (floating-point conditional select)
 */
int translate_fp_fcsel(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint32_t *pstate);

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * translate_fp_dispatch - Dispatch FP instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state
 * @pstate: Processor state (NZCV flags)
 * Returns: 0 on success, -1 if not a FP instruction
 */
int translate_fp_dispatch(uint32_t encoding, code_buf_t *code_buf,
                          Vector128 *vec_regs, uint64_t *x_regs, uint32_t *pstate);

#endif /* ROSETTA_REFACTORED_FLOAT_H */
