/* ============================================================================
 * Rosetta Refactored - Statistics Header
 * ============================================================================
 *
 * This module provides statistics collection and reporting for the
 * Rosetta translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_STATS_H
#define ROSETTA_REFACTORED_STATS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Statistics Types
 * ============================================================================ */

/* Translation statistics */
typedef struct {
    /* Block translation counts */
    uint64_t blocks_translated;
    uint64_t blocks_cached;
    uint64_t blocks_evicted;

    /* Cache statistics */
    uint64_t cache_lookups;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t cache_inserts;
    uint64_t cache_flushes;

    /* Instruction counts by type */
    uint64_t insns_total;
    uint64_t insns_alu;
    uint64_t insns_alu_add;
    uint64_t insns_alu_sub;
    uint64_t insns_alu_and;
    uint64_t insns_alu_orr;
    uint64_t insns_alu_eor;
    uint64_t insns_alu_mvn;
    uint64_t insns_alu_mul;
    uint64_t insns_alu_div;
    uint64_t insns_alu_shift;
    uint64_t insns_mem;
    uint64_t insns_mem_ldr;
    uint64_t insns_mem_str;
    uint64_t insns_mem_ldp;
    uint64_t insns_mem_stp;
    uint64_t insns_branch;
    uint64_t insns_branch_b;
    uint64_t insns_branch_bl;
    uint64_t insns_branch_br;
    uint64_t insns_branch_ret;
    uint64_t insns_branch_cond;
    uint64_t insns_branch_cbz;
    uint64_t insns_branch_cbnz;
    uint64_t insns_mov;
    uint64_t insns_compare;
    uint64_t insns_system;
    uint64_t insns_neon;
    uint64_t insns_unknown;

    /* Code size statistics */
    uint64_t code_size_total;
    uint64_t code_size_arm64;
    uint64_t code_size_x86;
    uint64_t code_size_peak;

    /* Performance counters */
    uint64_t cycles_guest;
    uint64_t cycles_host;
    uint64_t translations_time_us;
    uint64_t execution_time_us;

    /* Error statistics */
    uint64_t errors_translation;
    uint64_t errors_execution;
    uint64_t errors_memory;
} rosetta_stats_t;

/* Per-block statistics */
typedef struct {
    uint64_t guest_pc;
    uint64_t host_pc;
    uint32_t arm64_size;
    uint32_t x86_size;
    uint8_t insn_count;
    uint8_t flags;
    uint16_t hit_count;
} rosetta_block_stats_t;

/* Block statistics flags */
#define ROS_BLOCK_VALID     0x01
#define ROS_BLOCK_CACHED    0x02
#define ROS_BLOCK_HOT       0x04
#define ROS_BLOCK_COLD      0x08

/* Statistics configuration */
#define ROS_STATS_MAX_BLOCKS  1024
#define ROS_STATS_HISTORY_SIZE 256

/* ============================================================================
 * Statistics API
 * ============================================================================ */

/**
 * Initialize statistics subsystem
 * Returns: 0 on success, -1 on error
 */
int rosetta_stats_init(void);

/**
 * Cleanup statistics subsystem
 */
void rosetta_stats_cleanup(void);

/**
 * Reset all statistics
 */
void rosetta_stats_reset(void);

/**
 * Get current statistics
 * @stats: Pointer to statistics structure
 */
void rosetta_stats_get(rosetta_stats_t *stats);

/**
 * Print statistics to stdout
 * @verbose: Print detailed breakdown
 */
void rosetta_stats_print(bool verbose);

/**
 * Export statistics to JSON format
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Bytes written
 */
int rosetta_stats_export_json(char *buf, size_t buf_size);

/* ============================================================================
 * Statistics Recording
 * ============================================================================ */

/**
 * Record block translation
 * @guest_pc: Guest PC
 * @host_pc: Host PC
 * @arm64_size: ARM64 block size
 * @x86_size: x86_64 block size
 * @insn_count: Number of instructions
 */
void rosetta_stats_record_block(uint64_t guest_pc, uint64_t host_pc,
                                 uint32_t arm64_size, uint32_t x86_size,
                                 int insn_count);

/**
 * Record cache hit
 */
void rosetta_stats_record_cache_hit(void);

/**
 * Record cache miss
 */
void rosetta_stats_record_cache_miss(void);

