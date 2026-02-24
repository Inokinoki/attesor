/* ============================================================================
 * Rosetta Binary Translator - JIT Core and Translation Cache
 * ============================================================================
 *
 * This module implements the JIT compilation core:
 * - Translation cache management (hash-based lookup/insert)
 * - Code cache allocation and management
 * - Translation block lifecycle management
 * - Block chaining for direct-threaded execution
 *
 * ============================================================================ */

#include "rosetta_jit.h"
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

/* Global JIT context (for simplicity in this implementation) */
static jit_context_t g_jit_context;
static bool g_jit_initialized = false;

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a 64-bit address for cache lookup
 *
 * Uses a simple multiplicative hash function optimized for
 * addresses that are typically aligned to 4-byte boundaries.
 */
u32 hash_address(u64 addr)
{
    /* Golden ratio multiplicative hash */
    u64 hash = addr * 2654435761ULL;
    return (u32)(hash >> 32);
}

/**
 * Hash a null-terminated string (DJB2 algorithm)
 *
 * Commonly used for symbol names and file paths.
 */
u32 hash_string(const char *s)
{
    u32 hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/**
 * Compute hash over arbitrary data
 *
 * Uses a simple rolling hash suitable for hashing
 * blocks of code or data structures.
 */
u32 hash_compute(const void *data, size_t len)
{
    const u8 *bytes = (const u8 *)data;
    u32 hash = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        hash = hash * 31 + bytes[i];
    }

    return hash;
}

/* ============================================================================
 * JIT Core Initialization
 * ============================================================================ */

/**
 * Initialize JIT compiler
 */
int jit_init(jit_context_t *ctx, u32 cache_size)
{
    if (!ctx) return ROSETTA_ERR_INVAL;

    /* Default cache size if not specified */
    if (cache_size == 0) {
        cache_size = CODE_CACHE_DEFAULT_SIZE;
    }

    /* Allocate code cache with RW permissions initially */
    ctx->code_cache = (u8 *)mmap(NULL, cache_size,
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ctx->code_cache == MAP_FAILED) {
        ctx->code_cache = NULL;
        return ROSETTA_ERR_NOMEM;
    }

    ctx->code_cache_size = cache_size;
    ctx->code_cache_offset = 0;

    /* Initialize translation cache */
    memset(ctx->cache, 0, sizeof(ctx->cache));
    ctx->cache_insert_index = 0;

    /* Initialize code buffer for emission */
    code_buffer_init(&ctx->emit_buf, ctx->code_cache, cache_size);

    /* Reset statistics */
    ctx->blocks_translated = 0;
    ctx->cache_hits = 0;
    ctx->cache_misses = 0;

    /* Set flags */
    ctx->initialized = true;
    ctx->hot_path = false;

    return ROSETTA_OK;
}

/**
 * Cleanup JIT compiler resources
 */
void jit_cleanup(jit_context_t *ctx)
{
    if (!ctx) return;

    /* Free code cache */
    if (ctx->code_cache) {
        munmap(ctx->code_cache, ctx->code_cache_size);
        ctx->code_cache = NULL;
    }

    ctx->code_cache_size = 0;
    ctx->code_cache_offset = 0;
    ctx->initialized = false;
}

/**
 * Reset JIT state (flush caches, keep allocations)
 */
void jit_reset(jit_context_t *ctx)
{
    if (!ctx) return;

    /* Flush translation cache */
    translation_flush(ctx);

    /* Reset code cache offset (but keep memory mapped) */
    ctx->code_cache_offset = 0;
    code_buffer_init(&ctx->emit_buf, ctx->code_cache, ctx->code_cache_size);

    /* Reset statistics */
    ctx->blocks_translated = 0;
    ctx->cache_hits = 0;
    ctx->cache_misses = 0;
}

/* ============================================================================
 * Translation Cache Management
 * ============================================================================ */

/**
 * Look up translation by guest PC
 *
 * Performs a hash-based lookup to find a cached translation
 * for the given guest PC.
 */
