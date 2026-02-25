/* ============================================================================
 * Rosetta Translation Cache Module
 * ============================================================================
 *
 * This module handles caching of translated code blocks for fast lookup
 * during execution.
 * ============================================================================ */

#ifndef ROSETTA_CACHE_H
#define ROSETTA_CACHE_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Translation Cache Entry Structure
 * ============================================================================ */

/**
 * Translation cache entry
 */
typedef struct {
    uint64_t guest_addr;    /* Guest PC address */
    uint64_t host_addr;     /* Host translated code address */
    uint32_t hash;          /* Hash of guest address */
    uint32_t refcount;      /* Reference count */
} TranslationCacheEntry;

/* ============================================================================
 * Translation Cache Functions
 * ============================================================================ */

/**
 * Initialize translation cache
 */
void rosetta_cache_init(void);

/**
 * Look up a translation in the cache
 * @param guest_pc Guest PC to look up
 * @return Host translated code pointer if found, NULL otherwise
 */
void *rosetta_cache_lookup(uint64_t guest_pc);

/**
 * Insert a translation into the cache
 * @param guest Guest PC
 * @param host Host translated code address
 * @param size Size of translated block
 * @return 0 on success, -1 on failure
 */
int rosetta_cache_insert(uint64_t guest, uint64_t host, size_t size);

/**
 * Invalidate a cache entry
 * @param guest_pc Guest PC to invalidate
 */
void rosetta_cache_invalidate(uint64_t guest_pc);

/**
 * Flush entire translation cache
 */
void rosetta_cache_flush(void);

#endif /* ROSETTA_CACHE_H */
