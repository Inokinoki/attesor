/* ============================================================================
 * Rosetta Refactored - SIMD String/Memory Operations Header
 * ============================================================================
 *
 * This header declares SIMD-optimized string and memory operations
 * for the Rosetta translation layer.
 *
 * Functions include:
 * - memchr_simd: SIMD-optimized memory search
 * - strcmp_simd: SIMD-optimized string compare
 * - memcmp_simd: SIMD-optimized memory compare
 * - memset_simd: SIMD-optimized memory set
 * - memcpy_simd: SIMD-optimized memory copy
 * ============================================================================ */

#ifndef ROSETTA_SIMD_STRING_H
#define ROSETTA_SIMD_STRING_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * SIMD Memory Search
 * ============================================================================ */

/**
 * memchr_simd - SIMD-optimized memory search
 * @ptr: Pointer to memory block
 * @len: Length to search (or -1 for unlimited)
 * Returns: Pointer to first zero byte, or NULL if not found
 */
void *memchr_simd(const void *ptr, long len);

/**
 * memchr_simd_unaligned - SIMD-optimized search for unaligned addresses
 * @ptr: Pointer to memory block
 * Returns: Pointer to first zero byte
 */
void *memchr_simd_unaligned(const void *ptr);

/* ============================================================================
 * SIMD String Compare
 * ============================================================================ */

/**
 * strcmp_simd - SIMD-optimized string compare
 * @s1: First string
 * @s2: Second string
 * Returns: Difference between first differing characters
 */
int strcmp_simd(const char *s1, const char *s2);

/**
 * strncmp_simd - SIMD-optimized bounded string compare
 * @s1: First string
 * @s2: Second string
 * @n: Maximum number of characters to compare
 * Returns: Difference between first differing characters, or 0 if equal
 */
int strncmp_simd(const char *s1, const char *s2, size_t n);

/* ============================================================================
 * SIMD Memory Operations
 * ============================================================================ */

/**
 * memcmp_simd - SIMD-optimized memory compare
 * @s1: First memory block
 * @s2: Second memory block
 * @n: Number of bytes to compare
 * Returns: Difference between first differing bytes, or 0 if equal
 */
int memcmp_simd(const void *s1, const void *s2, size_t n);

/**
 * memset_simd - SIMD-optimized memory set
 * @s: Pointer to memory block
 * @c: Value to set
 * @n: Number of bytes to set
 * Returns: Pointer to memory block
 */
void *memset_simd(void *s, int c, size_t n);

/**
 * memcpy_simd - SIMD-optimized memory copy
 * @dest: Destination buffer
 * @src: Source buffer
 * @n: Number of bytes to copy
 * Returns: Pointer to destination buffer
 */
void *memcpy_simd(void *dest, const void *src, size_t n);

/* ============================================================================
 * NEON Vector Helpers (internal use)
 * ============================================================================ */

/**
 * neon_uminv - NEON UMINV - unsigned minimum across vector
 * @vec: 16-byte vector
 * Returns: Minimum byte value in vector
 */
u8 neon_uminv(const u8 vec[16]);

/**
 * neon_umaxv - NEON UMAXV - unsigned maximum across vector
 * @vec: 16-byte vector
 * Returns: Maximum byte value in vector
 */
u8 neon_umaxv(const u8 vec[16]);

/**
 * has_zero_byte - Check if 64-bit word contains a zero byte
 * @x: 64-bit value
 * Returns: Non-zero if any byte is zero
 */
u64 has_zero_byte(u64 x);

#endif /* ROSETTA_SIMD_STRING_H */