void *translation_lookup(jit_context_t *ctx, u64 guest_pc)
{
    u32 hash = hash_address(guest_pc);
    u32 index = hash & TRANSLATION_CACHE_MASK;

    if (!ctx || !ctx->initialized) return NULL;

    /* Check cache entry */
    if (ctx->cache[index].guest_addr == guest_pc &&
        ctx->cache[index].host_addr != 0) {
        ctx->cache[index].refcount++;
        ctx->cache_hits++;
        return (void *)(uintptr_t)ctx->cache[index].host_addr;
    }

    ctx->cache_misses++;
    return NULL;
}

/**
 * Insert translation into cache
 *
 * Inserts a new guest-to-host translation mapping into
 * the translation cache using direct-mapped placement.
 */
int translation_insert(jit_context_t *ctx, u64 guest, u64 host, size_t size)
{
    u32 hash = hash_address(guest);
    u32 index = hash & TRANSLATION_CACHE_MASK;

    if (!ctx || !ctx->initialized) return ROSETTA_ERR_INVAL;

    /* Insert into cache (simple direct-mapped cache) */
    ctx->cache[index].guest_addr = guest;
    ctx->cache[index].host_addr = host;
    ctx->cache[index].block_size = (u32)size;
    ctx->cache[index].hash = hash;
    ctx->cache[index].refcount = 1;
    ctx->cache[index].flags = BLOCK_FLAG_VALID;
    ctx->cache[index].next = NULL;

    ctx->cache_insert_index++;
    ctx->blocks_translated++;

    return ROSETTA_OK;
}

/**
 * Invalidate translation for guest PC
 */
int translation_invalidate(jit_context_t *ctx, u64 guest_pc)
{
    u32 hash = hash_address(guest_pc);
    u32 index = hash & TRANSLATION_CACHE_MASK;

    if (!ctx || !ctx->initialized) return ROSETTA_ERR_INVAL;

    /* Invalidate cache entry */
    if (ctx->cache[index].guest_addr == guest_pc) {
        ctx->cache[index].guest_addr = 0;
        ctx->cache[index].host_addr = 0;
        ctx->cache[index].refcount = 0;
        ctx->cache[index].flags = 0;
    }

    return ROSETTA_OK;
}

/**
 * Flush entire translation cache
 */
void translation_flush(jit_context_t *ctx)
{
    u32 i;

    if (!ctx || !ctx->initialized) return;

    /* Clear all cache entries */
    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        ctx->cache[i].guest_addr = 0;
        ctx->cache[i].host_addr = 0;
        ctx->cache[i].hash = 0;
        ctx->cache[i].refcount = 0;
        ctx->cache[i].flags = 0;
        ctx->cache[i].next = NULL;
    }

    ctx->cache_insert_index = 0;
}

/* ============================================================================
 * Translation Block Management
 * ============================================================================ */

/**
 * Allocate a new translation block
 */
TranslationBlock *translation_alloc_block(u64 guest_pc)
{
    TranslationBlock *block;

    block = (TranslationBlock *)calloc(1, sizeof(TranslationBlock));
    if (!block) return NULL;

    block->guest_pc = guest_pc;
    block->guest_size = 0;
    block->host_code = NULL;
    block->host_size = 0;
    block->hash = hash_address(guest_pc);
    block->flags = 0;
    block->num_instructions = 0;
    block->successor = NULL;
    block->predecessor = NULL;
    block->execute_count = 0;

    return block;
}

/**
 * Free a translation block
 */
void translation_free_block(TranslationBlock *block)
{
    if (!block) return;

    /* Note: host_code points into JIT code cache, don't free it here */
    free(block);
}

/**
 * Mark translation block as valid
 */
void translation_block_set_valid(TranslationBlock *block)
{
    if (!block) return;
    block->flags |= BLOCK_FLAG_VALID;
}

/**
 * Check if translation block is valid
 */
bool translation_block_is_valid(TranslationBlock *block)
{
    if (!block) return false;
    return (block->flags & BLOCK_FLAG_VALID) != 0;
}

/* ============================================================================
 * Block Chaining (Direct Threaded Code)
 * ============================================================================ */

/**
 * Link two translation blocks (block chaining)
 *
 * Creates a direct jump from the end of from_block to to_block,
 * bypassing the dispatch loop for hot paths.
 */
int translation_chain_blocks(TranslationBlock *from_block, TranslationBlock *to_block)
{
    if (!from_block || !to_block) return ROSETTA_ERR_INVAL;

    /* Set up chaining */
    from_block->successor = to_block;
    to_block->predecessor = from_block;
    from_block->flags |= BLOCK_FLAG_LINKED;

    /* In a full implementation, this would patch the last few bytes
     * of from_block's host code to jump directly to to_block->host_code */

    return ROSETTA_OK;
}

