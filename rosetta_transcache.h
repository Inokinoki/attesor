/*
 * Rosetta Translation Cache Header
 *
 * This module provides the translation cache for storing
 * guest-to-host code translation mappings.
 */

#ifndef ROSETTA_TRANSCACHE_H
#define ROSETTA_TRANSCACHE_H

#include <stdint.h>
#include <stddef.h>

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
 * Cache Configuration
 * ============================================================================ */

#define TRANSLATION_CACHE_SIZE  4096
#define TRANSLATION_CACHE_MASK  (TRANSLATION_CACHE_SIZE - 1)
#define TRANSLATION_CACHE_BITS  12

/* ============================================================================
 * Translation Cache Operations
 * ============================================================================ */

/**
 * Initialize translation cache
 */
void translation_cache_init(void);

/**
 * Flush translation cache
 */
void translation_cache_flush(void);

/**
 * Look up a translation in the cache
 * @param guest_pc Guest ARM64 PC to look up
 * @return Host x86_64 PC if found, NULL otherwise
 */
void *translation_cache_lookup(uint64_t guest_pc);

/**
 * Insert a translation into the cache
 * @param guest Guest ARM64 PC
 * @param host Host x86_64 PC
 * @param size Size of translated block
 * @return 0 on success, -1 on failure
 */
int translation_cache_insert(uint64_t guest, uint64_t host, size_t size);

/**
 * Remove a translation from the cache
 * @param guest_pc Guest ARM64 PC to remove
 * @return 0 on success, -1 if not found
 */
int translation_cache_remove(uint64_t guest_pc);

/**
 * Invalidate a translation block
 * @param guest_pc Guest ARM64 PC to invalidate
 */
void translation_cache_invalidate(uint64_t guest_pc);

/**
 * Check if a translation is valid
 * @param guest_pc Guest ARM64 PC to check
 * @return 1 if valid, 0 otherwise
 */
int translation_cache_is_valid(uint64_t guest_pc);

/**
 * Mark a block as hot
 * @param guest_pc Guest ARM64 PC
 */
void translation_cache_mark_hot(uint64_t guest_pc);

/**
 * Get cache statistics
 * @param hits Output: number of cache hits
 * @param misses Output: number of cache misses
 * @param entries Output: number of valid entries
 */
void translation_cache_stats(uint32_t *hits, uint32_t *misses, uint32_t *entries);

/**
 * Get cache size
 * @return Number of entries in cache
 */
size_t translation_cache_get_size(void);

/**
 * Check if cache is full
 * @return 1 if full, 0 otherwise
 */
int translation_cache_is_full(void);

#endif /* ROSETTA_TRANSCACHE_H */
