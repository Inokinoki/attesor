/* ============================================================================
 * Rosetta SIMD Helper Module
 * ============================================================================
 *
 * This module provides SIMD-optimized memory and string operations using
 * ARM64 NEON instructions for parallel byte processing.
 * ============================================================================ */

#ifndef ROSETTA_SIMD_H
#define ROSETTA_SIMD_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * SIMD Primitive Operations
 * ============================================================================ */

/**
 * NEON UMINV - Find minimum byte across 16-byte vector
 *
 * @param bytes Array of 16 bytes to reduce
 * @return Minimum byte value
 */
uint8_t neon_uminv(const uint8_t bytes[16]);

/* ============================================================================
 * SIMD Memory Operations
 * ============================================================================ */

/**
 * SIMD-optimized memory search (memchr equivalent)
 *
 * Searches for a null byte (or specific pattern) in memory using NEON
 * instructions for parallel comparison.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (negative = unlimited)
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd(const void *ptr, long len);

/**
 * SIMD-optimized memory search for unaligned data
 *
 * Searches for a null byte in potentially unaligned memory.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd_unaligned(const void *ptr, long len);

/**
 * SIMD-optimized memory compare
 *
 * Compares n bytes of two memory regions using NEON instructions
 * for parallel byte comparison.
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);

/* ============================================================================
 * SIMD String Operations
 * ============================================================================ */

/**
 * SIMD-optimized string comparison (strcmp equivalent)
 *
 * Compares two strings using NEON instructions for parallel byte comparison.
 *
 * @param s1 First string
 * @param s2 Second string
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strcmp_simd(const char *s1, const char *s2);

/**
 * SIMD-optimized string comparison with length limit (strncmp equivalent)
 *
 * Compares up to n bytes of two strings using NEON instructions.
 *
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n);

/* ============================================================================
 * SIMD Memory Copy/Set Operations
 * ============================================================================ */

/**
 * SIMD-optimized memcpy for aligned data
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return dest
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n);

/**
 * SIMD-optimized memset
 *
 * @param s Memory region to set
 * @param c Byte value to set
 * @param n Number of bytes to set
 * @return s
 */
void *rosetta_memset_simd(void *s, int c, size_t n);

#endif /* ROSETTA_SIMD_H */
