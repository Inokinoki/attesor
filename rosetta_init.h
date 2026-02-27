/* ============================================================================
 * Rosetta Initialization Header
 * ============================================================================
 *
 * This module provides initialization and teardown functions for the
 * Rosetta binary translation environment.
 * ============================================================================ */

#ifndef ROSETTA_INIT_H
#define ROSETTA_INIT_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

/**
 * Initialize the translation environment
 * @param entry_callback Output: callback for translated entry point
 */
void init_translation_env(void **entry_callback);

/**
 * Initialize translation cache
 */
void init_translation_cache(void);

/**
 * Initialize JIT compiler
 * @return 0 on success, -1 on failure
 */
int init_jit_compiler(void);

/**
 * Initialize memory mappings for guest code
 * @return 0 on success, -1 on failure
 */
int init_memory_mappings(void);

/**
 * Clean up translation environment
 */
void cleanup_translation_env(void);

/* ============================================================================
 * FP Register Access
 * ============================================================================ */

/**
 * Read FP Control Register
 * @return FPCR value
 */
uint32_t read_fpcr(void);

/**
 * Write FP Control Register
 * @param val Value to write
 */
void write_fpcr(uint32_t val);

/**
 * Read FP Status Register
 * @return FPSR value
 */
uint32_t read_fpsr(void);

/**
 * Write FP Status Register
 * @param val Value to write
 */
void write_fpsr(uint32_t val);

/**
 * Set FP registers from values
 * @param fpcr_value FPCR value
 * @param fpsr_value FPSR value
 */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value);

/**
 * Clear FP registers to default values
 */
void clear_fp_registers(void);

/* ============================================================================
 * Rosetta Entry Point
 * ============================================================================ */

/**
 * Rosetta entry point
 * Called when translated code starts execution
 */
void rosetta_entry(void);

#endif /* ROSETTA_INIT_H */
