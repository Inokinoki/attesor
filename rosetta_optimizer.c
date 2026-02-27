/* ============================================================================
 * Rosetta Binary Translator - Optimizer Module Implementation
 * ============================================================================
 *
 * This module implements optimization wrappers for the translation layer,
 * including inline caching, fast paths, and statistics-integrated execution.
 * ============================================================================ */

#include "rosetta_optimizer.h"
#include "rosetta_refactored_exec.h"
#include "rosetta_trans_cache.h"
#include "rosetta_refactored_stats.h"

/* ============================================================================
 * Hot Block Tracking
 * ============================================================================ */

#define DEFAULT_HOT_THRESHOLD   10

static uint32_t g_hot_block_threshold = DEFAULT_HOT_THRESHOLD;

/* Simple hot block tracking using a small cache */
#define HOT_BLOCK_CACHE_SIZE    64

typedef struct {
    uint64_t pc;
    uint32_t count;
    uint32_t priority;
} hot_block_entry_t;

static hot_block_entry_t g_hot_block_cache[HOT_BLOCK_CACHE_SIZE];

/* ============================================================================
 * Inline Translation Cache
 * ============================================================================ */

/**
 * translate_block_inline - Translate with inline cache optimization
 * @guest_pc: Starting program counter
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block_inline(uint64_t guest_pc)
{
    /* Fast path: check cache first */
    void *cached = refactored_translation_cache_lookup(guest_pc);
    if (cached) {
        /* Record cache hit for statistics */
        rosetta_stats_record_cache_hit();
        return cached;
    }

    /* Record cache miss */
    rosetta_stats_record_cache_miss();

    /* Slow path: full translation */
    return translate_block(guest_pc);
}

/**
 * translate_and_execute - Translate and execute in one step
 * @guest_pc: Starting program counter
 * Returns: 0 on success, -1 on error
 */
int translate_and_execute(uint64_t guest_pc)
{
    void *code = translate_block_inline(guest_pc);
    if (!code) {
        return -1;
    }

    rosetta_execute(code);
    return 0;
}

/* ============================================================================
 * Statistics-Integrated Translation
 * ============================================================================ */

/**
 * translate_block_with_stats - Translate block with statistics
 * @guest_pc: Starting program counter
 * @record_stats: Whether to record statistics
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block_with_stats(uint64_t guest_pc, bool record_stats)
{
    void *result = translate_block(guest_pc);

    if (record_stats && result) {
        rosetta_stats_record_block(guest_pc, (uint64_t)(uintptr_t)result, 0, 0, 0);
    }

    return result;
}

/**
 * execute_with_stats - Execute with statistics tracking
 * @host_code: Pointer to translated code
 * @insn_count: Number of instructions in block
 */
void execute_with_stats(void *host_code, int insn_count)
{
    if (!host_code) return;

    (void)insn_count;  /* Could be used for more detailed stats */
    rosetta_execute(host_code);
}

/* ============================================================================
 * Hot/Cold Path Optimization
 * ============================================================================ */

/**
 * is_block_hot - Check if a block is frequently executed
 * @guest_pc: Program counter to check
 * Returns: 1 if hot, 0 otherwise
 */
int is_block_hot(uint64_t guest_pc)
{
    /* Check hot block cache */
    for (size_t i = 0; i < HOT_BLOCK_CACHE_SIZE; i++) {
        if (g_hot_block_cache[i].pc == guest_pc) {
            return g_hot_block_cache[i].count >= g_hot_block_threshold;
        }
    }
    return 0;
}

/**
 * get_hot_block_threshold - Get hot block threshold
 * Returns: Number of executions to be considered hot
 */
uint32_t get_hot_block_threshold(void)
{
    return g_hot_block_threshold;
}

/**
 * set_hot_block_threshold - Set hot block threshold
 * @threshold: New threshold value
 */
void set_hot_block_threshold(uint32_t threshold)
{
    if (threshold > 0) {
        g_hot_block_threshold = threshold;
    }
}

/* ============================================================================
 * Translation Hints
 * ============================================================================ */

/**
 * translate_block_with_hints - Translate with optimization hints
 * @guest_pc: Starting program counter
 * @hints: Optimization hints bitmask
 * Returns: Pointer to translated code, or NULL on error
 */
void *translate_block_with_hints(uint64_t guest_pc, uint32_t hints)
{
    void *result;

    /* Handle fast path hint */
    if (hints & ROS_OPT_HINT_FAST_PATH) {
        result = translate_block_inline(guest_pc);
    } else {
        result = translate_block(guest_pc);
    }

    /* Track hot blocks */
    if (hints & ROS_OPT_HINT_HOT) {
        for (size_t i = 0; i < HOT_BLOCK_CACHE_SIZE; i++) {
            if (g_hot_block_cache[i].pc == guest_pc) {
                g_hot_block_cache[i].count++;
                break;
            } else if (g_hot_block_cache[i].pc == 0) {
                g_hot_block_cache[i].pc = guest_pc;
                g_hot_block_cache[i].count = 1;
                break;
            }
        }
    }

    return result;
}

/**
 * mark_block_priority - Mark a block for preferential treatment
 * @guest_pc: Program counter to mark
 * @priority: Priority level (0-3, higher = more important)
 */
void mark_block_priority(uint64_t guest_pc, uint32_t priority)
{
    if (priority > 3) priority = 3;

    for (size_t i = 0; i < HOT_BLOCK_CACHE_SIZE; i++) {
        if (g_hot_block_cache[i].pc == guest_pc) {
            g_hot_block_cache[i].priority = priority;
            return;
        } else if (g_hot_block_cache[i].pc == 0) {
            g_hot_block_cache[i].pc = guest_pc;
            g_hot_block_cache[i].priority = priority;
            return;
        }
    }

    /* Cache full - replace lowest priority entry */
    size_t min_idx = 0;
    for (size_t i = 1; i < HOT_BLOCK_CACHE_SIZE; i++) {
        if (g_hot_block_cache[i].priority < g_hot_block_cache[min_idx].priority) {
            min_idx = i;
        }
    }

    g_hot_block_cache[min_idx].pc = guest_pc;
    g_hot_block_cache[min_idx].priority = priority;
    g_hot_block_cache[min_idx].count = 0;
}
