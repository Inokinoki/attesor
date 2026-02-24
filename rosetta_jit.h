#ifndef ROSETTA_JIT_H
#define ROSETTA_JIT_H

/* ============================================================================
 * Rosetta Binary Translator - JIT Core and Translation Cache
 * ============================================================================ */

#include "rosetta_types.h"
#include "rosetta_codegen.h"

/* ============================================================================
 * Translation Cache Configuration
 * ============================================================================ */

#define TRANSLATION_CACHE_BITS    12      /* 4096 entries */
#define TRANSLATION_CACHE_SIZE    (1U << TRANSLATION_CACHE_BITS)
#define TRANSLATION_CACHE_MASK    (TRANSLATION_CACHE_SIZE - 1)

#define CODE_CACHE_DEFAULT_SIZE   (16 * 1024 * 1024)  /* 16MB code cache */
#define CODE_CACHE_PAGE_SIZE      4096

/* Translation block flags */
#define BLOCK_FLAG_VALID          0x01
#define BLOCK_FLAG_HOT            0x02
#define BLOCK_FLAG_LINKED         0x04
#define BLOCK_FLAG_SYSCALL        0x08

/* ============================================================================
 * Translation Cache Entry
 * ============================================================================ */

typedef struct translation_cache_entry {
    u64 guest_addr;                     /* Guest ARM64 PC */
    u64 host_addr;                      /* Host x86_64 code address */
    u32 block_size;                     /* Size of translated block in bytes */
    u32 hash;                           /* Hash of guest address */
    u32 refcount;                       /* Reference count for LRU */
    u32 flags;                          /* Block flags */
    struct translation_cache_entry *next;  /* Next entry for chaining */
} TranslationCacheEntry;

/* ============================================================================
 * Translation Block Structure
 * ============================================================================ */

typedef struct translation_block {
    u64 guest_pc;                       /* Guest PC this block translates */
    u64 guest_size;                     /* Size of guest basic block */
    u8 *host_code;                      /* Pointer to translated code */
    u32 host_size;                      /* Size of translated code */
    u32 hash;                           /* Hash of guest PC */
    u32 flags;                          /* Block flags */
    u32 num_instructions;               /* Number of guest instructions */

    /* Block chaining for fast dispatch */
    struct translation_block *successor; /* Next block in chain */
    struct translation_block *predecessor; /* Previous block */

    /* Statistics (optional, for profiling) */
    u32 execute_count;                  /* Number of times executed */
} TranslationBlock;

/* ============================================================================
 * JIT Context / State
 * ============================================================================ */

typedef struct jit_context {
    /* Code cache */
    u8 *code_cache;                     /* JIT code cache */
    u32 code_cache_size;                /* Total code cache size */
    u32 code_cache_offset;              /* Current write position */

    /* Translation cache */
    TranslationCacheEntry cache[TRANSLATION_CACHE_SIZE];
    u32 cache_insert_index;             /* Round-robin insert index */

    /* Current translation state */
    code_buffer_t emit_buf;             /* Code emission buffer */
    u64 current_guest_pc;               /* PC being translated */

    /* Statistics */
    u32 blocks_translated;              /* Total blocks translated */
    u32 cache_hits;                     /* Translation cache hits */
    u32 cache_misses;                   /* Translation cache misses */

    /* Flags */
    bool initialized;                   /* JIT initialized */
    bool hot_path;                      /* Using fast path translation */
} jit_context_t;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a 64-bit address for cache lookup
 * @param addr Address to hash
 * @return 32-bit hash value
 */
u32 hash_address(u64 addr);

/**
 * Hash a null-terminated string (DJB2 algorithm)
 * @param s String to hash
 * @return 32-bit hash value
 */
u32 hash_string(const char *s);

/**
 * Compute hash over arbitrary data
 * @param data Data to hash
 * @param len Length of data
 * @return 32-bit hash value
 */
u32 hash_compute(const void *data, size_t len);

/* ============================================================================
 * JIT Core Initialization
 * ============================================================================ */

/**
 * Initialize JIT compiler
 * @param ctx JIT context to initialize
 * @param cache_size Code cache size (0 for default)
 * @return ROSETTA_OK on success
 */
int jit_init(jit_context_t *ctx, u32 cache_size);

/**
 * Cleanup JIT compiler resources
 * @param ctx JIT context to cleanup
 */
void jit_cleanup(jit_context_t *ctx);

/**
 * Reset JIT state (flush caches, keep allocations)
 * @param ctx JIT context to reset
 */
void jit_reset(jit_context_t *ctx);

/* ============================================================================
 * Translation Cache Management
 * ============================================================================ */

/**
 * Look up translation by guest PC
 * @param ctx JIT context
 * @param guest_pc Guest ARM64 PC to look up
 * @return Host x86_64 code pointer if found, NULL otherwise
 */
void *translation_lookup(jit_context_t *ctx, u64 guest_pc);

/**
 * Insert translation into cache
 * @param ctx JIT context
 * @param guest Guest ARM64 PC
 * @param host Host x86_64 code address
 * @param size Size of translated block
 * @return ROSETTA_OK on success
 */
