/* ============================================================================
 * Rosetta Instruction Decode Cache Implementation
 * ============================================================================
 *
 * Caches decoded x86_64 instructions to avoid redundant decoding.
 * ============================================================================ */

#include "rosetta_insn_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Instruction Cache State
 * ============================================================================ */

static insn_cache_entry_t g_insn_cache[INSN_CACHE_SIZE];
static insn_cache_stats_t g_stats;
static int g_cache_enabled = 1;
static int g_cache_initialized = 0;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a guest PC address for cache lookup
 */
static inline u32 hash_guest_pc(u64 guest_pc)
{
    /* Simple hash function */
    return (u32)(guest_pc ^ (guest_pc >> 32));
}

/* ============================================================================
 * Cache API Implementation
 * ============================================================================ */

/**
 * Initialize instruction cache
 */
int insn_cache_init(void)
{
    if (g_cache_initialized) {
        return 0;  /* Already initialized */
    }

    /* Clear cache */
    memset(g_insn_cache, 0, sizeof(g_insn_cache));

    /* Clear statistics */
    memset(&g_stats, 0, sizeof(g_stats));

    /* Enable cache */
    g_cache_enabled = 1;
    g_cache_initialized = 1;

    return 0;
}

/**
 * Cleanup instruction cache
 */
void insn_cache_cleanup(void)
{
    if (!g_cache_initialized) {
        return;
    }

    /* Clear cache */
    memset(g_insn_cache, 0, sizeof(g_insn_cache));

    /* Clear statistics */
    memset(&g_stats, 0, sizeof(g_stats));

    /* Mark as uninitialized */
    g_cache_initialized = 0;
}

/**
 * Look up instruction in cache
 */
int insn_cache_lookup(u64 guest_pc, x86_insn_t *insn)
{
    if (!g_cache_initialized || !g_cache_enabled || !insn) {
        return 0;
    }

    /* Calculate cache index */
    u32 hash = hash_guest_pc(guest_pc);
    u32 index = hash & INSN_CACHE_MASK;

    /* Update statistics */
    g_stats.lookups++;

    /* Check cache entry */
    insn_cache_entry_t *entry = &g_insn_cache[index];

    if ((entry->flags & INSN_CACHE_FLAG_VALID) &&
        (entry->guest_pc == guest_pc)) {

        /* Cache hit */
        g_stats.hits++;
        entry->access_count++;

        /* Copy instruction */
        memcpy(insn, &entry->insn, sizeof(x86_insn_t));

        return 1;  /* Found */
    }

    /* Cache miss */
    g_stats.misses++;
    return 0;  /* Not found */
}

/**
 * Insert instruction into cache
 */
int insn_cache_insert(u64 guest_pc, const x86_insn_t *insn)
{
    if (!g_cache_initialized || !g_cache_enabled || !insn) {
        return -1;
    }

    /* Calculate cache index */
    u32 hash = hash_guest_pc(guest_pc);
    u32 index = hash & INSN_CACHE_MASK;

    /* Get cache entry */
    insn_cache_entry_t *entry = &g_insn_cache[index];

    /* Check if we need to evict */
    if ((entry->flags & INSN_CACHE_FLAG_VALID) &&
        (entry->guest_pc != guest_pc)) {
        /* Evict existing entry */
        g_stats.evictions++;
    }

    /* Insert new entry */
    entry->guest_pc = guest_pc;
    memcpy(&entry->insn, insn, sizeof(x86_insn_t));
    entry->hash = hash;
    entry->flags = INSN_CACHE_FLAG_VALID;
    entry->access_count = 1;

    /* Update statistics */
    g_stats.inserts++;

    return 0;
}

/**
 * Invalidate instruction cache entry
 */
void insn_cache_invalidate(u64 guest_pc)
{
    if (!g_cache_initialized) {
        return;
    }

    /* Calculate cache index */
    u32 hash = hash_guest_pc(guest_pc);
    u32 index = hash & INSN_CACHE_MASK;

    /* Get cache entry */
    insn_cache_entry_t *entry = &g_insn_cache[index];

    /* Invalidate if matching */
    if ((entry->flags & INSN_CACHE_FLAG_VALID) &&
        (entry->guest_pc == guest_pc)) {
        entry->flags = 0;
        entry->access_count = 0;
    }
}

/**
 * Flush entire instruction cache
 */
void insn_cache_flush(void)
{
    if (!g_cache_initialized) {
        return;
    }

    /* Clear cache */
    memset(g_insn_cache, 0, sizeof(g_insn_cache));

    /* Note: Statistics are preserved */
}

/**
 * Get cache statistics
 */
void insn_cache_get_stats(insn_cache_stats_t *stats)
{
    if (!g_cache_initialized || !stats) {
        return;
    }

    memcpy(stats, &g_stats, sizeof(insn_cache_stats_t));
}

/**
 * Calculate hit rate
 */
double insn_cache_hit_rate(void)
{
    if (!g_cache_initialized) {
        return 0.0;
    }

    u64 total = g_stats.hits + g_stats.misses;
    if (total == 0) {
        return 0.0;
    }

    return (100.0 * g_stats.hits) / total;
}

/**
 * Check if cache is enabled
 */
int insn_cache_is_enabled(void)
{
    return g_cache_enabled && g_cache_initialized;
}

/**
 * Enable or disable cache
 */
void insn_cache_set_enabled(int enabled)
{
    g_cache_enabled = enabled ? 1 : 0;
}
