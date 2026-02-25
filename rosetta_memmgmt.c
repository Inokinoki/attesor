/*
 * Rosetta Memory Management
 *
 * This module provides memory management functions for guest
 * memory mapping, protection, and address translation.
 */

#define _DARWIN_C_SOURCE  /* For posix_memalign on macOS */

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>

/* ============================================================================
 * Guest Memory State
 * ============================================================================ */

/* Guest memory mapping state */
static void *guest_memory_base = NULL;
static size_t guest_memory_size = 0;

/* ============================================================================
 * Memory Mapping Operations
 * ============================================================================ */

/**
 * memory_map_guest - Map guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest(uint64_t guest, uint64_t size)
{
    void *ret = mmap((void *)(uintptr_t)guest, size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/**
 * memory_map_guest_with_prot - Map guest memory with specific protection
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot)
{
    void *ret = mmap((void *)(uintptr_t)guest, size, prot,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (ret == MAP_FAILED) {
        return NULL;
    }
    if (guest_memory_base == NULL) {
        guest_memory_base = ret;
        guest_memory_size = size;
    }
    return ret;
}

/**
 * memory_unmap_guest - Unmap guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int memory_unmap_guest(uint64_t guest, uint64_t size)
{
    int ret = munmap((void *)(uintptr_t)guest, size);
    if (ret < 0) {
        return -1;
    }
    if ((uintptr_t)guest == (uintptr_t)guest_memory_base) {
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
    return 0;
}

/**
 * memory_protect_guest - Change protection on guest memory region
 *
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
 * @return 0 on success, -1 on failure
 */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot)
{
    int ret = mprotect((void *)(uintptr_t)guest, size, prot);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

/**
 * memory_translate_addr - Translate guest address to host address
 *
 * @param guest Guest address
 * @return Host address, or NULL if not mapped
 */
void *memory_translate_addr(uint64_t guest)
{
    /* Simple identity mapping for now */
    /* In a full implementation, this would look up the mapping */
    if (guest_memory_base != NULL &&
        guest >= (uintptr_t)guest_memory_base &&
        guest < (uintptr_t)guest_memory_base + guest_memory_size) {
        return (void *)(uintptr_t)guest;
    }
    return NULL;
}

/**
 * memory_init - Initialize memory management subsystem
 *
 * @return 0 on success, -1 on failure
 */
int memory_init(void)
{
    guest_memory_base = NULL;
    guest_memory_size = 0;
    return 0;
}

/**
 * memory_cleanup - Clean up memory management subsystem
 */
void memory_cleanup(void)
{
    if (guest_memory_base != NULL) {
        munmap(guest_memory_base, guest_memory_size);
        guest_memory_base = NULL;
        guest_memory_size = 0;
    }
}

/* ============================================================================
 * Memory Management Utilities
 * ============================================================================ */

/**
 * rosetta_mmap_anonymous - Anonymous memory mapping
 *
 * @param size Size of mapping
 * @param prot Protection flags
 * @return Pointer to mapped memory, or MAP_FAILED on error
 */
void *rosetta_mmap_anonymous(size_t size, int prot)
{
    return mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/**
 * rosetta_munmap_region - Unmap memory region
 *
 * @param addr Address to unmap
 * @param size Size of region
 * @return 0 on success, -1 on failure
 */
int rosetta_munmap_region(void *addr, size_t size)
{
    return munmap(addr, size);
}

/**
 * rosetta_mprotect_region - Change protection of memory region
 *
 * @param addr Address of region
 * @param size Size of region
 * @param prot New protection flags
 * @return 0 on success, -1 on failure
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot)
{
    return mprotect(addr, size, prot);
}

/**
 * rosetta_memalign - Allocate aligned memory
 *
 * @param alignment Alignment requirement
 * @param size Size of allocation
 * @return Pointer to allocated memory, or NULL on failure
 */
void *rosetta_memalign(size_t alignment, size_t size)
{
    void *ptr;
    int ret;

    ret = posix_memalign(&ptr, alignment, size);
    if (ret != 0) {
        return NULL;
    }
    return ptr;
}

/**
 * rosetta_memzero - Zero-fill memory region
 *
 * @param s Memory region to zero
 * @param n Number of bytes
 */
void rosetta_memzero(void *s, size_t n)
{
    volatile uint8_t *p = (volatile uint8_t *)s;
    while (n--) {
        *p++ = 0;
    }
}
