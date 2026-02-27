/* ============================================================================
 * Rosetta Translator - Translation Cache Header
 * ============================================================================
 *
 * This module provides translation cache management for binary translation.
 * It implements hash-based lookup and insertion of translated code blocks.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_CACHE_H
#define ROSETTA_TRANS_CACHE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Translation cache configuration */
#define REFACTORED_TRANSLATION_CACHE_BITS 12
#define REFACTORED_TRANSLATION_CACHE_SIZE (1U << REFACTORED_TRANSLATION_CACHE_BITS)
#define REFACTORED_TRANSLATION_CACHE_MASK (REFACTORED_TRANSLATION_CACHE_SIZE - 1)

/* Code cache configuration */
#define REFACTORED_CODE_CACHE_SIZE (16 * 1024 * 1024)  /* 16 MB */

/* Translation block flags */
#define TRANS_BLOCK_VALID    0x01
#define TRANS_BLOCK_LINKED   0x02
#define TRANS_BLOCK_HOT      0x04
#define TRANS_BLOCK_DIRTY    0x08

/* Translation cache entry */
typedef struct {
    uint64_t guest_pc;      /* Guest ARM64 PC */
    void *host_addr;        /* Host x86_64 code address */
    uint32_t size;          /* Size of translated code */
    uint32_t hash;          /* Hash of guest PC */
    uint32_t flags;         /* Block flags */
    uint32_t refcount;      /* Reference count */
} trans_cache_entry_t;

/* Code cache state */
typedef struct {
    uint8_t *buffer;        /* Code cache buffer */
    size_t size;            /* Total cache size */
    size_t offset;          /* Current allocation offset */
} code_cache_t;

/* Translation cache state */
typedef struct {
    trans_cache_entry_t entries[REFACTORED_TRANSLATION_CACHE_SIZE];
    code_cache_t code_cache;
    uint32_t hits;
    uint32_t misses;
    uint32_t inserts;
    uint32_t flushes;
} trans_cache_t;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * trans_hash_address - Hash a 64-bit address for cache lookup
 * @addr: Address to hash
 * Returns: 32-bit hash value
 */
uint32_t trans_hash_address(uint64_t addr);

/**
 * trans_hash_string - Hash a null-terminated string
 * @s: String to hash
 * Returns: 32-bit hash value
 */
uint32_t trans_hash_string(const char *s);

/* ============================================================================
 * Translation Cache Management
 * ============================================================================ */

/**
 * trans_cache_init - Initialize translation cache
 * @cache: Translation cache to initialize
 * Returns: 0 on success, -1 on error
 */
int trans_cache_init(trans_cache_t *cache);

/**
 * trans_cache_cleanup - Cleanup translation cache
 * @cache: Translation cache to cleanup
 */
void trans_cache_cleanup(trans_cache_t *cache);

/**
 * trans_cache_lookup - Look up translation by guest PC
 * @cache: Translation cache
 * @guest_pc: Guest ARM64 PC to look up
 * Returns: Host x86_64 code pointer if found, NULL otherwise
 */
void *trans_cache_lookup(trans_cache_t *cache, uint64_t guest_pc);

/**
 * trans_cache_insert - Insert translation into cache
 * @cache: Translation cache
 * @guest_pc: Guest ARM64 PC
 * @host_addr: Host x86_64 code address
 * @size: Size of translated code
 * Returns: 0 on success, -1 on error
 */
int trans_cache_insert(trans_cache_t *cache, uint64_t guest_pc, void *host_addr, uint32_t size);

/**
 * trans_cache_invalidate - Invalidate translation for guest PC
 * @cache: Translation cache
 * @guest_pc: Guest ARM64 PC to invalidate
 * Returns: 0 on success, -1 on error
 */
int trans_cache_invalidate(trans_cache_t *cache, uint64_t guest_pc);

/**
 * trans_cache_flush - Flush entire translation cache
 * @cache: Translation cache to flush
 */
void trans_cache_flush(trans_cache_t *cache);

/**
 * trans_cache_get_size - Get number of valid entries
 * @cache: Translation cache
 * Returns: Number of valid cache entries
 */
uint32_t trans_cache_get_size(trans_cache_t *cache);

