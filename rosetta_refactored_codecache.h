/* ============================================================================
 * Rosetta Refactored - Code Cache Management Header
 * ============================================================================
 *
 * This module provides code cache management functions for the Rosetta
 * translation layer, including allocation, deallocation, and protection.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_CODECACHE_H
#define ROSETTA_REFACTORED_CODECACHE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * Code Cache Configuration
 * ============================================================================ */

/* Default code cache size (16MB) */
#define ROS_CODE_CACHE_DEFAULT_SIZE  (16 * 1024 * 1024)

/* Minimum code cache size (1MB) */
#define ROS_CODE_CACHE_MIN_SIZE      (1 * 1024 * 1024)

/* Maximum code cache size (256MB) */
#define ROS_CODE_CACHE_MAX_SIZE      (256 * 1024 * 1024)

/* Code cache alignment */
#define ROS_CODE_CACHE_ALIGNMENT     4096

/* Code cache protection flags */
#define ROS_CODE_PROT_NONE   0x0
#define ROS_CODE_PROT_READ   0x1
#define ROS_CODE_PROT_WRITE  0x2
#define ROS_CODE_PROT_EXEC   0x4

/* Code block flags */
#define ROS_CODE_FLAG_NONE      0x00
#define ROS_CODE_FLAG_VALID     0x01
#define ROS_CODE_FLAG_HOT       0x02
#define ROS_CODE_FLAG_LINKED    0x04
#define ROS_CODE_FLAG_DIRTY     0x08

/* ============================================================================
 * Code Cache Structures
 * ============================================================================ */

/* Code block descriptor */
typedef struct {
    uint64_t guest_pc;      /* Guest PC */
    uint64_t host_pc;       /* Host PC (translated code) */
    uint32_t size;          /* Size of translated code */
    uint32_t flags;         /* Code block flags */
    uint32_t hash;          /* Hash of guest PC */
    uint32_t refcount;      /* Reference count */
    uint64_t chain[2];      /* Chained successor blocks */
} ros_code_block_t;

/* Code cache descriptor */
typedef struct {
    void *base;             /* Base address of code cache */
    size_t size;            /* Total size of code cache */
    size_t used;            /* Used space */
    size_t free;            /* Free space */
    size_t alignment;       /* Alignment requirement */
    uint32_t block_count;   /* Number of blocks */
    uint32_t max_blocks;    /* Maximum blocks */
    bool initialized;       /* Cache initialized */
} ros_code_cache_t;

/* ============================================================================
 * Code Cache Initialization
 * ============================================================================ */

/**
 * rosetta_code_cache_init - Initialize code cache
 * @size: Initial cache size (0 for default)
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_init(size_t size);

/**
 * rosetta_code_cache_cleanup - Cleanup code cache
 */
void rosetta_code_cache_cleanup(void);

/**
 * rosetta_code_cache_is_initialized - Check if cache is initialized
 * Returns: true if initialized, false otherwise
 */
bool rosetta_code_cache_is_initialized(void);

/* ============================================================================
 * Code Block Allocation
 * ============================================================================ */

/**
 * rosetta_code_cache_alloc - Allocate code block
 * @size: Required size
 * Returns: Pointer to allocated code, or NULL on error
 */
void *rosetta_code_cache_alloc(size_t size);

/**
 * rosetta_code_cache_alloc_aligned - Allocate aligned code block
 * @size: Required size
 * @alignment: Required alignment
 * Returns: Pointer to allocated code, or NULL on error
 */
void *rosetta_code_cache_alloc_aligned(size_t size, size_t alignment);

/**
 * rosetta_code_cache_block_free - Free code block
 * @block: Code block to free
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_block_free(void *block);

/**
 * rosetta_code_cache_reset - Reset entire code cache
 */
void rosetta_code_cache_reset(void);

/* ============================================================================
 * Code Block Protection
 * ============================================================================ */

/**
 * rosetta_code_cache_protect - Change code block protection
 * @block: Code block
 * @size: Block size
 * @prot: Protection flags
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_protect(void *block, size_t size, int prot);

/**
 * rosetta_code_cache_mark_executable - Mark code block as executable
 * @block: Code block
 * @size: Block size
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_mark_executable(void *block, size_t size);

/**
 * rosetta_code_cache_mark_writable - Mark code block as writable
 * @block: Code block
 * @size: Block size
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_mark_writable(void *block, size_t size);

/**
 * rosetta_code_cache_flush - Flush instruction cache
 * @block: Code block (NULL for entire cache)
 * @size: Block size
 */
void rosetta_code_cache_flush(void *block, size_t size);

/* ============================================================================
 * Code Cache Information
 * ============================================================================ */

/**
 * rosetta_code_cache_get_size - Get total cache size
 * Returns: Total cache size in bytes
 */
size_t rosetta_code_cache_get_size(void);

/**
 * rosetta_code_cache_get_used - Get used space
 * Returns: Used space in bytes
 */
size_t rosetta_code_cache_get_used(void);

/**
 * rosetta_code_cache_get_free - Get free space
 * Returns: Free space in bytes
 */
size_t rosetta_code_cache_get_free(void);

/**
 * rosetta_code_cache_is_full - Check if cache is full
 * @min_size: Minimum required size
 * Returns: true if not enough space, false otherwise
 */
bool rosetta_code_cache_is_full(size_t min_size);

/**
 * rosetta_code_cache_get_block_count - Get number of blocks
 * Returns: Number of code blocks
 */
uint32_t rosetta_code_cache_get_block_count(void);

/**
 * rosetta_code_cache_get_info - Get cache information
 * @cache: Cache descriptor to fill
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_get_info(ros_code_cache_t *cache);

/* ============================================================================
 * Code Block Management
 * ============================================================================ */

/**
 * rosetta_code_block_init - Initialize code block descriptor
 * @block: Block descriptor
 * @guest_pc: Guest PC
 * @host_pc: Host PC
 * @size: Block size
 */
void rosetta_code_block_init(ros_code_block_t *block, uint64_t guest_pc,
                              uint64_t host_pc, uint32_t size);

/**
 * rosetta_code_block_is_valid - Check if block is valid
 * @block: Block descriptor
 * Returns: true if valid, false otherwise
 */
bool rosetta_code_block_is_valid(ros_code_block_t *block);

/**
 * rosetta_code_block_set_flag - Set block flag
 * @block: Block descriptor
 * @flag: Flag to set
 */
void rosetta_code_block_set_flag(ros_code_block_t *block, uint32_t flag);

/**
 * rosetta_code_block_clear_flag - Clear block flag
 * @block: Block descriptor
 * @flag: Flag to clear
 */
void rosetta_code_block_clear_flag(ros_code_block_t *block, uint32_t flag);

/**
 * rosetta_code_block_chain - Chain two blocks together
 * @from: Source block
 * @to: Target block
 * @index: Chain index (0 or 1)
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_block_chain(ros_code_block_t *from, ros_code_block_t *to, int index);

/**
 * rosetta_code_block_unchain - Unchain a block
 * @block: Block to unchain
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_block_unchain(ros_code_block_t *block);

#endif /* ROSETTA_REFACTORED_CODECACHE_H */
