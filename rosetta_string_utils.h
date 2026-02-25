/* ============================================================================
 * Rosetta Translator - String Utilities Header
 * ============================================================================
 *
 * This module provides string operations for the Rosetta binary
 * translation layer, including SIMD-optimized and standard functions.
 * ============================================================================ */

#ifndef ROSETTA_STRING_UTILS_H
#define ROSETTA_STRING_UTILS_H

#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * String Utilities (Session 14)
 * ============================================================================ */

/**
 * Calculate string length
 * @param s Input string
 * @return Length of string (not including null terminator)
 */
size_t rosetta_strlen(const char *s);

/**
 * Copy string
 * @param dest Destination buffer
 * @param src Source string
 * @return Pointer to destination
 */
char *rosetta_strcpy(char *dest, const char *src);

/**
 * Concatenate strings
 * @param dest Destination string (must have sufficient space)
 * @param src Source string to append
 * @return Pointer to destination
 */
char *rosetta_strcat(char *dest, const char *src);

/**
 * Compare two strings
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strcmp(const char *s1, const char *s2);

/**
 * Compare strings with length limit
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strncmp(const char *s1, const char *s2, size_t n);

/* ============================================================================
 * String Utilities Extended (Session 24)
 * ============================================================================ */

/**
 * Copy string with length limit
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum bytes to copy
 * @return dest
 */
char *rosetta_strncpy(char *dest, const char *src, size_t n);

/**
 * Concatenate strings with length limit
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum bytes to append
 * @return dest
 */
char *rosetta_strncat(char *dest, const char *src, size_t n);

/**
 * Case-insensitive string compare
 * @param s1 First string
 * @param s2 Second string
 * @return < 0 if s1 < s2, 0 if s1 == s2, > 0 if s1 > s2
 */
int rosetta_strcasecmp(const char *s1, const char *s2);

/**
 * Find character in string
 * @param s String to search
 * @param c Character to find
 * @return Pointer to first occurrence, NULL if not found
 */
char *rosetta_strchr(const char *s, int c);

/**
 * Find last occurrence of character
 * @param s String to search
 * @param c Character to find
 * @return Pointer to last occurrence, NULL if not found
 */
char *rosetta_strrchr(const char *s, int c);

/**
 * Get length of prefix substring
 * @param s String to search
 * @param accept Characters to accept
 * @return Length of initial segment consisting of accept characters
 */
size_t rosetta_strspn(const char *s, const char *accept);

/**
 * Get length of complementary prefix substring
 * @param s String to search
 * @param reject Characters to reject
 * @return Length of initial segment consisting of non-reject characters
 */
size_t rosetta_strcspn(const char *s, const char *reject);

/* ============================================================================
 * Additional String Utilities (Session 34)
 * ============================================================================ */

/**
 * Duplicate a string
 * @param s Source string
 * @return Pointer to newly allocated duplicate string (must be freed)
 */
char *rosetta_strdup(const char *s);

/**
 * Find substring in string
 * @param haystack String to search in
 * @param needle String to search for
 * @return Pointer to first occurrence of needle, or NULL if not found
 */
char *rosetta_strstr(const char *haystack, const char *needle);

/**
 * Find first match of any character from set
 * @param s String to search
 * @param charset Set of characters to match
 * @return Pointer to first matching character, or NULL if none found
 */
char *rosetta_strpbrk(const char *s, const char *charset);

/**
 * Tokenize a string
 * @param str String to tokenize (NULL on subsequent calls)
 * @param delim Delimiter characters
 * @return Pointer to next token, or NULL if no more tokens
 */
char *rosetta_strtok(char *str, const char *delim);

/**
 * Find memory region in memory region
 * @param haystack Pointer to memory to search in
 * @param haystack_len Length of haystack
 * @param needle Pointer to memory to search for
 * @param needle_len Length of needle
 * @return Pointer to first occurrence, or NULL if not found
 */
void *rosetta_memmem(const void *haystack, size_t haystack_len,
                     const void *needle, size_t needle_len);

/**
 * Find last occurrence of byte in memory
 * @param s Pointer to memory to search
 * @param c Byte to search for
 * @param n Length of memory region
 * @return Pointer to last occurrence, or NULL if not found
 */
void *rosetta_memrchr(const void *s, int c, size_t n);

/**
 * Calculate string length with limit
 * @param s String to measure
 * @param maxlen Maximum length to check
 * @return Length of string or maxlen if no null terminator found
 */
size_t rosetta_strnlen(const char *s, size_t maxlen);

/**
 * Copy string with size limit
 * @param dest Destination buffer
 * @param src Source string
 * @param destsize Size of destination buffer
 * @return Length of src
 */
size_t rosetta_strlcpy(char *dest, const char *src, size_t destsize);

/**
 * Concatenate strings with size limit
 * @param dest Destination buffer (must be null-terminated)
 * @param src Source string to append
 * @param destsize Total size of destination buffer
 * @return Total length of string we tried to create
 */
size_t rosetta_strlcat(char *dest, const char *src, size_t destsize);

#endif /* ROSETTA_STRING_UTILS_H */
