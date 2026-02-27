/* ============================================================================
 * Rosetta Refactored - Code Cache Management Implementation
 * ============================================================================
 *
 * This module implements code cache management functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_codecache.h"
#include "rosetta_refactored_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* ============================================================================
 * Global Code Cache State
 * ============================================================================ */

static ros_code_cache_t g_code_cache;
static uint8_t *g_code_buffer = NULL;
static size_t g_code_offset = 0;

/* ============================================================================
 * Code Cache Initialization
 * ============================================================================ */

/**
 * rosetta_code_cache_init - Initialize code cache
 */
int rosetta_code_cache_init(size_t size)
{
    if (g_code_cache.initialized) {
        rosetta_code_cache_cleanup();
    }

    /* Validate size */
    if (size == 0) {
        size = ROS_CODE_CACHE_DEFAULT_SIZE;
    }
    if (size < ROS_CODE_CACHE_MIN_SIZE) {
        size = ROS_CODE_CACHE_MIN_SIZE;
    }
    if (size > ROS_CODE_CACHE_MAX_SIZE) {
        size = ROS_CODE_CACHE_MAX_SIZE;
    }

    /* Allocate executable memory */
    g_code_buffer = (uint8_t *)rosetta_mmap_anonymous(size,
                                                       PROT_READ | PROT_WRITE | PROT_EXEC);
    if (g_code_buffer == NULL || g_code_buffer == MAP_FAILED) {
        fprintf(stderr, "Failed to allocate code cache (size=%zu)\n", size);
        return -1;
    }

    /* Initialize cache descriptor */
    g_code_cache.base = g_code_buffer;
    g_code_cache.size = size;
    g_code_cache.used = 0;
    g_code_cache.free = size;
    g_code_cache.alignment = ROS_CODE_CACHE_ALIGNMENT;
    g_code_cache.block_count = 0;
    g_code_cache.max_blocks = size / 64;  /* Estimate: 64 bytes per block */
    g_code_cache.initialized = true;
    g_code_offset = 0;

    return 0;
}

/**
 * rosetta_code_cache_cleanup - Cleanup code cache
 */
void rosetta_code_cache_cleanup(void)
{
    if (!g_code_cache.initialized) {
        return;
    }

    if (g_code_buffer && g_code_buffer != MAP_FAILED) {
        rosetta_munmap_region(g_code_buffer, g_code_cache.size);
    }

    memset(&g_code_cache, 0, sizeof(g_code_cache));
    g_code_buffer = NULL;
    g_code_offset = 0;
}

/**
 * rosetta_code_cache_is_initialized - Check if cache is initialized
 */
bool rosetta_code_cache_is_initialized(void)
{
    return g_code_cache.initialized;
}

/* ============================================================================
 * Code Block Allocation
 * ============================================================================ */

/**
 * rosetta_code_cache_alloc - Allocate code block
 */
void *rosetta_code_cache_alloc(size_t size)
{
    size_t aligned_size;
    uint8_t *ptr;

    if (!g_code_cache.initialized) {
        if (rosetta_code_cache_init(0) != 0) {
            return NULL;
        }
    }

    /* Align size */
    aligned_size = (size + g_code_cache.alignment - 1) & ~(g_code_cache.alignment - 1);

    /* Check if enough space */
    if (g_code_cache.free < aligned_size) {
        fprintf(stderr, "Code cache full (needed=%zu, free=%zu)\n",
                aligned_size, g_code_cache.free);
        return NULL;
    }

    /* Allocate from current offset */
    ptr = g_code_buffer + g_code_offset;
    g_code_offset += aligned_size;
    g_code_cache.used += aligned_size;
    g_code_cache.free -= aligned_size;
    g_code_cache.block_count++;

    return ptr;
}

/**
 * rosetta_code_cache_alloc_aligned - Allocate aligned code block
 */
void *rosetta_code_cache_alloc_aligned(size_t size, size_t alignment)
{
    size_t aligned_offset;
    size_t aligned_size;
    uint8_t *ptr;

    if (!g_code_cache.initialized) {
        if (rosetta_code_cache_init(0) != 0) {
            return NULL;
        }
    }

    /* Align size and offset */
    aligned_size = (size + alignment - 1) & ~(alignment - 1);
    aligned_offset = (g_code_offset + alignment - 1) & ~(alignment - 1);

    /* Check if enough space */
    if (g_code_cache.free < (aligned_offset - g_code_offset + aligned_size)) {
        return NULL;
    }

    /* Allocate from aligned offset */
    ptr = g_code_buffer + aligned_offset;
    g_code_offset = aligned_offset + aligned_size;
    g_code_cache.used += aligned_size;
    g_code_cache.free -= aligned_size;
    g_code_cache.block_count++;

    return ptr;
}

/**
 * rosetta_code_cache_block_free - Free code block
 */
int rosetta_code_cache_block_free(void *block)
{
    (void)block;  /* Simple bump allocator - individual frees not supported */
    /* In a more sophisticated implementation, we could track and reuse blocks */
    return 0;
}

/**
 * rosetta_code_cache_reset - Reset entire code cache
 */
void rosetta_code_cache_reset(void)
{
    if (!g_code_cache.initialized) {
        return;
    }

    g_code_offset = 0;
    g_code_cache.used = 0;
    g_code_cache.free = g_code_cache.size;
    g_code_cache.block_count = 0;
}

