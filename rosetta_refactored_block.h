/* ============================================================================
 * Rosetta Refactored - Block Translation Header
 * ============================================================================
 *
 * This module provides block-level translation functions for the Rosetta
 * translation layer, including basic block detection, translation, and
 * caching.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_BLOCK_H
#define ROSETTA_REFACTORED_BLOCK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "rosetta_types.h"
#include "rosetta_emit_x86.h"

/* ============================================================================
 * Block Configuration
 * ============================================================================ */

/* Maximum instructions per block */
#define ROS_BLOCK_MAX_INSNS     64

/* Maximum block size in bytes (ARM64 instructions are 4 bytes) */
#define ROS_BLOCK_MAX_SIZE      (ROS_BLOCK_MAX_INSNS * 4)

/* Minimum instructions per block */
#define ROS_BLOCK_MIN_INSNS     1

/* Block flags */
#define ROS_BLOCK_FLAG_VALID    0x01
#define ROS_BLOCK_FLAG_CACHED   0x02
#define ROS_BLOCK_FLAG_HOT      0x04
#define ROS_BLOCK_FLAG_BRANCH   0x08
#define ROS_BLOCK_FLAG_SYSCALL  0x10

/* ============================================================================
 * Block Descriptor
 * ============================================================================ */

/* Block translation descriptor */
typedef struct {
    uint64_t guest_pc;          /* Guest PC of block start */
    uint64_t host_pc;           /* Host PC of translated code */
    uint32_t guest_size;        /* Size in ARM64 instructions (bytes) */
    uint32_t host_size;         /* Size of translated x86_64 code (bytes) */
    uint16_t insn_count;        /* Number of instructions */
    uint16_t flags;             /* Block flags */
    uint32_t hash;              /* Block hash for lookup */
    uint32_t refcount;          /* Reference count */
    uint32_t hit_count;         /* Cache hit count */
} rosetta_block_t;

/* ============================================================================
 * Block Translation Functions
 * ============================================================================ */

/**
 * rosetta_block_init - Initialize block translation subsystem
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_init(void);

/**
 * rosetta_block_cleanup - Cleanup block translation subsystem
 */
void rosetta_block_cleanup(void);

/**
 * rosetta_block_translate - Translate a basic block
 * @guest_pc: Guest program counter
 * @max_insns: Maximum instructions to translate
 * Returns: Pointer to translated code, or NULL on error
 */
void *rosetta_block_translate(uint64_t guest_pc, int max_insns);

/**
 * rosetta_block_translate_fast - Fast translation (cache lookup)
 * @guest_pc: Guest program counter
 * Returns: Cached translation or NULL
 */
void *rosetta_block_translate_fast(uint64_t guest_pc);

/**
 * rosetta_block_lookup - Look up translated block
 * @guest_pc: Guest program counter
 * Returns: Block descriptor or NULL
 */
rosetta_block_t *rosetta_block_lookup(uint64_t guest_pc);

/**
 * rosetta_block_insert - Insert translated block into cache
 * @guest_pc: Guest program counter
 * @host_code: Translated code pointer
 * @size: Translated code size
 * @insn_count: Number of instructions
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_insert(uint64_t guest_pc, void *host_code,
                         size_t size, int insn_count);

/**
 * rosetta_block_remove - Remove block from cache
 * @guest_pc: Guest program counter
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_remove(uint64_t guest_pc);

/**
 * rosetta_block_invalidate - Invalidate all cached blocks
 */
void rosetta_block_invalidate(void);

/**
 * rosetta_block_get_info - Get block information
 * @guest_pc: Guest program counter
 * @block: Block descriptor to fill
 * Returns: 0 on success, -1 if not found
 */
int rosetta_block_get_info(uint64_t guest_pc, rosetta_block_t *block);

/**
 * rosetta_block_print - Print block information
 * @guest_pc: Guest program counter
 */
void rosetta_block_print(uint64_t guest_pc);

/* ============================================================================
 * Block Analysis Functions
 * ============================================================================ */

/**
 * rosetta_block_analyze - Analyze basic block boundaries
 * @guest_pc: Guest program counter
 * @max_insns: Maximum instructions to analyze
 * Returns: Number of instructions in block
 */
int rosetta_block_analyze(uint64_t guest_pc, int max_insns);

/**
 * rosetta_block_is_branch - Check if instruction is a branch
 * @encoding: ARM64 instruction encoding
 * Returns: true if branch, false otherwise
 */
bool rosetta_block_is_branch(uint32_t encoding);

/**
 * rosetta_block_is_terminator - Check if instruction terminates block
 * @encoding: ARM64 instruction encoding
 * Returns: true if terminator, false otherwise
 */
bool rosetta_block_is_terminator(uint32_t encoding);

/**
 * rosetta_block_get_branch_target - Get branch target address
 * @encoding: ARM64 instruction encoding
 * @pc: Current program counter
 * Returns: Target address
 */
uint64_t rosetta_block_get_branch_target(uint32_t encoding, uint64_t pc);

/* ============================================================================
 * Block Cache Statistics
 * ============================================================================ */

/**
 * rosetta_block_cache_stats - Get cache statistics
 * @total_blocks: Total blocks cached
 * @total_hits: Total cache hits
 * @total_misses: Total cache misses
 */
void rosetta_block_cache_stats(uint32_t *total_blocks,
                               uint32_t *total_hits,
                               uint32_t *total_misses);

/**
 * rosetta_block_cache_hit_rate - Get cache hit rate
 * Returns: Hit rate as percentage (0.0 - 100.0)
 */
double rosetta_block_cache_hit_rate(void);

/**
 * rosetta_block_cache_count - Get number of cached blocks
 * Returns: Number of blocks in cache
 */
size_t rosetta_block_cache_count(void);

#endif /* ROSETTA_REFACTORED_BLOCK_H */
