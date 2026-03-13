#ifndef ROSETTA_INSN_CACHE_H
#define ROSETTA_INSN_CACHE_H

/* ============================================================================
 * Rosetta Instruction Decode Cache
 * ============================================================================
 *
 * Caches decoded x86_64 instructions to avoid redundant decoding.
 * ============================================================================ */

#include "rosetta_x86_decode.h"
#include "rosetta_types.h"

/* ============================================================================
 * Instruction Cache Configuration
 * ============================================================================ */

#define INSN_CACHE_BITS           10      /* 1024 entries */
#define INSN_CACHE_SIZE           (1U << INSN_CACHE_BITS)
#define INSN_CACHE_MASK           (INSN_CACHE_SIZE - 1)

/* Cache entry flags */
#define INSN_CACHE_FLAG_VALID     0x01
#define INSN_CACHE_FLAG_USED      0x02

/* ============================================================================
 * Instruction Cache Entry
 * ============================================================================ */

typedef struct insn_cache_entry {
    u64 guest_pc;                      /* Guest PC address */
    x86_insn_t insn;                   /* Decoded instruction */
    u32 hash;                          /* Hash of guest PC */
    u32 flags;                         /* Entry flags */
    u32 access_count;                  /* Access count for LRU */
} insn_cache_entry_t;

/* ============================================================================
 * Instruction Cache Statistics
 * ============================================================================ */

typedef struct insn_cache_stats {
    u64 lookups;                       /* Total cache lookups */
    u64 hits;                          /* Cache hits */
    u64 misses;                        /* Cache misses */
    u64 inserts;                       /* Cache inserts */
    u64 evictions;                     /* Cache evictions */
} insn_cache_stats_t;

/* ============================================================================
 * Instruction Cache API
 * ============================================================================ */

/**
 * Initialize instruction cache
 * @return ROSETTA_OK on success
 */
int insn_cache_init(void);

/**
 * Cleanup instruction cache
 */
void insn_cache_cleanup(void);

/**
 * Look up instruction in cache
 * @param guest_pc Guest PC address
 * @param insn Output: decoded instruction (if found)
 * @return 1 if found, 0 if not found
 */
int insn_cache_lookup(u64 guest_pc, x86_insn_t *insn);

/**
 * Insert instruction into cache
 * @param guest_pc Guest PC address
 * @param insn Decoded instruction to cache
 * @return ROSETTA_OK on success
 */
int insn_cache_insert(u64 guest_pc, const x86_insn_t *insn);

/**
 * Invalidate instruction cache entry
 * @param guest_pc Guest PC address to invalidate
 */
void insn_cache_invalidate(u64 guest_pc);

/**
 * Flush entire instruction cache
 */
void insn_cache_flush(void);

/**
 * Get cache statistics
 * @param stats Output: cache statistics
 */
void insn_cache_get_stats(insn_cache_stats_t *stats);

/**
 * Calculate hit rate
 * @return Hit rate as percentage (0.0-100.0)
 */
double insn_cache_hit_rate(void);

/**
 * Check if cache is enabled
 * @return 1 if enabled, 0 if disabled
 */
int insn_cache_is_enabled(void);

/**
 * Enable or disable cache
 * @param enabled 1 to enable, 0 to disable
 */
void insn_cache_set_enabled(int enabled);

#endif /* ROSETTA_INSN_CACHE_H */
