/*
 * Rosetta SIMD-Optimized Memory Operations
 *
 * This module provides SIMD-accelerated memory and string operations
 * using ARM64 NEON instructions for parallel processing.
 */

#ifndef ROSETTA_SIMD_MEM_H
#define ROSETTA_SIMD_MEM_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * SIMD Memory Search Operations
 * ============================================================================ */

/**
 * SIMD-optimized memchr - Search for byte in memory
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (negative = unlimited)
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd(const void *ptr, long len);

/**
 * SIMD-optimized memchr for unaligned memory
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd_unaligned(const void *ptr, long len);

/* ============================================================================
 * SIMD String Compare Operations
 * ============================================================================ */

/**
 * SIMD-optimized strcmp - Compare two strings
 * @param s1 First string
 * @param s2 Second string
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strcmp_simd(const char *s1, const char *s2);

/**
 * SIMD-optimized strncmp - Compare up to n bytes of two strings
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n);

/**
 * SIMD-optimized memcmp - Compare two memory regions
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);

/* ============================================================================
 * SIMD Memory Copy Operations
 * ============================================================================ */

/**
 * SIMD-optimized memcpy for aligned memory regions
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Destination pointer
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n);

/* ============================================================================
 * SIMD Memory Set Operations
 * ============================================================================ */

/**
 * SIMD-optimized memset - Fill memory with byte pattern
 * @param s Memory region to fill
 * @param c Byte value to fill with
 * @param n Number of bytes to fill
 * @return Pointer to filled memory
 */
void *rosetta_memset_simd(void *s, int c, size_t n);

#endif /* ROSETTA_SIMD_MEM_H */
