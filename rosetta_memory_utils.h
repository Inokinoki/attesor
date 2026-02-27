/* ============================================================================
 * Rosetta Translator - Memory Utilities Header
 * ============================================================================
 *
 * This module provides memory operations for the Rosetta binary
 * translation layer, including SIMD-optimized and standard functions.
 * ============================================================================ */

#ifndef ROSETTA_MEMORY_UTILS_H
#define ROSETTA_MEMORY_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>

/* ============================================================================
 * SIMD-Optimized Memory Operations (Session 14)
 * ============================================================================ */

/**
 * SIMD-optimized memcpy for aligned data
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n);

/**
 * SIMD-optimized memset
 * @param s Memory region
 * @param c Value to set
 * @param n Number of bytes
 * @return Pointer to memory region
 */
void *rosetta_memset_simd(void *s, int c, size_t n);

/**
 * SIMD-optimized memory search for null bytes
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (-1 for unlimited)
 * @return Pointer to found null byte, or NULL if not found
 */
void *rosetta_memchr_simd_nul(const void *ptr, long len);

/**
 * SIMD-optimized memory search for unaligned data (null bytes)
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (-1 for unlimited)
 * @return Pointer to found null byte, or NULL if not found
 */
void *rosetta_memchr_simd_unaligned_nul(const void *ptr, long len);

/**
 * SIMD-optimized memory compare
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);

/* ============================================================================
 * Standard Memory Operations (Session 14)
 * ============================================================================ */

/**
 * Copy memory region
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memcpy(void *dest, const void *src, size_t n);

/**
 * Copy memory region with overlap handling
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memmove(void *dest, const void *src, size_t n);

/**
 * Compare memory regions
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_memcmp(const void *s1, const void *s2, size_t n);

/**
 * Set memory region
 * @param s Memory region
 * @param c Value to set
 * @param n Number of bytes
 * @return Pointer to memory region
 */
void *rosetta_memset(void *s, int c, size_t n);

/**
 * Find byte in memory
 * @param s Memory region to search
 * @param c Byte value to find
 * @param n Number of bytes to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr(const void *s, int c, size_t n);

/* ============================================================================
 * Memory Utilities Extended (Session 24)
 * ============================================================================ */

/**
 * Zero-fill memory region
 * @param s Pointer to memory
 * @param n Number of bytes to zero
 */
void rosetta_memzero(void *s, size_t n);

/**
 * Copy memory until character found
 * @param dest Destination buffer
 * @param src Source buffer
 * @param c Character to stop at
 * @param n Maximum bytes to copy
 * @return Pointer after copied character, NULL if not found
 */
void *rosetta_memccpy(void *dest, const void *src, int c, size_t n);

/**
 * Find first byte NOT equal to c
 * @param s Memory region to search
 * @param c Byte value to match against
 * @param n Size of region
 * @return Pointer to first non-matching byte, NULL if all match
 */
void *rosetta_memchr_inv(const void *s, int c, size_t n);

/**
 * Constant-time memory compare (security-sensitive)
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Size to compare
 * @return 0 if equal, non-zero otherwise
 */
int rosetta_memcmp_consttime(const void *s1, const void *s2, size_t n);

/**
 * Find first byte equal to any in mask
 * @param s Pointer to memory to search
 * @param mask 16-byte mask of bytes to match
 * @param n Length of memory region
 * @return Pointer to first matching byte, or NULL if none found
 */
void *rosetta_memchr_eq(const void *s, const uint8_t mask[16], size_t n);

/**
 * Optimized memcpy for non-overlapping regions
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to copy
 * @return Destination pointer
 */
void *rosetta_memcpy_nonoverlapping(void *dest, const void *src, size_t n);

/**
 * Safe memory move with overlap detection
 * @param dest Destination pointer
 * @param src Source pointer
 * @param n Number of bytes to move
 * @return Destination pointer
 */
void *rosetta_memmove_safe(void *dest, const void *src, size_t n);

/**
 * Swap contents of two memory regions
 * @param a First memory region
 * @param b Second memory region
 * @param n Number of bytes to swap
 */
void rosetta_memswap(void *a, void *b, size_t n);

/**
 * Fill memory with word pattern
 * @param dest Destination pointer
 * @param word Word pattern to fill
 * @param n Number of bytes to fill
 * @return Destination pointer
 */
void *rosetta_memfill_word(void *dest, uint64_t word, size_t n);

/* ============================================================================
 * Memory Management Helpers (Session 24)
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
 * @param addr Address of region
 * @param size Size of region
 * @return 0 on success, -1 on error
 */
int rosetta_munmap_region(void *addr, size_t size);

/**
 * Change protection on memory region
 * @param addr Address of region
 * @param size Size of region
 * @param prot New protection flags
 * @return 0 on success, -1 on error
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot);

/**
 * Aligned memory allocation
 * @param alignment Alignment requirement
 * @param size Size to allocate
 * @return Pointer to allocated memory, or NULL on error
 */
void *rosetta_memalign(size_t alignment, size_t size);

#endif /* ROSETTA_MEMORY_UTILS_H */
