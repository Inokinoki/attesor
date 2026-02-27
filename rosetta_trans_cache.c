/* ============================================================================
 * Rosetta Translator - Translation Cache Implementation
 * ============================================================================
 *
 * This module implements translation cache management for binary translation.
 * ============================================================================ */

#include "rosetta_trans_cache.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* Global translation cache instance */
static trans_cache_t g_trans_cache;
static bool g_cache_initialized = false;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

uint32_t trans_hash_address(uint64_t addr)
{
    /* Golden ratio multiplicative hash */
    uint64_t hash = addr * 2654435761ULL;
    return (uint32_t)(hash >> 32);
}

uint32_t trans_hash_string(const char *s)
{
    /* DJB2 hash algorithm */
    uint32_t hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/* ============================================================================
 * Translation Cache Management
 * ============================================================================ */

int trans_cache_init(trans_cache_t *cache)
{
    if (!cache) return -1;

    /* Clear cache entries */
    memset(cache->entries, 0, sizeof(cache->entries));

    /* Initialize code cache */
    cache->code_cache.buffer = (uint8_t *)mmap(NULL, REFACTORED_CODE_CACHE_SIZE,
                                                PROT_READ | PROT_WRITE,
                                                MAP_PRIVATE | MAP_ANON, -1, 0);
    if (cache->code_cache.buffer == MAP_FAILED) {
        cache->code_cache.buffer = NULL;
        return -1;
    }

    cache->code_cache.size = REFACTORED_CODE_CACHE_SIZE;
    cache->code_cache.offset = 0;

    /* Reset statistics */
    cache->hits = 0;
    cache->misses = 0;
    cache->inserts = 0;
    cache->flushes = 0;

    return 0;
}

void trans_cache_cleanup(trans_cache_t *cache)
{
    if (!cache) return;

    /* Free code cache */
    if (cache->code_cache.buffer) {
        munmap(cache->code_cache.buffer, cache->code_cache.size);
        cache->code_cache.buffer = NULL;
    }

    /* Clear cache entries */
    memset(cache->entries, 0, sizeof(cache->entries));
    cache->code_cache.size = 0;
    cache->code_cache.offset = 0;
}

void *trans_cache_lookup(trans_cache_t *cache, uint64_t guest_pc)
{
    uint32_t hash = trans_hash_address(guest_pc);
    uint32_t index = hash & REFACTORED_TRANSLATION_CACHE_MASK;
    trans_cache_entry_t *entry;

    if (!cache) return NULL;

    entry = &cache->entries[index];

    if (entry->guest_pc == guest_pc &&
        entry->host_addr != NULL &&
        (entry->flags & TRANS_BLOCK_VALID)) {
        entry->refcount++;
        cache->hits++;
        return entry->host_addr;
    }

    cache->misses++;
    return NULL;
}

int trans_cache_insert(trans_cache_t *cache, uint64_t guest_pc, void *host_addr, uint32_t size)
{
    uint32_t hash = trans_hash_address(guest_pc);
    uint32_t index = hash & REFACTORED_TRANSLATION_CACHE_MASK;
    trans_cache_entry_t *entry;

    if (!cache || !host_addr || size == 0) return -1;

    entry = &cache->entries[index];

    entry->guest_pc = guest_pc;
    entry->host_addr = host_addr;
    entry->size = size;
    entry->hash = hash;
    entry->flags = TRANS_BLOCK_VALID;
    entry->refcount = 1;

    cache->inserts++;

    return 0;
}

int trans_cache_invalidate(trans_cache_t *cache, uint64_t guest_pc)
{
    uint32_t hash = trans_hash_address(guest_pc);
    uint32_t index = hash & REFACTORED_TRANSLATION_CACHE_MASK;
    trans_cache_entry_t *entry;

    if (!cache) return -1;

    entry = &cache->entries[index];

    if (entry->guest_pc == guest_pc) {
        entry->guest_pc = 0;
        entry->host_addr = NULL;
        entry->size = 0;
        entry->refcount = 0;
        entry->flags = 0;
    }

    return 0;
}

void trans_cache_flush(trans_cache_t *cache)
{
    uint32_t i;

    if (!cache) return;

    /* Clear all cache entries */
    for (i = 0; i < REFACTORED_TRANSLATION_CACHE_SIZE; i++) {
        cache->entries[i].guest_pc = 0;
        cache->entries[i].host_addr = NULL;
        cache->entries[i].size = 0;
        cache->entries[i].hash = 0;
        cache->entries[i].refcount = 0;
        cache->entries[i].flags = 0;
    }

    /* Reset code cache */
    cache->code_cache.offset = 0;

    /* Reset statistics */
    cache->hits = 0;
    cache->misses = 0;
    cache->inserts = 0;
    cache->flushes++;
}

uint32_t trans_cache_get_size(trans_cache_t *cache)
{
    uint32_t i, count = 0;

    if (!cache) return 0;

    for (i = 0; i < REFACTORED_TRANSLATION_CACHE_SIZE; i++) {
        if (cache->entries[i].guest_pc != 0 &&
            cache->entries[i].host_addr != NULL &&
            (cache->entries[i].flags & TRANS_BLOCK_VALID)) {
            count++;
        }
    }

    return count;
}

bool trans_cache_is_full(trans_cache_t *cache)
{
    return trans_cache_get_size(cache) >= REFACTORED_TRANSLATION_CACHE_SIZE;
}

/* ============================================================================
 * Code Cache Management
 * ============================================================================ */

void *trans_code_cache_alloc(code_cache_t *cache, size_t size)
{
    uint8_t *ptr;

    if (!cache || !cache->buffer || size == 0) return NULL;

    /* Check if we have enough space */
    if (cache->offset + size > cache->size) {
        return NULL;  /* Cache full */
    }

    /* Allocate from current position */
    ptr = cache->buffer + cache->offset;
    cache->offset += size;

    return ptr;
}

void *trans_code_cache_alloc_aligned(code_cache_t *cache, size_t size, size_t alignment)
{
    size_t aligned_offset;
    uint8_t *ptr;

    if (!cache || !cache->buffer || size == 0) return NULL;

    /* Calculate aligned offset */
    aligned_offset = (cache->offset + alignment - 1) & ~(alignment - 1);

    /* Check if we have enough space */
    if (aligned_offset + size > cache->size) {
        return NULL;  /* Cache full */
    }

    /* Allocate from aligned position */
    ptr = cache->buffer + aligned_offset;
    cache->offset = aligned_offset + size;

    return ptr;
}

void trans_code_cache_reset(code_cache_t *cache)
{
    if (!cache) return;
    cache->offset = 0;
}

size_t trans_code_cache_get_free(code_cache_t *cache)
{
    if (!cache) return 0;
    return cache->size - cache->offset;
}

size_t trans_code_cache_get_used(code_cache_t *cache)
{
    if (!cache) return 0;
    return cache->offset;
}

/* ============================================================================
 * Global Translation Cache (for backward compatibility)
 * ============================================================================ */

uint32_t refactored_hash_address(uint64_t addr)
{
    return trans_hash_address(addr);
}

void *refactored_translation_cache_lookup(uint64_t guest_pc)
{
    if (!g_cache_initialized) return NULL;
    return trans_cache_lookup(&g_trans_cache, guest_pc);
}

void refactored_translation_cache_insert(uint64_t guest_pc, void *host_addr, uint32_t size)
{
    if (!g_cache_initialized) return;
    trans_cache_insert(&g_trans_cache, guest_pc, host_addr, size);
}

void *refactored_code_cache_alloc(size_t size)
{
    if (!g_cache_initialized) return NULL;
    return trans_code_cache_alloc(&g_trans_cache.code_cache, size);
}

int refactored_translation_cache_init(void)
{
    if (g_cache_initialized) return 0;

    if (trans_cache_init(&g_trans_cache) != 0) {
        return -1;
    }

    g_cache_initialized = true;
    return 0;
}

void refactored_translation_cache_cleanup(void)
{
    if (!g_cache_initialized) return;

    trans_cache_cleanup(&g_trans_cache);
    g_cache_initialized = false;
}

/* ============================================================================
 * Block Chaining Support
 * ============================================================================ */

/**
 * trans_cache_chain_blocks - Chain two translation blocks together
 * @from_block: Source block to patch
 * @to_block: Target block to chain to
 * @branch_type: Type of branch (0 = fall-through, 1 = taken)
 * Returns: 0 on success, -1 on error
 */
int trans_cache_chain_blocks(trans_cache_entry_t *from_block,
                             trans_cache_entry_t *to_block,
                             int branch_type)
{
    if (!from_block || !to_block || !(from_block->flags & TRANS_BLOCK_VALID)) {
        return -1;
    }

    /* Mark source block as linked */
    from_block->flags |= TRANS_BLOCK_LINKED;

    /* In a full implementation, we would patch the x86_64 code at the end
     * of from_block to directly jump to to_block->host_addr, eliminating
     * the need to return to the dispatch loop.
     *
     * For now, we just mark the linkage for statistics purposes.
     */

    (void)branch_type;  /* Used for conditional branches */

    return 0;
}

/**
 * trans_cache_unchain_block - Remove chaining from a block
 * @block: Block to unchain
 * Returns: 0 on success, -1 on error
 */
int trans_cache_unchain_block(trans_cache_entry_t *block)
{
    if (!block) return -1;

    block->flags &= ~TRANS_BLOCK_LINKED;

    /* In a full implementation, we would restore the original RET
     * instruction at the end of the block.
     */

    return 0;
}

/**
 * refactored_chain_blocks - Chain two blocks (global instance)
 * @guest_from: Guest PC of source block
 * @guest_to: Guest PC of target block
 * @branch_type: Type of branch
 * Returns: 0 on success, -1 on error
 */
int refactored_chain_blocks(uint64_t guest_from, uint64_t guest_to, int branch_type)
{
    uint32_t hash_from, hash_to;
    trans_cache_entry_t *entry_from, *entry_to;

    if (!g_cache_initialized) return -1;

    hash_from = trans_hash_address(guest_from);
    hash_to = trans_hash_address(guest_to);

    entry_from = &g_trans_cache.entries[hash_from & REFACTORED_TRANSLATION_CACHE_MASK];
    entry_to = &g_trans_cache.entries[hash_to & REFACTORED_TRANSLATION_CACHE_MASK];

    /* Verify entries match */
    if (entry_from->guest_pc != guest_from ||
        entry_to->guest_pc != guest_to) {
        return -1;
    }

    return trans_cache_chain_blocks(entry_from, entry_to, branch_type);
}

/**
 * refactored_unchain_block - Unchain a block (global instance)
 * @guest_pc: Guest PC of block to unchain
 * Returns: 0 on success, -1 on error
 */
int refactored_unchain_block(uint64_t guest_pc)
{
    uint32_t hash;
    trans_cache_entry_t *entry;

    if (!g_cache_initialized) return -1;

    hash = trans_hash_address(guest_pc);
    entry = &g_trans_cache.entries[hash & REFACTORED_TRANSLATION_CACHE_MASK];

    if (entry->guest_pc != guest_pc) {
        return -1;
    }

    return trans_cache_unchain_block(entry);
}
