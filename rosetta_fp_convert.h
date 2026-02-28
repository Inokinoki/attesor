/* ============================================================================
 * Rosetta Refactored - FP Convert Operations Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 floating-point conversion
 * instruction translation to x86_64.
 *
 * Includes: SCVTF, UCVTF, FCVTNS, FCVTNU, FCVTZS, FCVTZU, FMOV, etc.
 * ============================================================================ */

#ifndef ROSETTA_FP_CONVERT_H
#define ROSETTA_FP_CONVERT_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include <stdint.h>

/* ============================================================================
 * FP Convert - Integer to Float
 * ============================================================================ */

/**
 * translate_fp_scvtf - Translate ARM64 SCVTF (signed int to float)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * Returns: 0 on success, -1 on failure
 */
int translate_fp_scvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_ucvtf - Translate ARM64 UCVTF (unsigned int to float)
 */
int translate_fp_ucvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Convert - Float to Integer
 * ============================================================================ */

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
 * translate_fp_fcvtn - Translate ARM64 FCVTN (narrow float to half)
 */
int translate_fp_fcvtn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_fcvtxn - Translate ARM64 FCVTXN (narrow with exponent boost)
 */
int translate_fp_fcvtxn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Convert - Float to Float
 * ============================================================================ */

/**
 * translate_fp_cvtds - Translate ARM64 FCVTD (double to single)
 */
int translate_fp_cvtds(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_cvtsd - Translate ARM64 FCVTS (single to double)
 */
int translate_fp_cvtsd(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_cvthf - Translate ARM64 FCVT (half to float)
 */
int translate_fp_cvthf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_cvtfh - Translate ARM64 FCVT (float to half)
 */
int translate_fp_cvtfh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/* ============================================================================
 * FP Move Operations
 * ============================================================================ */

/**
 * translate_fp_mov - Translate ARM64 FMOV (FP register move or immediate)
 */
int translate_fp_mov(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs);

/**
 * translate_fp_mov_gpr - Translate ARM64 FMOV (GPR <-> FP register)
 */
int translate_fp_mov_gpr(uint32_t encoding, code_buf_t *code_buf,
                         Vector128 *vec_regs, uint64_t *x_regs);

/* ============================================================================
 * FP Convert Dispatch
 * ============================================================================ */

/**
 * translate_fp_convert_dispatch - Dispatch FP convert instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state
 * Returns: 0 on success, -1 if not an FP convert instruction
 */
int translate_fp_convert_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                  Vector128 *vec_regs, uint64_t *x_regs);

#endif /* ROSETTA_FP_CONVERT_H */