int translation_insert(jit_context_t *ctx, u64 guest, u64 host, size_t size);

/**
 * Invalidate translation for guest PC
 * @param ctx JIT context
 * @param guest_pc Guest PC to invalidate
 * @return ROSETTA_OK on success
 */
int translation_invalidate(jit_context_t *ctx, u64 guest_pc);

/**
 * Flush entire translation cache
 * @param ctx JIT context
 */
void translation_flush(jit_context_t *ctx);

/* ============================================================================
 * Translation Block Management
 * ============================================================================ */

/**
 * Allocate a new translation block
 * @param guest_pc Guest PC for this block
 * @return New translation block or NULL on failure
 */
TranslationBlock *translation_alloc_block(u64 guest_pc);

/**
 * Free a translation block
 * @param block Block to free
 */
void translation_free_block(TranslationBlock *block);

/**
 * Mark translation block as valid
 * @param block Block to mark valid
 */
void translation_block_set_valid(TranslationBlock *block);

/**
 * Check if translation block is valid
 * @param block Block to check
 * @return true if valid
 */
bool translation_block_is_valid(TranslationBlock *block);

/* ============================================================================
 * Block Chaining (Direct Threaded Code)
 * ============================================================================ */

/**
 * Link two translation blocks (block chaining)
 * @param from_block Source block
 * @param to_block Target block (successor)
 * @return ROSETTA_OK on success
 */
int translation_chain_blocks(TranslationBlock *from_block, TranslationBlock *to_block);

/**
 * Unlink all chains from a block
 * @param block Block to unlink
 */
void translation_unchain_blocks(TranslationBlock *block);

/**
 * Get chained successor block
 * @param block Block to query
 * @return Successor block or NULL
 */
TranslationBlock *translation_get_successor(TranslationBlock *block);

/* ============================================================================
 * Code Cache Management
 * ============================================================================ */

/**
 * Allocate memory from code cache
 * @param ctx JIT context
 * @param size Size in bytes
 * @return Pointer to allocated memory or NULL
 */
u8 *code_cache_alloc(jit_context_t *ctx, u32 size);

/**
 * Allocate aligned memory from code cache
 * @param ctx JIT context
 * @param size Size in bytes
 * @param alignment Alignment requirement
 * @return Pointer to aligned memory
 */
u8 *code_cache_alloc_aligned(jit_context_t *ctx, u32 size, u32 alignment);

/**
 * Mark code cache region as executable
 * @param ctx JIT context
 * @param offset Start offset in code cache
 * @param size Size of region
 * @return ROSETTA_OK on success
 */
int code_cache_mark_executable(jit_context_t *ctx, u32 offset, u32 size);

/**
 * Get free space in code cache
 * @param ctx JIT context
 * @return Bytes remaining
 */
u32 code_cache_get_free_space(jit_context_t *ctx);

/**
 * Reset code cache to initial state
 * @param ctx JIT context
 */
void code_cache_reset(jit_context_t *ctx);

/* ============================================================================
 * Translation Entry Points
 * ============================================================================ */

/**
 * Translate ARM64 basic block to x86_64
 * @param ctx JIT context
 * @param guest_pc Guest ARM64 PC to translate
 * @return Pointer to translated x86_64 code
 */
void *translate_block(jit_context_t *ctx, u64 guest_pc);

/**
 * Fast path translation (lookup-only, no re-translation)
 * @param ctx JIT context
 * @param guest_pc Guest ARM64 PC
 * @return Cached translation or NULL
 */
void *translate_block_fast(jit_context_t *ctx, u64 guest_pc);

/**
 * Execute translated block
 * @param ctx JIT context
 * @param guest_pc Guest PC to execute
 * @param state Thread state
 * @return Next guest PC or 0 on exit
 */
u64 jit_execute(jit_context_t *ctx, u64 guest_pc, thread_state_t *state);

/* ============================================================================
 * Statistics and Debugging
 * ============================================================================ */

/**
 * Get translation cache size
 * @param ctx JIT context
 * @return Number of valid entries
 */
u32 translation_cache_get_size(jit_context_t *ctx);

/**
 * Check if translation cache is full
 * @param ctx JIT context
 * @return true if cache is full
 */
bool translation_cache_is_full(jit_context_t *ctx);

/**
 * Get JIT statistics
 * @param ctx JIT context
 * @param blocks_translated Output: blocks translated
 * @param cache_hits Output: cache hits
 * @param cache_misses Output: cache misses
 */
void jit_get_stats(jit_context_t *ctx, u32 *blocks_translated,
                   u32 *cache_hits, u32 *cache_misses);

/* ============================================================================
 * Global JIT Instance (for backward compatibility)
 * ============================================================================ */

/**
 * Get global JIT context
 * @return Pointer to global JIT context
 */
jit_context_t *jit_get_global_context(void);

/**
 * Initialize global JIT instance
 * @param cache_size Size of code cache (0 for default)
 * @return ROSETTA_OK on success
 */
int jit_init_global(u32 cache_size);

/**
 * Cleanup global JIT instance
 */
void jit_cleanup_global(void);

#endif /* ROSETTA_JIT_H */
