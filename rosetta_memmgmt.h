/*
 * Rosetta Memory Management Header
 *
 * This module provides memory management functions for guest
 * memory mapping, protection, and address translation.
 */

#ifndef ROSETTA_MEMMGMT_H
#define ROSETTA_MEMMGMT_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Memory Mapping Operations
 * ============================================================================ */

/**
 * Map guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest(uint64_t guest, uint64_t size);

/**
 * Map guest memory with specific protection
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags
 * @return Pointer to mapped memory, or NULL on failure
 */
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot);

/**
 * Unmap guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @return 0 on success, -1 on failure
 */
int memory_unmap_guest(uint64_t guest, uint64_t size);

/**
 * Change protection on guest memory region
 * @param guest Guest address
 * @param size Size in bytes
 * @param prot Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
 * @return 0 on success, -1 on failure
 */
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot);

/**
 * Translate guest address to host address
 * @param guest Guest address
 * @return Host address, or NULL if not mapped
 */
void *memory_translate_addr(uint64_t guest);

/**
 * Initialize memory management subsystem
 * @return 0 on success, -1 on failure
 */
int memory_init(void);

/**
 * Clean up memory management subsystem
 */
void memory_cleanup(void);

/* ============================================================================
 * Memory Management Utilities
 * ============================================================================ */

/**
 * Anonymous memory mapping
 * @param size Size of mapping
 * @param prot Protection flags
 * @return Pointer to mapped memory, or MAP_FAILED on error
 */
void *rosetta_mmap_anonymous(size_t size, int prot);

/**
 * Unmap memory region
 * @param addr Address to unmap
 * @param size Size of region
 * @return 0 on success, -1 on failure
 */
int rosetta_munmap_region(void *addr, size_t size);

/**
 * Change protection of memory region
 * @param addr Address of region
 * @param size Size of region
 * @param prot New protection flags
 * @return 0 on success, -1 on failure
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot);

/**
 * Allocate aligned memory
 * @param alignment Alignment requirement
 * @param size Size of allocation
 * @return Pointer to allocated memory, or NULL on failure
 */
void *rosetta_memalign(size_t alignment, size_t size);

/**
 * Zero-fill memory region
 * @param s Memory region to zero
 * @param n Number of bytes
 */
void rosetta_memzero(void *s, size_t n);

#endif /* ROSETTA_MEMMGMT_H */
