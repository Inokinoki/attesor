/* ============================================================================
 * Rosetta CPU Context Module
 * ============================================================================
 *
 * This module handles CPU context save/restore operations including
 * general purpose registers, vector registers, and FP control registers.
 * ============================================================================ */

#ifndef ROSETTA_CONTEXT_H
#define ROSETTA_CONTEXT_H

#include "rosetta_refactored.h"
#include <stdint.h>

/* ============================================================================
 * Context Save/Restore Functions
 * ============================================================================ */

/**
 * Save CPU context to memory structure
 *
 * Saves all general purpose registers, vector registers,
 * and FP control registers to a contiguous memory area.
 *
 * @param ctx Source CPU context
 * @param save_area Destination memory area (0x210 bytes)
 * @param flags Save flags
 */
void save_cpu_context_full(CPUContext *ctx, void *save_area, long flags);

/**
 * Restore CPU context from memory structure
 *
 * @param ctx Destination CPU context
 * @param save_area Source memory area
 */
void restore_cpu_context_full(CPUContext *ctx, const void *save_area);

/**
 * Save FP context only
 *
 * @param save Array to save FP state (32 x 128-bit registers + fpsr/fpcr)
 */
void save_fp_context(uint64_t *save);

/**
 * Restore FP context only
 *
 * @param save Array containing saved FP state
 */
void restore_fp_context(uint64_t *save);

/* ============================================================================
 * FP Context Management
 * ============================================================================ */

/**
 * Load FP/Vortex registers from memory
 *
 * @param ctx Pointer to 16-byte register data
 * @return Vector128 register value
 */
Vector128 load_vector_register(const Vector128 *ctx);

/**
 * Set FP control and status registers
 *
 * @param fpcr_value New FPCR value
 * @param fpsr_value New FPSR value
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value);

/**
 * Clear FP control and status registers
 */
void clear_fp_registers(void);

/**
 * No-operation FP operation
 * Returns zero vector
 */
Vector128 fp_noop(void);

#endif /* ROSETTA_CONTEXT_H */
