/* ============================================================================
 * Rosetta Translator - String Utilities Implementation
 * ============================================================================
 *
 * This module provides string operations for the Rosetta binary
 * translation layer, including SIMD-optimized and standard functions.
 * ============================================================================ */

#include "rosetta_string_utils.h"
#include "rosetta_memory_utils.h"
#include "rosetta_refactored_helpers.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * String Utilities (Session 14)
 * ============================================================================ */

/**
 * Calculate string length
 */
size_t rosetta_strlen(const char *s)
{
    const char *p = s;

    /* Process 8 bytes at a time for efficiency */
    while (1) {
        /* Check for null byte in current 8-byte chunk */
        uint64_t chunk = *(const uint64_t *)p;

        /* Check if any byte is zero (null terminator) */
        if (((chunk - 0x0101010101010101ULL) & ~chunk & 0x8080808080808080ULL) != 0) {
            /* Null byte found, check each byte */
            while (*p != '\0') {
                p++;
            }
            return (size_t)(p - s);
        }
        p += 8;
    }
}

/**
 * Copy string
 */
char *rosetta_strcpy(char *dest, const char *src)
{
    char *orig_dest = dest;

    /* Copy 8 bytes at a time until null terminator found */
    while (1) {
        uint64_t chunk = *(const uint64_t *)src;
        *(uint64_t *)dest = chunk;

        /* Check if null byte was in this chunk */
        if (((chunk - 0x0101010101010101ULL) & ~chunk & 0x8080808080808080ULL) != 0) {
            /* Find exact position of null and finish byte-by-byte */
            while (*dest != '\0') {
                dest++;
                src++;
            }
            break;
        }
        dest += 8;
        src += 8;
    }

    return orig_dest;
}

/**
 * Concatenate strings
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
 * Compare two strings
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
 * Compare strings with length limit
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

/* ============================================================================
 * String Utilities Extended (Session 24)
 * ============================================================================ */

/**
 * Copy string with length limit
 */
char *rosetta_strncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    return dest;
}

/**
 * Concatenate strings with length limit
 */
char *rosetta_strncat(char *dest, const char *src, size_t n)
{
    strncat(dest, src, n);
    return dest;
}

/**
 * Case-insensitive string compare
 */
int rosetta_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}

/**
 * Find character in string
 */
char *rosetta_strchr(const char *s, int c)
{
    return strchr(s, c);
}

/**
 * Find last occurrence of character
 */
char *rosetta_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

/**
 * Get length of prefix substring
 */
size_t rosetta_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}

/**
 * Get length of complementary prefix substring
 */
size_t rosetta_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

/* ============================================================================
 * Additional String Utilities (Session 34)
 * ============================================================================ */

/**
 * Duplicate a string
 */
char *rosetta_strdup(const char *s)
{
    size_t len;
    char *dup;

    if (s == NULL) {
        return NULL;
    }

    len = rosetta_strlen(s);
    dup = (char *)rosetta_malloc(len + 1);
    if (dup != NULL) {
        rosetta_memcpy(dup, s, len + 1);
    }

    return dup;
}

/**
 * Find substring in string
 */
char *rosetta_strstr(const char *haystack, const char *needle)
{
    size_t h_len, n_len;
    const char *h;

    if (haystack == NULL || needle == NULL) {
        return NULL;
    }

    h_len = rosetta_strlen(haystack);
    n_len = rosetta_strlen(needle);

    if (n_len == 0) {
        return (char *)haystack;
    }

    if (n_len > h_len) {
        return NULL;
    }

    for (h = haystack; h_len >= n_len; h++, h_len--) {
        if (rosetta_memcmp(h, needle, n_len) == 0) {
            return (char *)h;
        }
    }

    return NULL;
}

/**
 * Find first match of any character from set
 */
char *rosetta_strpbrk(const char *s, const char *charset)
{
    const char *c;

    if (s == NULL || charset == NULL) {
        return NULL;
    }

    while (*s != '\0') {
        for (c = charset; *c != '\0'; c++) {
            if (*s == *c) {
                return (char *)s;
            }
        }
        s++;
    }

    return NULL;
}

/**
 * Tokenize a string
 */
char *rosetta_strtok(char *str, const char *delim)
{
    static char *last;
    char *token_start;

    if (str != NULL) {
        last = str;
    } else if (last == NULL) {
        return NULL;
    }

    /* Skip leading delimiters */
    while (*last != '\0') {
        const char *d;
        int is_delim = 0;

        for (d = delim; *d != '\0'; d++) {
            if (*last == *d) {
                is_delim = 1;
                break;
            }
        }

        if (!is_delim) {
            break;
        }
        last++;
    }

    if (*last == '\0') {
        return NULL;
    }

    token_start = last;

    /* Find end of token */
    while (*last != '\0') {
        const char *d;
        int is_delim = 0;

        for (d = delim; *d != '\0'; d++) {
            if (*last == *d) {
                is_delim = 1;
                break;
            }
        }

        if (is_delim) {
            *last = '\0';
            last++;
            return token_start;
        }
        last++;
    }

    return token_start;
}

/**
 * Find memory region in memory region
 */
void *rosetta_memmem(const void *haystack, size_t haystack_len,
                     const void *needle, size_t needle_len)
{
    const uint8_t *h = (const uint8_t *)haystack;

    if (needle_len == 0) {
        return (void *)haystack;
    }

    if (needle_len > haystack_len) {
        return NULL;
    }

    while (haystack_len >= needle_len) {
        if (rosetta_memcmp(h, needle, needle_len) == 0) {
            return (void *)h;
        }
        h++;
        haystack_len--;
    }

    return NULL;
}

/**
 * Find last occurrence of byte in memory
 */
void *rosetta_memrchr(const void *s, int c, size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    const uint8_t *end = p + n;
    unsigned char uc = (unsigned char)c;

    while (end > p) {
        if (*--end == uc) {
            return (void *)end;
        }
    }

    return NULL;
}

/**
 * Calculate string length with limit
 */
size_t rosetta_strnlen(const char *s, size_t maxlen)
{
    size_t i;

    for (i = 0; i < maxlen && s[i] != '\0'; i++);

    return i;
}

/**
 * Copy string with size limit
 */
size_t rosetta_strlcpy(char *dest, const char *src, size_t destsize)
{
    size_t src_len = rosetta_strlen(src);
    size_t copy_len;

    if (destsize > 0) {
        copy_len = (src_len < destsize - 1) ? src_len : destsize - 1;
        rosetta_memcpy(dest, src, copy_len);
        dest[copy_len] = '\0';
    }

    return src_len;
}

/**
 * Concatenate strings with size limit
 */
size_t rosetta_strlcat(char *dest, const char *src, size_t destsize)
{
    size_t dest_len = rosetta_strlen(dest);
    size_t src_len = rosetta_strlen(src);
    size_t copy_len;

    if (destsize > dest_len) {
        copy_len = (src_len < destsize - dest_len - 1) ? src_len : destsize - dest_len - 1;
        rosetta_memcpy(dest + dest_len, src, copy_len);
        dest[dest_len + copy_len] = '\0';
    }

    return dest_len + src_len;
}