/**
 * Unlink all chains from a block
 */
void translation_unchain_blocks(TranslationBlock *block)
{
    if (!block) return;

    /* Clear successor link */
    if (block->successor) {
        block->successor->predecessor = NULL;
        block->successor = NULL;
    }

    /* Clear predecessor link */
    if (block->predecessor) {
        block->predecessor->successor = NULL;
        block->predecessor = NULL;
    }

    block->flags &= ~BLOCK_FLAG_LINKED;
}

/**
 * Get chained successor block
 */
TranslationBlock *translation_get_successor(TranslationBlock *block)
{
    if (!block) return NULL;
    return block->successor;
}

/* ============================================================================
 * Code Cache Management
 * ============================================================================ */

/**
 * Allocate memory from code cache
 */
u8 *code_cache_alloc(jit_context_t *ctx, u32 size)
{
    u8 *ptr;

    if (!ctx || !ctx->initialized) return NULL;

    /* Check if we have enough space */
    if (ctx->code_cache_offset + size > ctx->code_cache_size) {
        return NULL;  /* Cache full */
    }

    /* Allocate from current position */
    ptr = ctx->code_cache + ctx->code_cache_offset;
    ctx->code_cache_offset += size;

    return ptr;
}

/**
 * Allocate aligned memory from code cache
 */
u8 *code_cache_alloc_aligned(jit_context_t *ctx, u32 size, u32 alignment)
{
    u32 aligned_offset;
    u8 *ptr;

    if (!ctx || !ctx->initialized) return NULL;

    /* Calculate aligned offset */
    aligned_offset = (ctx->code_cache_offset + alignment - 1) & ~(alignment - 1);

    /* Check if we have enough space */
    if (aligned_offset + size > ctx->code_cache_size) {
        return NULL;  /* Cache full */
    }

    /* Allocate from aligned position */
    ptr = ctx->code_cache + aligned_offset;
    ctx->code_cache_offset = aligned_offset + size;

    return ptr;
}

/**
 * Mark code cache region as executable
 *
 * Changes protection of code cache region to RX (read + execute).
 */
int code_cache_mark_executable(jit_context_t *ctx, u32 offset, u32 size)
{
    u8 *addr;
    long page_size;
    u32 aligned_offset, aligned_size;

    if (!ctx || !ctx->initialized) return ROSETTA_ERR_INVAL;

    page_size = sysconf(_SC_PAGESIZE);
    if (page_size < 0) page_size = 4096;

    /* Align to page boundaries */
    aligned_offset = offset & ~(page_size - 1);
    aligned_size = ((offset + size) - aligned_offset + page_size - 1) & ~(page_size - 1);

    addr = ctx->code_cache + aligned_offset;

    /* Change protection to RX */
    if (mprotect(addr, aligned_size, PROT_READ | PROT_EXEC) != 0) {
        return ROSETTA_ERR_FAULT;
    }

    return ROSETTA_OK;
}

/**
 * Get free space in code cache
 */
u32 code_cache_get_free_space(jit_context_t *ctx)
{
    if (!ctx || !ctx->initialized) return 0;
    return ctx->code_cache_size - ctx->code_cache_offset;
}

/**
 * Reset code cache to initial state
 */
void code_cache_reset(jit_context_t *ctx)
{
    if (!ctx || !ctx->initialized) return;
    ctx->code_cache_offset = 0;
}

/* ============================================================================
 * Translation Entry Points
 * ============================================================================ */

/**
 * Translate ARM64 basic block to x86_64
 *
 * Main translation entry point. Decodes ARM64 instructions
 * and emits equivalent x86_64 machine code.
 */