/**
 * Record instruction translation
 * @type: Instruction type (ALU, MEM, BRANCH, etc.)
 */
void rosetta_stats_record_insn(const char *type);

/**
 * Record ALU instruction
 * @subtype: ALU subtype (add, sub, etc.)
 */
void rosetta_stats_record_alu(const char *subtype);

/**
 * Record memory instruction
 * @subtype: Memory subtype (ldr, str, etc.)
 */
void rosetta_stats_record_mem(const char *subtype);

/**
 * Record branch instruction
 * @subtype: Branch subtype (b, bl, ret, etc.)
 */
void rosetta_stats_record_branch(const char *subtype);

/**
 * Record translation error
 * @error_code: Error code
 */
void rosetta_stats_record_error(const char *error_code);

/**
 * Record execution time
 * @time_us: Execution time in microseconds
 */
void rosetta_stats_record_execution_time(uint64_t time_us);

/**
 * Record translation time
 * @time_us: Translation time in microseconds
 */
void rosetta_stats_record_translation_time(uint64_t time_us);

/* ============================================================================
 * Block Statistics
 * ============================================================================ */

/**
 * Get block statistics
 * @index: Block index
 * @stats: Pointer to block statistics structure
 * Returns: 0 on success, -1 on error
 */
int rosetta_stats_get_block(size_t index, rosetta_block_stats_t *stats);

/**
 * Get number of tracked blocks
 * Returns: Number of blocks
 */
size_t rosetta_stats_get_block_count(void);

/**
 * Find hottest blocks
 * @blocks: Output array
 * @count: Number of blocks to find
 * Returns: Number of blocks found
 */
size_t rosetta_stats_get_hot_blocks(rosetta_block_stats_t *blocks, size_t count);

/**
 * Find coldest blocks
 * @blocks: Output array
 * @count: Number of blocks to find
 * Returns: Number of blocks found
 */
size_t rosetta_stats_get_cold_blocks(rosetta_block_stats_t *blocks, size_t count);

/* ============================================================================
 * Histogram and Profiling
 * ============================================================================ */

/* Instruction size histogram buckets */
#define ROS_STATS_HIST_BUCKETS  16

/**
 * Get instruction size histogram
 * @hist: Output histogram array
 * @count: Number of buckets
 */
void rosetta_stats_get_insn_size_histogram(uint32_t *hist, size_t count);

/**
 * Get block size histogram
 * @hist: Output histogram array
 * @count: Number of buckets
 */
void rosetta_stats_get_block_size_histogram(uint32_t *hist, size_t count);

/**
 * Get cache hit rate
 * Returns: Hit rate as percentage (0.0 - 100.0)
 */
double rosetta_stats_get_cache_hit_rate(void);

/**
 * Get average block size
 * Returns: Average block size in instructions
 */
double rosetta_stats_get_avg_block_size(void);

/**
 * Get average expansion ratio
 * Returns: x86 bytes per ARM64 byte
 */
double rosetta_stats_get_expansion_ratio(void);

/* ============================================================================
 * Local Statistics (for backward compatibility)
 * ============================================================================ */

/**
 * Get local statistics
 * @translations: Output translations count
 * @cache_hits: Output cache hits
 * @cache_misses: Output cache misses
 * @total_insns: Output total instructions
 * @alu: Output ALU instructions
 * @mem: Output memory instructions
 * @branch: Output branch instructions
 * @system: Output system instructions
 */
void rosetta_stats_get_local(uint64_t *translations, uint64_t *cache_hits,
                              uint64_t *cache_misses, uint64_t *total_insns,
                              uint64_t *alu, uint64_t *mem,
                              uint64_t *branch, uint64_t *system);

/**
 * Reset local statistics
 */
void rosetta_stats_reset_local(void);

/**
 * Record local translation
 */
void rosetta_stats_record_local_translation(void);

/**
 * Record local cache hit
 */
void rosetta_stats_record_local_cache_hit(void);

/**
 * Record local cache miss
 */
void rosetta_stats_record_local_cache_miss(void);

/**
 * Record local instruction
 * @type: Instruction type (ALU, MEM, BRANCH, SYSTEM)
 */
void rosetta_stats_record_local_insn(const char *type);

#endif /* ROSETTA_REFACTORED_STATS_H */