/**
 * trans_cache_is_full - Check if cache is full
 * @cache: Translation cache
 * Returns: true if cache is full, false otherwise
 */
bool trans_cache_is_full(trans_cache_t *cache);

/* ============================================================================
 * Code Cache Management
 * ============================================================================ */

/**
 * trans_code_cache_alloc - Allocate memory from code cache
 * @cache: Code cache
 * @size: Size in bytes
 * Returns: Pointer to allocated memory or NULL
 */
void *trans_code_cache_alloc(code_cache_t *cache, size_t size);

/**
 * trans_code_cache_alloc_aligned - Allocate aligned memory from code cache
 * @cache: Code cache
 * @size: Size in bytes
 * @alignment: Alignment requirement
 * Returns: Pointer to allocated memory or NULL
 */
void *trans_code_cache_alloc_aligned(code_cache_t *cache, size_t size, size_t alignment);

/**
 * trans_code_cache_reset - Reset code cache to initial state
 * @cache: Code cache
 */
void trans_code_cache_reset(code_cache_t *cache);

/**
 * trans_code_cache_get_free - Get free space in code cache
 * @cache: Code cache
 * Returns: Free space in bytes
 */
size_t trans_code_cache_get_free(code_cache_t *cache);

/**
 * trans_code_cache_get_used - Get used space in code cache
 * @cache: Code cache
 * Returns: Used space in bytes
 */
size_t trans_code_cache_get_used(code_cache_t *cache);

/* ============================================================================
 * Global Translation Cache (for backward compatibility)
 * ============================================================================ */

/**
 * refactored_hash_address - Hash a 64-bit address (global instance)
 * @addr: Address to hash
 * Returns: 32-bit hash value
 */
uint32_t refactored_hash_address(uint64_t addr);

/**
 * refactored_translation_cache_lookup - Look up translation (global instance)
 * @guest_pc: Guest ARM64 PC to look up
 * Returns: Host x86_64 code pointer if found, NULL otherwise
 */
void *refactored_translation_cache_lookup(uint64_t guest_pc);

/**
 * refactored_translation_cache_insert - Insert translation (global instance)
 * @guest_pc: Guest ARM64 PC
 * @host_addr: Host x86_64 code address
 * @size: Size of translated code
 */
void refactored_translation_cache_insert(uint64_t guest_pc, void *host_addr, uint32_t size);

/**
 * refactored_code_cache_alloc - Allocate from code cache (global instance)
 * @size: Size in bytes
 * Returns: Pointer to allocated memory or NULL
 */
void *refactored_code_cache_alloc(size_t size);

/**
 * refactored_translation_cache_init - Initialize global translation cache
 * Returns: 0 on success, -1 on error
 */
int refactored_translation_cache_init(void);

/**
 * refactored_translation_cache_cleanup - Cleanup global translation cache
 */
void refactored_translation_cache_cleanup(void);

/* ============================================================================
 * Block Chaining (for direct threaded code)
 * ============================================================================ */

/**
 * trans_cache_chain_blocks - Chain two translation blocks together
 * @from_block: Source block
 * @to_block: Target block
 * @branch_type: Branch type (0=fall-through, 1=taken)
 * Returns: 0 on success, -1 on error
 */
int trans_cache_chain_blocks(trans_cache_entry_t *from_block,
                             trans_cache_entry_t *to_block,
                             int branch_type);

/**
 * trans_cache_unchain_block - Remove chaining from a block
 * @block: Block to unchain
 * Returns: 0 on success, -1 on error
 */
int trans_cache_unchain_block(trans_cache_entry_t *block);

/**
 * refactored_chain_blocks - Chain two blocks (global instance)
 * @guest_from: Source guest PC
 * @guest_to: Target guest PC
 * @branch_type: Branch type
 * Returns: 0 on success, -1 on error
 */
int refactored_chain_blocks(uint64_t guest_from, uint64_t guest_to, int branch_type);

/**
 * refactored_unchain_block - Unchain a block (global instance)
 * @guest_pc: Guest PC of block
 * Returns: 0 on success, -1 on error
 */
int refactored_unchain_block(uint64_t guest_pc);

#endif /* ROSETTA_TRANS_CACHE_H */