void *translate_block(jit_context_t *ctx, u64 guest_pc)
{
    void *cached;
    u8 *code_start;
    u32 code_size;

    if (!ctx || !ctx->initialized) return NULL;

    /* Check translation cache first */
    cached = translation_lookup(ctx, guest_pc);
    if (cached) {
        return cached;
    }

    /* Initialize code buffer at current code cache position */
    ctx->current_guest_pc = guest_pc;
    code_buffer_init(&ctx->emit_buf,
                     ctx->code_cache + ctx->code_cache_offset,
                     ctx->code_cache_size - ctx->code_cache_offset);

    code_start = ctx->emit_buf.buffer;

    /* Emit prologue (save guest state, setup frame) */
    emit_push_reg(&ctx->emit_buf, X86_RBP);
    emit_mov_reg_reg(&ctx->emit_buf, X86_RBP, X86_RSP);

    /* TODO: Full ARM64 instruction decoding and translation */
    /* For now, emit a simple return */
    emit_ud2(&ctx->emit_buf);  /* Undefined instruction as placeholder */

    /* Emit epilogue (restore guest state, teardown frame) */
    emit_mov_reg_reg(&ctx->emit_buf, X86_RSP, X86_RBP);
    emit_pop_reg(&ctx->emit_buf, X86_RBP);
    emit_ret(&ctx->emit_buf);

    code_size = code_buffer_get_size(&ctx->emit_buf);

    /* Mark code as executable */
    code_cache_mark_executable(ctx,
                               (u32)(code_start - ctx->code_cache),
                               code_size);

    /* Update code cache offset */
    ctx->code_cache_offset += code_size;

    /* Insert into translation cache */
    translation_insert(ctx, guest_pc, (u64)(uintptr_t)code_start, code_size);

    return code_start;
}

/**
 * Fast path translation (lookup-only, no re-translation)
 */
void *translate_block_fast(jit_context_t *ctx, u64 guest_pc)
{
    if (!ctx || !ctx->initialized) return NULL;
    return translation_lookup(ctx, guest_pc);
}

/**
 * Execute translated block
 *
 * Looks up or translates a block, then executes it.
 * Returns the next guest PC to execute.
 */
u64 jit_execute(jit_context_t *ctx, u64 guest_pc, thread_state_t *state)
{
    void (*host_func)(void);

    if (!ctx || !ctx->initialized) return 0;

    /* Look up or translate */
    host_func = (void (*)(void))translate_block(ctx, guest_pc);
    if (!host_func) {
        return 0;  /* Translation failed */
    }

    /* TODO: Set up guest state in host registers */
    /* TODO: Call the translated function */
    /* TODO: Retrieve next guest PC from return value */

    /* For now, return 0 to indicate end of execution */
    return 0;
}

/* ============================================================================
 * Statistics and Debugging
 * ============================================================================ */

/**
 * Get translation cache size
 */
u32 translation_cache_get_size(jit_context_t *ctx)
{
    u32 i, count = 0;

    if (!ctx || !ctx->initialized) return 0;

    for (i = 0; i < TRANSLATION_CACHE_SIZE; i++) {
        if (ctx->cache[i].guest_addr != 0 &&
            ctx->cache[i].host_addr != 0) {
            count++;
        }
    }

    return count;
}

/**
 * Check if translation cache is full
 */
bool translation_cache_is_full(jit_context_t *ctx)
{
    return translation_cache_get_size(ctx) >= TRANSLATION_CACHE_SIZE;
}

/**
 * Get JIT statistics
 */
void jit_get_stats(jit_context_t *ctx, u32 *blocks_translated,
                   u32 *cache_hits, u32 *cache_misses)
{
    if (!ctx) return;

    if (blocks_translated) *blocks_translated = ctx->blocks_translated;
    if (cache_hits) *cache_hits = ctx->cache_hits;
    if (cache_misses) *cache_misses = ctx->cache_misses;
}

/* ============================================================================
 * Global JIT Instance (for backward compatibility)
 * ============================================================================ */

/**
 * Get global JIT context
 */
jit_context_t *jit_get_global_context(void)
{
    return &g_jit_context;
}

/**
 * Initialize global JIT instance
 */
int jit_init_global(u32 cache_size)
{
    if (g_jit_initialized) return ROSETTA_OK;

    if (jit_init(&g_jit_context, cache_size) != ROSETTA_OK) {
        return ROSETTA_ERR_NOMEM;
    }

    g_jit_initialized = true;
    return ROSETTA_OK;
}

/**
 * Cleanup global JIT instance
 */
void jit_cleanup_global(void)
{
    if (!g_jit_initialized) return;

    jit_cleanup(&g_jit_context);
    g_jit_initialized = false;
}
