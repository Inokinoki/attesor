/* ============================================================================
 * Rosetta Refactored - Floating-Point Utilities Header
 * ============================================================================
 *
 * This header declares floating-point utility functions for the
 * Rosetta translation layer.
 *
 * Functions include:
 * - FP control register access (FPCR, FPSR)
 * - FP estimates (reciprocal, rsqrt)
 * - FP state management
 * - Vector register operations
 * ============================================================================ */

#ifndef ROSETTA_FP_UTILS_H
#define ROSETTA_FP_UTILS_H

#include "rosetta_types.h"
#include <stdint.h>

/* ============================================================================
 * FP Control Register Access
 * ============================================================================ */

/**
 * read_fpcr - Read FP Control Register
 * Returns: Current FPCR value
 */
u32 read_fpcr(void);

/**
 * write_fpcr - Write FP Control Register
 * @val: New FPCR value
 */
void write_fpcr(u32 val);

/**
 * read_fpsr - Read FP Status Register
 * Returns: Current FPSR value
 */
u32 read_fpsr(void);

/**
 * write_fpsr - Write FP Status Register
 * @val: New FPSR value
 */
void write_fpsr(u32 val);

/* ============================================================================
 * FP State Management
 * ============================================================================ */

/**
 * set_fp_registers - Set FP control and status registers
 * @fpcr_val: New FPCR value
 * @fpsr_val: New FPSR value
 */
void set_fp_registers(u64 fpcr_val, u64 fpsr_val);

/**
 * clear_fp_registers - Clear FP registers to zero
 * Returns: Zero vector
 */
vec128_t clear_fp_registers(void);

/**
 * fp_noop - FP no-operation
 * Returns: Zero vector
 */
vec128_t fp_noop(void);

/**
 * init_fp_state - Initialize FP state
 * Returns: Initialized FP state vector
 */
vec128_t init_fp_state(void);

/**
 * init_fp_context - Initialize FP context
 * @ctx: Vector context to initialize
 */
void init_fp_context(vec128_t *ctx);

/* ============================================================================
 * FP Estimates (ARM64 FRECPE, FRSQRTE emulation)
 * ============================================================================ */

/**
 * fp_recip_estimate - Floating-point reciprocal estimate
 * @x: Input value (FP format)
 * Returns: Estimated 1/x
 */
u32 fp_recip_estimate(u32 x);

/**
 * fp_rsqrt_estimate - Floating-point reciprocal square root estimate
 * @x: Input value (FP format)
 * Returns: Estimated 1/sqrt(x)
 */
u32 fp_rsqrt_estimate(u32 x);

/* ============================================================================
 * Vector Register Operations
 * ============================================================================ */

/**
 * load_vector_reg - Load vector register from memory
 * @src: Source pointer
 * Returns: Loaded vector
 */
vec128_t load_vector_reg(const vec128_t *src);

/**
 * store_vector_reg - Store vector register to memory
 * @dest: Destination pointer
 * @v: Vector to store
 */
void store_vector_reg(vec128_t *dest, vec128_t v);

#endif /* ROSETTA_FP_UTILS_H */
