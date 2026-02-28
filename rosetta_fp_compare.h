/* ============================================================================
 * Rosetta Refactored - FP Compare Operations Header
 * ============================================================================
 *
 * This header defines the interface for ARM64 floating-point compare
 * instruction translation to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_FP_COMPARE_H
#define ROSETTA_FP_COMPARE_H

#include "rosetta_refactored.h"
#include "rosetta_emit_x86.h"
#include <stdint.h>

/* ============================================================================
 * FP Compare Operations
 * ============================================================================ */

/**
 * translate_fp_cmp - Translate ARM64 FCMP (floating-point compare)
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state (for result)
 * @pstate: Processor state (NZCV flags output)
 * Returns: 0 on success, -1 on failure
 */
int translate_fp_cmp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs, uint32_t *pstate);

/**
 * translate_fp_cmpe - Translate ARM64 FCMPE (compare with exceptions)
 */
int translate_fp_cmpe(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                      uint64_t *x_regs, uint32_t *pstate);

/**
 * translate_fp_cmp_zero - Translate ARM64 FCMP with zero immediate
 */
int translate_fp_cmp_zero(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                          uint32_t *pstate);

/* ============================================================================
 * FP Compare Dispatch
 * ============================================================================ */

/**
 * translate_fp_compare_dispatch - Dispatch FP compare instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state
 * @pstate: Processor state
 * Returns: 0 on success, -1 if not an FP compare instruction
 */
int translate_fp_compare_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                  Vector128 *vec_regs, uint64_t *x_regs, uint32_t *pstate);

#endif /* ROSETTA_FP_COMPARE_H */
