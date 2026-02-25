/* ============================================================================
 * Rosetta Translation Cache Module
 * ============================================================================
 *
 * This module handles caching of translated code blocks for fast lookup
 * during execution.
 * ============================================================================ */

#include "rosetta_cache.h"
#include <string.h>

/* Declare hash_address from rosetta_refactored.c */
extern uint32_t hash_address(uint64_t addr);

/* Static translation cache - use size from rosetta_types.h */
static TranslationCacheEntry translation_cache[TRANSLATION_CACHE_SIZE];

/**
 * Initialize translation cache
 */
void rosetta_cache_init(void)
{
    memset(translation_cache, 0, sizeof(translation_cache));
}

/**
 * Look up a translation in the cache
 * @param guest_pc Guest PC to look up
 * @return Host translated code pointer if found, NULL otherwise
 */
void *rosetta_cache_lookup(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & (TRANSLATION_CACHE_SIZE - 1);

    /* Check cache entry */
    if (translation_cache[index].guest_addr == guest_pc &&
        translation_cache[index].host_addr != 0) {
        translation_cache[index].refcount++;
        return (void *)translation_cache[index].host_addr;
    }

    return NULL;
}

/**
 * Insert a translation into the cache
 * @param guest Guest PC
 * @param host Host translated code address
 * @param size Size of translated block (unused for direct-mapped cache)
 * @return 0 on success, -1 on failure
 */
int rosetta_cache_insert(uint64_t guest, uint64_t host, size_t size)
{
    uint32_t hash = hash_address(guest);
    uint32_t index = hash & (TRANSLATION_CACHE_SIZE - 1);

    (void)size;  /* Direct-mapped cache doesn't need size */

    /* Insert into cache (simple direct-mapped cache) */
    translation_cache[index].guest_addr = guest;
    translation_cache[index].host_addr = host;
    translation_cache[index].hash = hash;
    translation_cache[index].refcount = 1;

    return 0;
}

/**
 * Invalidate a cache entry
 * @param guest_pc Guest PC to invalidate
 */
void rosetta_cache_invalidate(uint64_t guest_pc)
{
    uint32_t hash = hash_address(guest_pc);
    uint32_t index = hash & (TRANSLATION_CACHE_SIZE - 1);

    if (translation_cache[index].guest_addr == guest_pc) {
        translation_cache[index].guest_addr = 0;
        translation_cache[index].host_addr = 0;
        translation_cache[index].refcount = 0;
    }
}

/**
 * Flush entire translation cache
 */
void rosetta_cache_flush(void)
{
    for (int i = 0; i < (int)TRANSLATION_CACHE_SIZE; i++) {
        translation_cache[i].guest_addr = 0;
        translation_cache[i].host_addr = 0;
        translation_cache[i].hash = 0;
        translation_cache[i].refcount = 0;
    }
}

/* End of rosetta_cache.c */
