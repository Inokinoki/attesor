/* ============================================================================
 * Rosetta Refactored - Memory Management Header
 * ============================================================================
 *
 * This module provides memory management functions for the Rosetta
 * translation layer, including mapping, unmapping, and protection.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_MEMORY_H
#define ROSETTA_REFACTORED_MEMORY_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Memory Protection Flags
 * ============================================================================ */

#define ROS_PROT_NONE   0x0
#define ROS_PROT_READ   0x1
#define ROS_PROT_WRITE  0x2
#define ROS_PROT_EXEC   0x4

/* ============================================================================
 * Memory Mapping Flags
 * ============================================================================ */

#define ROS_MAP_ANONYMOUS   0x20
#define ROS_MAP_PRIVATE     0x02
#define ROS_MAP_SHARED      0x01
#define ROS_MAP_FIXED       0x10

/* ============================================================================
 * Memory Management Functions
 * ============================================================================ */

/**
 * rosetta_memory_map - Map guest memory region
 * @guest_addr: Guest address
 * @size: Region size
 * @prot: Protection flags
 * @flags: Mapping flags
 * @fd: File descriptor (for file mappings)
 * @offset: Offset in file
 * Returns: Pointer to mapped region, or MAP_FAILED on error
 */
void *rosetta_memory_map(uint64_t guest_addr, size_t size, int prot,
                         int flags, int fd, int64_t offset);

/**
 * rosetta_memory_unmap - Unmap guest memory region
 * @guest_addr: Guest address
 * @size: Region size
 * Returns: 0 on success, -1 on error
 */
int rosetta_memory_unmap(uint64_t guest_addr, size_t size);

/**
 * rosetta_memory_protect - Change protection of guest memory region
 * @guest_addr: Guest address
 * @size: Region size
 * @prot: New protection flags
 * Returns: 0 on success, -1 on error
 */
int rosetta_memory_protect(uint64_t guest_addr, size_t size, int prot);

/**
 * rosetta_code_cache_alloc - Allocate executable code cache
 * @size: Required size
 * Returns: Pointer to allocated code region, or NULL on error
 */
void *rosetta_code_cache_alloc(size_t size);

/**
 * rosetta_code_cache_free - Free executable code cache
 * @code: Pointer to code region
 * @size: Region size
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_free(void *code, size_t size);

/**
 * rosetta_code_cache_protect - Change protection of code cache
 * @addr: Code address
 * @size: Region size
 * @prot: New protection flags
 * Returns: 0 on success, -1 on error
 */
int rosetta_code_cache_protect(void *addr, size_t size, int prot);

/**
 * rosetta_mmap_anonymous - Map anonymous memory region
 * @size: Region size
 * @prot: Protection flags
 * Returns: Pointer to mapped region, or MAP_FAILED on error
 */
void *rosetta_mmap_anonymous(size_t size, int prot);

/**
 * rosetta_munmap_region - Unmap memory region
 * @addr: Address to unmap
 * @size: Region size
 * Returns: 0 on success, -1 on error
 */
int rosetta_munmap_region(void *addr, size_t size);

/**
 * rosetta_mprotect_region - Change protection of memory region
 * @addr: Address
 * @size: Region size
 * @prot: New protection flags
 * Returns: 0 on success, -1 on error
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot);

/**
 * rosetta_memalign - Allocate aligned memory
 * @alignment: Required alignment (power of 2)
 * @size: Required size
 * Returns: Pointer to allocated memory, or NULL on error
 */
void *rosetta_memalign(size_t alignment, size_t size);

/**
 * rosetta_memzero - Zero-fill memory region
 * @s: Pointer to memory
 * @n: Number of bytes
 */
void rosetta_memzero(void *s, size_t n);

/**
 * rosetta_memory_copy - Copy memory region
 * @dest: Destination
 * @src: Source
 * @n: Number of bytes
 * Returns: Destination pointer
 */
void *rosetta_memory_copy(void *dest, const void *src, size_t n);

/**
 * rosetta_memory_move - Move memory region (overlapping safe)
 * @dest: Destination
 * @src: Source
 * @n: Number of bytes
 * Returns: Destination pointer
 */
void *rosetta_memory_move(void *dest, const void *src, size_t n);

/**
 * rosetta_memory_set - Set memory region to value
 * @s: Pointer to memory
 * @c: Value to set
 * @n: Number of bytes
 * Returns: Pointer to memory
 */
void *rosetta_memory_set(void *s, int c, size_t n);

/**
 * rosetta_memory_cmp - Compare memory regions
 * @s1: First region
 * @s2: Second region
 * @n: Number of bytes
 * Returns: 0 if equal, <0 if s1<s2, >0 if s1>s2
 */
int rosetta_memory_cmp(const void *s1, const void *s2, size_t n);

/**
 * rosetta_memory_find - Find byte in memory
 * @s: Memory region
 * @c: Byte to find
 * @n: Size of region
 * Returns: Pointer to byte, or NULL if not found
 */
void *rosetta_memory_find(const void *s, int c, size_t n);

#endif /* ROSETTA_REFACTORED_MEMORY_H */
