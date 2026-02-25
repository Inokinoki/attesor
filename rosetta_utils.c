/*
 * Rosetta String and Memory Utilities
 *
 * This module provides standard string and memory operations
 * for use within the Rosetta translator.
 */

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ============================================================================
 * String Length Operations
 * ============================================================================ */

/**
 * rosetta_strlen - Calculate string length
 *
 * @param s Null-terminated string
 * @return Length of string (excluding null terminator)
 */
size_t rosetta_strlen(const char *s)
{
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

/**
 * rosetta_strnlen - Calculate string length with limit
 *
 * @param s Null-terminated string
 * @param maxlen Maximum length to check
 * @return Length of string or maxlen if not found
 */
size_t rosetta_strnlen(const char *s, size_t maxlen)
{
    size_t len = 0;
    while (len < maxlen && *s++) {
        len++;
    }
    return len;
}

/* ============================================================================
 * String Copy Operations
 * ============================================================================ */

/**
 * rosetta_strcpy - Copy string
 *
 * @param dest Destination buffer
 * @param src Source string
 * @return Pointer to destination
 */
char *rosetta_strcpy(char *dest, const char *src)
{
    char *orig_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return orig_dest;
}

/**
 * rosetta_strncpy - Copy string with length limit
 *
 * @param dest Destination buffer
 * @param src Source string
 * @param n Maximum number of characters to copy
 * @return Pointer to destination
 */
char *rosetta_strncpy(char *dest, const char *src, size_t n)
{
    char *orig_dest = dest;
    while (n > 1 && *src) {
        *dest++ = *src++;
        n--;
    }
    if (n > 0) *dest = '\0';
    return orig_dest;
}

/* ============================================================================
 * String Concatenation Operations
 * ============================================================================ */

/**
 * rosetta_strcat - Concatenate strings
 *
 * @param dest Destination buffer (must have enough space)
 * @param src Source string to append
 * @return Pointer to destination
 */
char *rosetta_strcat(char *dest, const char *src)
{
    char *orig_dest = dest;

    /* Find end of destination string */
    while (*dest != '\0') {
        dest++;
    }

    /* Copy source to end of destination */
    while (*src != '\0') {
        *dest++ = *src++;
    }
    *dest = '\0';

    return orig_dest;
}

/**
 * rosetta_strncat - Concatenate strings with length limit
 *
 * @param dest Destination buffer
 * @param src Source string to append
 * @param n Maximum number of characters to append
 * @return Pointer to destination
 */
char *rosetta_strncat(char *dest, const char *src, size_t n)
{
    char *orig_dest = dest;

    /* Find end of destination string */
    while (*dest != '\0') {
        dest++;
    }

    /* Append up to n characters from source */
    while (n > 0 && *src != '\0') {
        *dest++ = *src++;
        n--;
    }
    *dest = '\0';

    return orig_dest;
}

/* ============================================================================
 * String Compare Operations
 * ============================================================================ */

/**
 * rosetta_strcmp - Compare two strings
 *
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * rosetta_strncmp - Compare strings with length limit
 *
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0) return 0;

    while (n > 1 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * rosetta_strcasecmp - Compare strings case-insensitively
 *
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_strcasecmp(const char *s1, const char *s2)
{
    unsigned char c1, c2;

    while (1) {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;

        /* Convert to lowercase for comparison */
        if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';

        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') return 0;
    }
}

/* ============================================================================
 * String Search Operations
 * ============================================================================ */

/**
 * rosetta_strchr - Find character in string
 *
 * @param s String to search
 * @param c Character to find
 * @return Pointer to first occurrence, or NULL if not found
 */
char *rosetta_strchr(const char *s, int c)
{
    while (*s && *s != (char)c) {
        s++;
    }
    return (*s == (char)c) ? (char *)s : NULL;
}

/**
 * rosetta_strrchr - Find last occurrence of character
 *
 * @param s String to search
 * @param c Character to find
 * @return Pointer to last occurrence, or NULL if not found
 */
char *rosetta_strrchr(const char *s, int c)
{
    const char *last = NULL;

    while (*s) {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }

    /* Handle null terminator case */
    if (c == '\0') {
        return (char *)s;
    }

    return (char *)last;
}

/**
 * rosetta_strstr - Find substring in string
 *
 * @param haystack String to search in
 * @param needle Substring to find
 * @return Pointer to first occurrence, or NULL if not found
 */
