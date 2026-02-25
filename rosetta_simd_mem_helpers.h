/* ============================================================================
 * Rosetta Translator - SIMD Memory Helpers Header
 * ============================================================================
 *
 * SIMD-optimized memory and string operations.
 * ============================================================================ */

#ifndef ROSETTA_SIMD_MEM_HELPERS_H
#define ROSETTA_SIMD_MEM_HELPERS_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * SIMD Memory Search Functions
 * ============================================================================ */

/**
 * SIMD-optimized memory search (memchr equivalent)
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (negative = unlimited)
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd(const void *ptr, long len);

/**
 * SIMD-optimized string comparison (strcmp equivalent)
 * @param s1 First string
 * @param s2 Second string
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strcmp_simd(const char *s1, const char *s2);

/**
 * SIMD-optimized string length (strlen equivalent)
 * @param s Null-terminated string
 * @return Length of string
 */
size_t rosetta_strlen_simd(const char *s);

/**
 * SIMD-optimized memory copy
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Destination pointer
 */
void *rosetta_memcpy_simd(void *dest, const void *src, size_t n);

/**
 * SIMD-optimized memory set
 * @param s Destination buffer
 * @param c Value to set
 * @param n Number of bytes to set
 * @return Destination pointer
 */
void *rosetta_memset_simd(void *s, int c, size_t n);

/**
 * SIMD-optimized memory compare
 * @param s1 First buffer
 * @param s2 Second buffer
 * @param n Number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);

#endif /* ROSETTA_SIMD_MEM_HELPERS_H */
