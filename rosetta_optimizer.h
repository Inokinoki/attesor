/* ============================================================================
 * Rosetta Binary Translator - Optimizer Module Header
 * ============================================================================
 *
 * This module provides optimization wrappers for the translation layer,
 * including inline caching, fast paths, and statistics-integrated execution.
 * ============================================================================ */

#ifndef ROSETTA_OPTIMIZER_H
#define ROSETTA_OPTIMIZER_H

#include "rosetta_refactored_types.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Inline Translation Cache
 * ============================================================================ */

/**
 * Translate block with inline cache lookup
 * @guest_pc: Starting program counter
 * Returns: Pointer to translated code, or NULL on error
 *
 * This is an optimized version that inlines the cache lookup
 * for the fast path.
 */
void *translate_block_inline(uint64_t guest_pc);

/**
 * Translate and execute in one step
 * @guest_pc: Starting program counter
 * Returns: 0 on success, -1 on error
 *
 * Convenience function for single-block execution.
 */
int translate_and_execute(uint64_t guest_pc);

/* ============================================================================
 * Statistics-Integrated Translation
 * ============================================================================ */

/**
 * Translate block with statistics tracking
 * @guest_pc: Starting program counter
 * @record_stats: Whether to record statistics
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block_with_stats(uint64_t guest_pc, bool record_stats);

/**
 * Execute translated code with statistics
 * @host_code: Pointer to translated code
 * @insn_count: Number of instructions in block
 */
void execute_with_stats(void *host_code, int insn_count);

/* ============================================================================
 * Hot/Cold Path Optimization
 * ============================================================================ */

/**
 * Check if a block is considered "hot" (frequently executed)
 * @guest_pc: Program counter to check
 * Returns: 1 if hot, 0 otherwise
 */
int is_block_hot(uint64_t guest_pc);

/**
 * Get hot block threshold
 * Returns: Number of executions to be considered hot
 */
uint32_t get_hot_block_threshold(void);

/**
 * Set hot block threshold
 * @threshold: New threshold value
 */
void set_hot_block_threshold(uint32_t threshold);

/* ============================================================================
 * Translation Hints
 * ============================================================================ */

#define ROS_OPT_HINT_NONE       0x00
#define ROS_OPT_HINT_FAST_PATH  0x01
#define ROS_OPT_HINT_COLD       0x02
#define ROS_OPT_HINT_HOT        0x04
#define ROS_OPT_HINT_LOOP       0x08
#define ROS_OPT_HINT_TAIL_CALL  0x10

/**
 * Translate block with optimization hints
 * @guest_pc: Starting program counter
 * @hints: Optimization hints bitmask
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block_with_hints(uint64_t guest_pc, uint32_t hints);

/**
 * Mark a block for preferential treatment
 * @guest_pc: Program counter to mark
 * @priority: Priority level (0-3, higher = more important)
 */
void mark_block_priority(uint64_t guest_pc, uint32_t priority);

#endif /* ROSETTA_OPTIMIZER_H */

