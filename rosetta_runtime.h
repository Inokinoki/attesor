/* ============================================================================
 * Rosetta Translator - Runtime and Entry Point
 * ============================================================================
 *
 * This module provides the runtime environment including:
 * - Entry point initialization
 * - FP register management
 * - CPU context save/restore
 * ============================================================================ */

#ifndef ROSETTA_RUNTIME_H
#define ROSETTA_RUNTIME_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Rosetta Entry Point
 * ============================================================================ */

/**
 * Rosetta Entry Point
 *
 * This is the main entry point for the Rosetta translator. It:
 * 1. Parses command-line arguments from the stack
 * 2. Initializes the translation environment
 * 3. Sets up the CPU context
 * 4. Transfers control to the translated code
 */
void rosetta_entry(void);

/* ============================================================================
 * FP Context Management
 * ============================================================================ */

/**
 * Load vector register from memory
 *
 * @param ctx Pointer to 16-byte register data
 * @return Vector128 value
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

/* ============================================================================
 * CPU Context Save/Restore
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

#endif /* ROSETTA_RUNTIME_H */