/* ============================================================================
 * Code Block Protection
 * ============================================================================ */

/**
 * rosetta_code_cache_protect - Change code block protection
 */
int rosetta_code_cache_protect(void *block, size_t size, int prot)
{
    int host_prot = 0;

    if (!block || !g_code_cache.initialized) {
        return -1;
    }

    if (prot & ROS_CODE_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_CODE_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_CODE_PROT_EXEC)  host_prot |= PROT_EXEC;

    return mprotect(block, size, host_prot);
}

/**
 * rosetta_code_cache_mark_executable - Mark code block as executable
 */
int rosetta_code_cache_mark_executable(void *block, size_t size)
{
    return rosetta_code_cache_protect(block, size,
                                       ROS_CODE_PROT_READ | ROS_CODE_PROT_EXEC);
}

/**
 * rosetta_code_cache_mark_writable - Mark code block as writable
 */
int rosetta_code_cache_mark_writable(void *block, size_t size)
{
    return rosetta_code_cache_protect(block, size,
                                       ROS_CODE_PROT_READ | PROT_WRITE);
}

/**
 * rosetta_code_cache_flush - Flush instruction cache
 */
void rosetta_code_cache_flush(void *block, size_t size)
{
    (void)block;
    (void)size;

#if defined(__APPLE__)
    /* macOS: sys_icache_invalidate */
    extern void sys_icache_invalidate(void *start, size_t len);
    if (block) {
        sys_icache_invalidate(block, size);
    } else if (g_code_cache.initialized) {
        sys_icache_invalidate(g_code_buffer, g_code_cache.size);
    }
#else
    /* Linux: __builtin___clear_cache */
    if (block) {
        __builtin___clear_cache((char *)block, (char *)block + size);
    } else if (g_code_cache.initialized) {
        __builtin___clear_cache((char *)g_code_buffer,
                                 (char *)g_code_buffer + g_code_cache.size);
    }
#endif
}

/* ============================================================================
 * Code Cache Information
 * ============================================================================ */

/**
 * rosetta_code_cache_get_size - Get total cache size
 */
size_t rosetta_code_cache_get_size(void)
{
    return g_code_cache.size;
}

/**
 * rosetta_code_cache_get_used - Get used space
 */
size_t rosetta_code_cache_get_used(void)
{
    return g_code_cache.used;
}

/**
 * rosetta_code_cache_get_free - Get free space
 */
size_t rosetta_code_cache_get_free(void)
{
    return g_code_cache.free;
}

/**
 * rosetta_code_cache_is_full - Check if cache is full
 */
bool rosetta_code_cache_is_full(size_t min_size)
{
    return g_code_cache.free < min_size;
}

/**
 * rosetta_code_cache_get_block_count - Get number of blocks
 */
uint32_t rosetta_code_cache_get_block_count(void)
{
    return g_code_cache.block_count;
}

/**
 * rosetta_code_cache_get_info - Get cache information
 */
int rosetta_code_cache_get_info(ros_code_cache_t *cache)
{
    if (!cache || !g_code_cache.initialized) {
        return -1;
    }

    *cache = g_code_cache;
    return 0;
}

/* ============================================================================
 * Code Block Management
 * ============================================================================ */

/**
 * rosetta_code_block_init - Initialize code block descriptor
 */
void rosetta_code_block_init(ros_code_block_t *block, uint64_t guest_pc,
                              uint64_t host_pc, uint32_t size)
{
    if (!block) return;

    memset(block, 0, sizeof(*block));
    block->guest_pc = guest_pc;
    block->host_pc = host_pc;
    block->size = size;
    block->flags = ROS_CODE_FLAG_VALID;
    block->hash = (uint32_t)(guest_pc ^ (guest_pc >> 32));
    block->refcount = 1;
}

/**
 * rosetta_code_block_is_valid - Check if block is valid
 */
bool rosetta_code_block_is_valid(ros_code_block_t *block)
{
    return block && (block->flags & ROS_CODE_FLAG_VALID);
}

/**
 * rosetta_code_block_set_flag - Set block flag
 */
void rosetta_code_block_set_flag(ros_code_block_t *block, uint32_t flag)
{
    if (block) {
        block->flags |= flag;
    }
}

/**
 * rosetta_code_block_clear_flag - Clear block flag
 */
void rosetta_code_block_clear_flag(ros_code_block_t *block, uint32_t flag)
{
    if (block) {
        block->flags &= ~flag;
    }
}

/**
 * rosetta_code_block_chain - Chain two blocks together
 */
int rosetta_code_block_chain(ros_code_block_t *from, ros_code_block_t *to, int index)
{
    if (!from || !to || index < 0 || index > 1) {
        return -1;
    }

    from->chain[index] = to->host_pc;
    from->flags |= ROS_CODE_FLAG_LINKED;
    return 0;
}

/**
 * rosetta_code_block_unchain - Unchain a block
 */
int rosetta_code_block_unchain(ros_code_block_t *block)
{
    if (!block) {
        return -1;
    }

    block->chain[0] = 0;
    block->chain[1] = 0;
    block->flags &= ~ROS_CODE_FLAG_LINKED;
    return 0;
}
