/* ============================================================================
 * Rosetta Refactored - Memory Management Implementation
 * ============================================================================
 *
 * This module implements memory management functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_memory.h"
#include "rosetta_refactored_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/* ============================================================================
 * Memory Mapping Functions
 * ============================================================================ */

/**
 * rosetta_memory_map - Map guest memory region
 */
void *rosetta_memory_map(uint64_t guest_addr, size_t size, int prot,
                         int flags, int fd, int64_t offset)
{
    int host_prot = 0;

    /* Convert protection flags */
    if (prot & ROS_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_PROT_EXEC)  host_prot |= PROT_EXEC;

    void *addr = mmap((void *)(uintptr_t)guest_addr, size, host_prot,
                      flags, fd, (off_t)offset);

    if (addr == MAP_FAILED) {
        fprintf(stderr, "Failed to map memory at 0x%llx (size=%zu)\n",
                (unsigned long long)guest_addr, size);
        return NULL;
    }

    return addr;
}

/**
 * rosetta_memory_unmap - Unmap guest memory region
 */
int rosetta_memory_unmap(uint64_t guest_addr, size_t size)
{
    (void)guest_addr;  /* Address is ignored on macOS */
    return munmap((void *)(uintptr_t)guest_addr, size);
}

/**
 * rosetta_memory_protect - Change protection of guest memory region
 */
int rosetta_memory_protect(uint64_t guest_addr, size_t size, int prot)
{
    int host_prot = 0;

    if (prot & ROS_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_PROT_EXEC)  host_prot |= PROT_EXEC;

    return mprotect((void *)(uintptr_t)guest_addr, size, host_prot);
}

/* ============================================================================
 * Code Cache Functions
 * ============================================================================ */

/**
 * rosetta_code_cache_alloc - Allocate executable code cache
 */
void *rosetta_code_cache_alloc(size_t size)
{
    void *code = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (code == MAP_FAILED) {
        return NULL;
    }

    return code;
}

/**
 * rosetta_code_cache_free - Free executable code cache
 */
int rosetta_code_cache_free(void *code, size_t size)
{
    return munmap(code, size);
}

/**
 * rosetta_code_cache_protect - Change protection of code cache
 */
int rosetta_code_cache_protect(void *addr, size_t size, int prot)
{
    int host_prot = 0;

    if (prot & ROS_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_PROT_EXEC)  host_prot |= PROT_EXEC;

    return mprotect(addr, size, host_prot);
}

/* ============================================================================
 * Anonymous Mapping Functions
 * ============================================================================ */

/**
 * rosetta_mmap_anonymous - Map anonymous memory region
 */
void *rosetta_mmap_anonymous(size_t size, int prot)
{
    int host_prot = 0;

    if (prot & ROS_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_PROT_EXEC)  host_prot |= PROT_EXEC;

    void *addr = mmap(NULL, size, host_prot,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
        return NULL;
    }

    return addr;
}

/**
 * rosetta_munmap_region - Unmap memory region
 */
int rosetta_munmap_region(void *addr, size_t size)
{
    return munmap(addr, size);
}

/**
 * rosetta_mprotect_region - Change protection of memory region
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot)
{
    int host_prot = 0;

    if (prot & ROS_PROT_READ)  host_prot |= PROT_READ;
    if (prot & ROS_PROT_WRITE) host_prot |= PROT_WRITE;
    if (prot & ROS_PROT_EXEC)  host_prot |= PROT_EXEC;

    return mprotect(addr, size, host_prot);
}

/* ============================================================================
 * Aligned Memory Allocation
 * ============================================================================ */

/**
 * rosetta_memalign - Allocate aligned memory
 */
void *rosetta_memalign(size_t alignment, size_t size)
{
    void *ptr;

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
#else
    /* Fallback: allocate extra and align manually */
    size_t extra = alignment + sizeof(void *);
    void *base = malloc(size + extra);
    if (!base) {
        return NULL;
    }

    /* Calculate aligned address */
    uintptr_t base_addr = (uintptr_t)base + extra;
    uintptr_t aligned_addr = (base_addr + (alignment - 1)) & ~(alignment - 1);

    /* Store original pointer just before aligned address */
    ((void **)aligned_addr)[-1] = base;

    return (void *)aligned_addr;
#endif
}

/**
 * rosetta_memzero - Zero-fill memory region
 */
void rosetta_memzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/* ============================================================================
 * Memory Operations
 * ============================================================================ */

/**
 * rosetta_memory_copy - Copy memory region
 */
void *rosetta_memory_copy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

/**
 * rosetta_memory_move - Move memory region (overlapping safe)
 */
void *rosetta_memory_move(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

/**
 * rosetta_memory_set - Set memory region to value
 */
void *rosetta_memory_set(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

/**
 * rosetta_memory_cmp - Compare memory regions
 */
int rosetta_memory_cmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

/**
 * rosetta_memory_find - Find byte in memory
 */
void *rosetta_memory_find(const void *s, int c, size_t n)
{
    return memchr(s, c, n);
}