char *rosetta_strstr(const char *haystack, const char *needle)
{
    size_t needle_len;
    size_t i;

    if (!*needle) return (char *)haystack;

    needle_len = rosetta_strlen(needle);

    while (*haystack) {
        for (i = 0; i < needle_len && haystack[i] == needle[i]; i++);
        if (i == needle_len) {
            return (char *)haystack;
        }
        haystack++;
    }

    return NULL;
}

/**
 * rosetta_strspn - Get length of prefix substring
 *
 * @param s String to scan
 * @param accept Characters to accept
 * @return Length of initial segment consisting only of characters in accept
 */
size_t rosetta_strspn(const char *s, const char *accept)
{
    size_t count = 0;

    while (*s) {
        const char *p = accept;
        while (*p && *p != *s) p++;
        if (!*p) break;
        s++;
        count++;
    }

    return count;
}

/**
 * rosetta_strcspn - Get length of non-prefix substring
 *
 * @param s String to scan
 * @param reject Characters to reject
 * @return Length of initial segment consisting only of characters not in reject
 */
size_t rosetta_strcspn(const char *s, const char *reject)
{
    size_t count = 0;

    while (*s) {
        const char *p = reject;
        while (*p && *p != *s) p++;
        if (*p) break;
        s++;
        count++;
    }

    return count;
}

/* ============================================================================
 * Memory Operations
 * ============================================================================ */

/**
 * rosetta_memcpy - Copy memory region
 *
 * Copies n bytes from source to destination.
 * Does not handle overlapping regions (use memmove for that).
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    /* Copy 8 bytes at a time when possible */
    while (n >= 8) {
        *(uint64_t *)d = *(const uint64_t *)s;
        d += 8;
        s += 8;
        n -= 8;
    }

    /* Copy remaining bytes */
    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

/**
 * rosetta_memmove - Copy memory region with overlap handling
 *
 * Copies n bytes from source to destination, handling overlapping regions.
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Pointer to destination
 */
void *rosetta_memmove(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (d == s) return dest;

    /* If dest is after src and they overlap, copy backwards */
    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    } else {
        /* Otherwise copy forwards */
        while (n >= 8) {
            *(uint64_t *)d = *(const uint64_t *)s;
            d += 8;
            s += 8;
            n -= 8;
        }
        while (n--) {
            *d++ = *s++;
        }
    }

    return dest;
}

/**
 * rosetta_memcmp - Compare memory regions
 *
 * Compares n bytes of two memory regions.
 *
 * @param s1 First memory region
 * @param s2 Second memory region
 * @param n Number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int rosetta_memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

/**
 * rosetta_memset - Set memory region
 *
 * Sets n bytes of memory to value c.
 *
 * @param s Memory region
 * @param c Value to set
 * @param n Number of bytes
 * @return Pointer to memory region
 */
void *rosetta_memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;
    uint64_t pattern;

    /* Build 8-byte pattern for faster filling */
    pattern = (uint8_t)c;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    /* Set 8 bytes at a time */
    while (n >= 8) {
        *(uint64_t *)p = pattern;
        p += 8;
        n -= 8;
    }

    /* Set remaining bytes */
    while (n--) {
        *p++ = (uint8_t)c;
    }

    return s;
}

/**
 * rosetta_memchr - Find byte in memory
 *
 * Searches for first occurrence of byte c in memory region.
 *
 * @param s Memory region to search
 * @param c Byte value to find
 * @param n Number of bytes to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr(const void *s, int c, size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    uint64_t pattern;

    /* Build 8-byte pattern for comparison */
    pattern = (uint8_t)c;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    while (n >= 8) {
        uint64_t chunk = *(const uint64_t *)p;

        /* XOR with pattern - if any byte matches, result has zero byte */
        uint64_t xored = chunk ^ pattern;

        /* Check if any byte is zero */
        if (((xored - 0x0101010101010101ULL) & ~xored & 0x8080808080808080ULL) != 0) {
            /* Match found, find exact position */
            for (int i = 0; i < 8; i++) {
                if (p[i] == (uint8_t)c) {
                    return (void *)(p + i);
                }
            }
        }
        p += 8;
        n -= 8;
    }

    /* Handle remaining bytes */
    while (n--) {
        if (*p == (uint8_t)c) {
            return (void *)p;
        }
        p++;
    }

    return NULL;
}
