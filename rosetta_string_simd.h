/* ============================================================================
 * Rosetta Translator - SIMD String Utilities Header
 * ============================================================================
 *
 * This module provides SIMD-optimized string and memory operations
 * for high-performance string processing.
 * ============================================================================ */

#ifndef ROSETTA_STRING_SIMD_H
#define ROSETTA_STRING_SIMD_H

#include <stdint.h>
#include <stddef.h>
#include "rosetta_refactored_vector.h"

/* ============================================================================
 * String Length and Comparison
 * ============================================================================ */

/**
 * rosetta_strlen_simd - Calculate string length using SIMD
 * @param s Input null-terminated string
 * @return Length of string (not including null terminator)
 *
 * Processes 16 bytes at a time using SIMD parallel comparison
 * to find the null terminator efficiently.
 */
size_t rosetta_strlen_simd(const char *s);

/**
 * rosetta_strcmp_simd - Compare two strings using SIMD
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 *
 * Uses SIMD parallel byte comparison for faster string comparison.
 */
int rosetta_strcmp_simd(const char *s1, const char *s2);

/**
 * rosetta_strncmp_simd - Compare up to n bytes of two strings
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n);

/**
 * rosetta_strcpy_simd - Copy string using SIMD
 * @param dest Destination buffer
 * @param src Source string
 * @return Pointer to destination buffer
 */
char *rosetta_strcpy_simd(char *dest, const char *src);

/**
 * rosetta_strncpy_simd - Copy up to n bytes of string
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum bytes to copy
 * @return Pointer to destination buffer
 */
char *rosetta_strncpy_simd(char *dest, const char *src, size_t n);

/* ============================================================================
 * Memory Search
 * ============================================================================ */

/**
 * rosetta_memchr_simd_search - Find byte in memory using SIMD
 * @param ptr Pointer to memory
 * @param c Byte value to search for
 * @param n Number of bytes to search
 * @return Pointer to first occurrence, or NULL if not found
 *
 * Uses SIMD parallel comparison to search for a byte value
 * much faster than byte-by-byte comparison.
 */
void *rosetta_memchr_simd_search(const void *ptr, int c, size_t n);

/**
 * rosetta_memchr_simd_variant - Variant with long length parameter
 * @param ptr Pointer to memory
 * @param c Byte value to search for
 * @param len Number of bytes to search (negative = unlimited)
 * @return Pointer to first occurrence, or NULL if not found
 */
void *rosetta_memchr_simd_variant(const void *ptr, int c, long len);

/* ============================================================================
 * Memory Comparison
 * ============================================================================ */

/**
 * rosetta_memcmp_simd - Compare memory regions using SIMD
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);

/* ============================================================================
 * Memory Copy
 * ============================================================================ */

/**
 * rosetta_memcpy_simd - Copy memory using SIMD
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination buffer
 *
 * Uses SIMD load/store for high-bandwidth memory copying.
 * Optimized for aligned, large copies.
 */
void *rosetta_memcpy_simd(void *dest, const void *src, size_t n);

/**
 * rosetta_memmove_simd - Move memory with overlap handling
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to move
 * @return Pointer to destination buffer
 *
 * Handles overlapping source and destination correctly.
 */
void *rosetta_memmove_simd(void *dest, const void *src, size_t n);

/**
 * rosetta_memset_simd - Set memory to a byte value
 * @param dest Destination buffer
 * @param c Byte value to set
 * @param n Number of bytes to set
 * @return Pointer to destination buffer
 *
 * Uses SIMD broadcast and store for fast initialization.
 */
void *rosetta_memset_simd(void *dest, int c, size_t n);

/* ============================================================================
 * String Search
 * ============================================================================ */

/**
 * rosetta_strstr_simd - Find substring in string
 * @param haystack String to search in
 * @param needle Substring to find
 * @return Pointer to first occurrence, or NULL if not found
 */
char *rosetta_strstr_simd(const char *haystack, const char *needle);

/**
 * rosetta_strchr_simd - Find character in string
 * @param s String to search
 * @param c Character to find
 * @return Pointer to first occurrence, or NULL if not found
 */
char *rosetta_strchr_simd(const char *s, int c);

/**
 * rosetta_strrchr_simd - Find last occurrence of character
 * @param s String to search
 * @param c Character to find
 * @return Pointer to last occurrence, or NULL if not found
 */
char *rosetta_strrchr_simd(const char *s, int c);

/* ============================================================================
 * SIMD Helper Functions
 * ============================================================================ */

/**
 * simd_has_zero_byte - Check if any byte in vector is zero
 * @param v Input vector
 * @return Non-zero if any byte is zero
 */
int simd_has_zero_byte(Vector128 v);

/**
 * simd_has_byte - Check if any byte in vector matches value
 * @param v Input vector
 * @param c Byte value to match
 * @return Non-zero if any byte matches
 */
int simd_has_byte(Vector128 v, uint8_t c);

/**
 * simd_byte_mask - Convert vector comparison to byte mask
 * @param v Input vector with comparison results
 * @return Byte mask (16 bits, one per byte)
 */
uint16_t simd_byte_mask(Vector128 v);

/**
 * simd_find_first_nonzero - Find index of first non-zero byte
 * @param v Input vector
 * @return Index of first non-zero byte (0-15), or 16 if all zero
 */
int simd_find_first_nonzero(Vector128 v);

/**
 * simd_broadcast_byte - Create vector with all bytes set to same value
 * @param c Byte value to broadcast
 * @return Vector with all 16 bytes set to c
 */
Vector128 simd_broadcast_byte(uint8_t c);

/* ============================================================================
 * Advanced SIMD String Operations
 * ============================================================================ */

/**
 * simd_strcmp_loop - Main loop for SIMD string comparison
 * @param p1 Pointer to first string
 * @param p2 Pointer to second string
 * @return 0 if equal up to first difference or null, non-zero otherwise
 */
int simd_strcmp_loop(const uint8_t *p1, const uint8_t *p2);

/**
 * simd_strlen_loop - Main loop for SIMD string length
 * @param p Pointer to string
 * @return Length accumulated from processed blocks
 */
size_t simd_strlen_loop(const uint8_t *p);

/**
 * simd_memset_loop - Main loop for SIMD memset
 * @param dest Destination pointer
 * @param v Vector value to store
 * @param count Number of vectors to write
 * @return Final destination pointer
 */
void *simd_memset_loop(void *dest, Vector128 v, size_t count);

/**
 * simd_memcpy_loop - Main loop for SIMD memcpy
 * @param dest Destination pointer
 * @param src Source pointer
 * @param count Number of vectors to copy
 * @return Final destination pointer
 */
void *simd_memcpy_loop(void *dest, const void *src, size_t count);

#endif /* ROSETTA_STRING_SIMD_H */
