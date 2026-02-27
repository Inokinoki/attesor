/*
 * Rosetta Translation Cache
 *
 * This module provides the translation cache for storing
 * guest-to-host code translation mappings.
 */

#include "rosetta_types.h"
#include "rosetta_hash.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Translation Cache Configuration
 * ============================================================================ */

/* These are also defined in rosetta_types.h - using local values for standalone operation */
#ifndef TRANSLATION_CACHE_BITS
#define TRANSLATION_CACHE_BITS  12
#endif
#ifndef TRANSLATION_CACHE_SIZE
#define TRANSLATION_CACHE_SIZE  (1U << TRANSLATION_CACHE_BITS)
#endif
#ifndef TRANSLATION_CACHE_MASK
#define TRANSLATION_CACHE_MASK  (TRANSLATION_CACHE_SIZE - 1)
#endif

/* ============================================================================
 * Translation Cache Entry Structure
 * ============================================================================ */

typedef struct {
    uint64_t guest_addr;    /* Guest ARM64 PC */
    uint64_t host_addr;     /* Host x86_64 PC */
    uint32_t hash;          /* Hash of guest PC */
    uint32_t refcount;      /* Reference count */
    uint32_t block_size;    /* Size of translated block */
    uint32_t flags;         /* Block flags */
} TranslationCacheEntry;

/* Block flags */
#define BLOCK_FLAG_VALID    0x01
#define BLOCK_FLAG_HOT      0x02
#define BLOCK_FLAG_LINKED   0x04

/* ============================================================================
 * Static Translation Cache
 * ============================================================================ */

static TranslationCacheEntry translation_cache[TRANSLATION_CACHE_SIZE];
static uint32_t cache_insert_index = 0;
static uint32_t cache_hits = 0;
static uint32_t cache_misses = 0;

/* ============================================================================
 * Translation Cache Operations
 * ============================================================================ */

/**
 * Initialize translation cache
 *
 * Clears all cache entries and resets statistics.
 */
void translation_cache_init(void)
{
    uint32_t i;
    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].guest_addr = 0;
        translation_cache[i].host_addr = 0;
        translation_cache[i].hash = 0;
        translation_cache[i].refcount = 0;
        translation_cache[i].block_size = 0;
        translation_cache[i].flags = 0;
    }
    cache_insert_index = 0;
    cache_hits = 0;
    cache_misses = 0;
}

/**
 * Flush translation cache
 *
 * Invalidates all cache entries without clearing them.
 */
void translation_cache_flush(void)
{
    uint32_t i;
    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].flags &= ~BLOCK_FLAG_VALID;
        translation_cache[i].refcount = 0;
    }
    cache_insert_index = 0;
}

/**
 * Look up a translation in the cache
 *
 * Performs a hash-based lookup to find a cached translation
 * for the given guest PC.
 *
 * @param guest_pc Guest ARM64 PC to look up
 * @return Host x86_64 PC if found, NULL otherwise
 */
void *translation_cache_lookup(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Check cache entry */
    if (translation_cache[index].guest_addr == guest_pc &&
        translation_cache[index].host_addr != 0 &&
        (translation_cache[index].flags & BLOCK_FLAG_VALID)) {
        translation_cache[index].refcount++;
        cache_hits++;
        return (void *)(uintptr_t)translation_cache[index].host_addr;
    }

    cache_misses++;
    return NULL;
}

/**
 * Insert a translation into the cache
 *
 * Inserts a new guest-to-host translation mapping into
 * the translation cache.
 *
 * @param guest Guest ARM64 PC
 * @param host Host x86_64 PC
 * @param size Size of translated block
 * @return 0 on success, -1 on failure
 */
int translation_cache_insert(uint64_t guest, uint64_t host, size_t size)
{
    uint32_t hash = hash_address(guest);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    /* Insert into cache (simple direct-mapped cache) */
    translation_cache[index].guest_addr = guest;
    translation_cache[index].host_addr = host;
    translation_cache[index].hash = hash;
    translation_cache[index].refcount = 1;
    translation_cache[index].block_size = (uint32_t)size;
    translation_cache[index].flags = BLOCK_FLAG_VALID;

    cache_insert_index = index;

    return 0;
}

/**
 * Remove a translation from the cache
 *
 * Invalidates a cache entry for the given guest PC.
 *
 * @param guest_pc Guest ARM64 PC to remove
 * @return 0 on success, -1 if not found
 */
int translation_cache_remove(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].flags &= ~BLOCK_FLAG_VALID;
        translation_cache[index].refcount = 0;
        return 0;
    }

    return -1;
}

/**
 * Invalidate a translation block
 *
 * Marks a block as invalid without removing it.
 *
 * @param guest_pc Guest ARM64 PC to invalidate
 */
void translation_cache_invalidate(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].flags &= ~BLOCK_FLAG_VALID;
    }
}

/**
 * Check if a translation is valid
 *
 * @param guest_pc Guest ARM64 PC to check
 * @return 1 if valid, 0 otherwise
 */
int translation_cache_is_valid(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    return (translation_cache[index].guest_addr == guest_pc &&
            translation_cache[index].host_addr != 0 &&
            (translation_cache[index].flags & BLOCK_FLAG_VALID));
}

/**
 * Get cache statistics
 *
 * @param hits Output: number of cache hits
 * @param misses Output: number of cache misses
 * @param entries Output: number of valid entries
 */
void translation_cache_stats(uint32_t *hits, uint32_t *misses, uint32_t *entries)
{
    uint32_t i;
    uint32_t valid_count = 0;

    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        if (translation_cache[i].flags & BLOCK_FLAG_VALID) {
            valid_count++;
        }
    }

    if (hits) *hits = cache_hits;
    if (misses) *misses = cache_misses;
    if (entries) *entries = valid_count;
}

/**
 * Mark a block as hot
 *
 * @param guest_pc Guest ARM64 PC
 */
void translation_cache_mark_hot(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & TRANSLATION_CACHE_MASK;

    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].flags |= BLOCK_FLAG_HOT;
    }
}

/**
 * Get cache size
 *
 * @return Number of entries in cache
 */
size_t translation_cache_get_size(void)
{
    return TRANSLATION_CACHE_SIZE;
}

/**
 * Check if cache is full
 *
 * @return 1 if full, 0 otherwise
 */
int translation_cache_is_full(void)
{
    uint32_t i;
    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        if (!(translation_cache[i].flags & BLOCK_FLAG_VALID)) {
            return 0;
        }
    }
    return 1;
}
