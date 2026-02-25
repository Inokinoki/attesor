/* ============================================================================
 * Rosetta Translator - SIMD String Utilities Implementation
 * ============================================================================
 *
 * This module implements SIMD-optimized string and memory operations
 * for high-performance string processing.
 * ============================================================================ */

#include "rosetta_string_simd.h"
#include "rosetta_refactored_vector.h"
#include <string.h>

/* ============================================================================
 * SIMD Helper Functions
 * ============================================================================ */

/**
 * simd_has_zero_byte - Check if any byte in vector is zero
 *
 * Uses the classic SWAR (SIMD Within A Register) technique:
 * (x - 0x01010101...) & ~x & 0x80808080... is non-zero iff any byte of x is zero.
 */
int simd_has_zero_byte(Vector128 v)
{
    uint64_t lo = v.lo;
    uint64_t hi = v.hi;

    uint64_t lo_mask = ((lo - 0x0101010101010101ULL) & ~lo & 0x8080808080808080ULL);
    uint64_t hi_mask = ((hi - 0x0101010101010101ULL) & ~hi & 0x8080808080808080ULL);

    return (lo_mask | hi_mask) != 0;
}

/**
 * simd_has_byte - Check if any byte in vector matches value
 */
int simd_has_byte(Vector128 v, uint8_t c)
{
    /* Broadcast c to all bytes */
    Vector128 broadcast = simd_broadcast_byte(c);

    /* XOR: matching bytes become zero */
    Vector128 diff = v128_xor(v, broadcast);

    /* Check if any byte is zero (meaning match found) */
    return simd_has_zero_byte(diff);
}

/**
 * simd_byte_mask - Convert vector comparison to byte mask
 */
uint16_t simd_byte_mask(Vector128 v)
{
    /* Extract most significant bit of each byte */
    uint16_t mask = 0;

    for (int i = 0; i < 16; i++) {
        uint8_t byte = v128_extract_byte(v, i);
        if (byte & 0x80) {
            mask |= (1 << i);
        }
    }

    return mask;
}

/**
 * simd_find_first_nonzero - Find index of first non-zero byte
 */
int simd_find_first_nonzero(Vector128 v)
{
    uint64_t lo = v.lo;
    uint64_t hi = v.hi;

    if (lo != 0) {
        /* Find first non-zero byte in low 64 bits */
        for (int i = 0; i < 8; i++) {
            if ((lo >> (i * 8)) & 0xFF) {
                return i;
            }
        }
    }

    if (hi != 0) {
        /* Find first non-zero byte in high 64 bits */
        for (int i = 0; i < 8; i++) {
            if ((hi >> (i * 8)) & 0xFF) {
                return i + 8;
            }
        }
    }

    return 16;  /* All bytes are zero */
}

/**
 * simd_broadcast_byte - Create vector with all bytes set to same value
 */
Vector128 simd_broadcast_byte(uint8_t c)
{
    return v128_dup(c);
}

/* ============================================================================
 * String Length
 * ============================================================================ */

/**
 * rosetta_strlen_simd - Calculate string length using SIMD
 *
 * Processes 16 bytes at a time to find the null terminator.
 * Much faster than byte-by-byte for long strings.
 */
size_t rosetta_strlen_simd(const char *s)
{
    const uint8_t *p = (const uint8_t *)s;

    /* Align to 16-byte boundary first */
    while ((uintptr_t)p & 0x0F) {
        if (*p == '\0') {
            return p - (const uint8_t *)s;
        }
        p++;
    }

    /* Process 16 bytes at a time */
    while (1) {
        Vector128 v = v128_load(p);

        if (simd_has_zero_byte(v)) {
            /* Null byte found in this block - find exact position */
            for (int i = 0; i < 16; i++) {
                if (v128_extract_byte(v, i) == 0) {
                    return (p - (const uint8_t *)s) + i;
                }
            }
        }

        p += 16;
    }
}

/**
 * simd_strlen_loop - Main loop for SIMD string length
 */
size_t simd_strlen_loop(const uint8_t *p)
{
    size_t len = 0;

    while (1) {
        Vector128 v = v128_load(p + len);

        if (simd_has_zero_byte(v)) {
            break;
        }

        len += 16;
    }

    return len;
}

/* ============================================================================
 * String Comparison
 * ============================================================================ */

/**
 * rosetta_strcmp_simd - Compare two strings using SIMD
 */
int rosetta_strcmp_simd(const char *s1, const char *s2)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    /* Handle aligned case - process 16 bytes at a time */
    while (1) {
        Vector128 v1 = v128_load(p1);
        Vector128 v2 = v128_load(p2);

        /* Check for differences */
        Vector128 diff = v128_xor(v1, v2);

        if (diff.lo != 0 || diff.hi != 0) {
            /* Found difference - find first differing byte */
            for (int i = 0; i < 16; i++) {
                uint8_t b1 = v128_extract_byte(v1, i);
                uint8_t b2 = v128_extract_byte(v2, i);
                if (b1 != b2) {
                    return (int)b1 - (int)b2;
                }
                if (b1 == '\0') {
                    return 0;  /* Both strings end here */
                }
            }
        }

        /* Check for null terminator (strings are equal so far) */
        if (simd_has_zero_byte(v1)) {
            return 0;  /* Strings are equal */
        }

        p1 += 16;
        p2 += 16;
    }
}

/**
 * simd_strcmp_loop - Main loop for SIMD string comparison
 */
int simd_strcmp_loop(const uint8_t *p1, const uint8_t *p2)
{
    while (1) {
        Vector128 v1 = v128_load(p1);
        Vector128 v2 = v128_load(p2);

        Vector128 diff = v128_xor(v1, v2);

        if (diff.lo != 0 || diff.hi != 0) {
            return -1;  /* Different */
        }

        if (simd_has_zero_byte(v1)) {
            return 0;  /* Equal and terminated */
        }

        p1 += 16;
        p2 += 16;
    }
}

/**
 * rosetta_strncmp_simd - Compare up to n bytes of two strings
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n)
{
    if (n == 0) {
        return 0;
    }

    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    const uint8_t *end1 = p1 + n;

    while (p1 < end1) {
        uint8_t b1 = *p1++;
        uint8_t b2 = *p2++;

        if (b1 != b2) {
            return (int)b1 - (int)b2;
        }

        if (b1 == '\0') {
            return 0;
        }
    }

    return 0;
}

/* ============================================================================
 * Memory Search
 * ============================================================================ */

/**
 * rosetta_memchr_simd - Find byte in memory using SIMD
 */
void *rosetta_memchr_simd(const void *ptr, int c, size_t n)
{
    const uint8_t *p = (const uint8_t *)ptr;
    const uint8_t *end = p + n;

    /* Broadcast search byte to all positions */
    Vector128 target = simd_broadcast_byte((uint8_t)c);

    /* Align to 16-byte boundary */
    while ((uintptr_t)p & 0x0F && p < end) {
        if (*p == (uint8_t)c) {
            return (void *)p;
        }
        p++;
    }

    /* Process 16 bytes at a time */
    while (p + 16 <= end) {
        Vector128 v = v128_load(p);

        /* Compare: result has 0xFF where bytes match */
        Vector128 cmp = v128_eq(v, target);

        if (cmp.lo != 0 || cmp.hi != 0) {
            /* Match found - find exact position */
            for (int i = 0; i < 16; i++) {
                if (v128_extract_byte(cmp, i) != 0) {
                    return (void *)(p + i);
                }
            }
        }

        p += 16;
    }

    /* Handle remaining bytes */
    while (p < end) {
        if (*p == (uint8_t)c) {
            return (void *)p;
        }
        p++;
    }

    return NULL;  /* Not found */
}

/**
 * rosetta_memchr_simd_variant - Variant with long length parameter
 */
void *rosetta_memchr_simd_variant(const void *ptr, int c, long len)
{
    if (len < 0) {
        /* Unlimited length - search until found or alignment boundary */
        return rosetta_memchr_simd(ptr, c, 4096);  /* Reasonable limit */
    }

    return rosetta_memchr_simd(ptr, c, (size_t)len);
}

/* ============================================================================
 * Memory Copy
 * ============================================================================ */

/**
 * rosetta_memcpy_simd - Copy memory using SIMD
 */
void *rosetta_memcpy_simd(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    /* Handle small copies with regular memcpy */
    if (n < 64) {
        memcpy(d, s, n);
        return dest;
    }

    /* Align to 16-byte boundary */
    while ((uintptr_t)s & 0x0F && n > 0) {
        *d++ = *s++;
        n--;
    }

    /* Copy 16 bytes at a time using SIMD */
    while (n >= 16) {
        Vector128 v = v128_load(s);
        v128_store(v, d);
        s += 16;
        d += 16;
        n -= 16;
    }

    /* Handle remaining bytes */
    if (n > 0) {
        memcpy(d, s, n);
    }

    return dest;
}

/**
 * simd_memcpy_loop - Main loop for SIMD memcpy
 */
void *simd_memcpy_loop(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (size_t i = 0; i < count; i++) {
        Vector128 v = v128_load(s + i * 16);
        v128_store(v, d + i * 16);
    }

    return d + count * 16;
}

/**
 * rosetta_memmove_simd - Move memory with overlap handling
 */
void *rosetta_memmove_simd(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if (s == d) {
        return dest;
    }

    /* Check for overlap: if dest is after src and overlaps, copy backwards */
    if (d > s && d < s + n) {
        /* Copy backwards */
        s += n;
        d += n;
        while (n-- > 0) {
            *--d = *--s;
        }
    } else {
        /* No overlap or dest before src - copy forwards */
        memcpy(d, s, n);
    }

    return dest;
}

/* ============================================================================
 * Memory Set
 * ============================================================================ */

/**
 * rosetta_memset_simd - Set memory to a byte value
 */
void *rosetta_memset_simd(void *dest, int c, size_t n)
{
    uint8_t *d = (uint8_t *)dest;

    /* Create broadcast vector */
    Vector128 v = simd_broadcast_byte((uint8_t)c);

    /* Align to 16-byte boundary */
    while ((uintptr_t)d & 0x0F && n > 0) {
        *d++ = (uint8_t)c;
        n--;
    }

    /* Set 16 bytes at a time */
    while (n >= 16) {
        v128_store(v, d);
        d += 16;
        n -= 16;
    }

    /* Handle remaining bytes */
    if (n > 0) {
        memset(d, c, n);
    }

    return dest;
}

/**
 * simd_memset_loop - Main loop for SIMD memset
 */
void *simd_memset_loop(void *dest, Vector128 v, size_t count)
{
    uint8_t *d = (uint8_t *)dest;

    for (size_t i = 0; i < count; i++) {
        v128_store(v, d + i * 16);
    }

    return d + count * 16;
}

/* ============================================================================
 * String Search
 * ============================================================================ */

/**
 * rosetta_strchr_simd - Find character in string
 */
char *rosetta_strchr_simd(const char *s, int c)
{
    const uint8_t *p = (const uint8_t *)s;

    /* Broadcast search character */
    Vector128 target = simd_broadcast_byte((uint8_t)c);

    /* Align to 16-byte boundary */
    while ((uintptr_t)p & 0x0F) {
        if (*p == (uint8_t)c) {
            return (char *)p;
        }
        if (*p == '\0') {
            return NULL;
        }
        p++;
    }

    /* Process 16 bytes at a time */
    while (1) {
        Vector128 v = v128_load(p);

        /* Check for null terminator */
        if (simd_has_zero_byte(v)) {
            /* String ends in this block - search remainder linearly */
            for (int i = 0; i < 16; i++) {
                if (v128_extract_byte(v, i) == 0) {
                    return NULL;  /* Reached end without finding character */
                }
                if (v128_extract_byte(v, i) == (uint8_t)c) {
                    return (char *)(p + i);
                }
            }
        }

        /* Check for target character */
        Vector128 cmp = v128_eq(v, target);
        if (cmp.lo != 0 || cmp.hi != 0) {
            /* Found - get exact position */
            for (int i = 0; i < 16; i++) {
                if (v128_extract_byte(cmp, i) != 0) {
                    return (char *)(p + i);
                }
            }
        }

        p += 16;
    }
}

/**
 * rosetta_strrchr_simd - Find last occurrence of character
 */
char *rosetta_strrchr_simd(const char *s, int c)
{
    const char *last = NULL;
    const char *p = s;

    /* Find all occurrences, return the last one */
    while ((p = rosetta_strchr_simd(p, c)) != NULL) {
        last = p;
        p++;  /* Continue search from next position */
    }

    return (char *)last;
}

/**
 * rosetta_strstr_simd - Find substring in string
 */
char *rosetta_strstr_simd(const char *haystack, const char *needle)
{
    size_t needle_len = rosetta_strlen_simd(needle);

    if (needle_len == 0) {
        return (char *)haystack;  /* Empty needle matches at start */
    }

    if (needle_len == 1) {
        return rosetta_strchr_simd(haystack, needle[0]);
    }

    const char *p = haystack;

    while (*p) {
        /* Quick check: first character match */
        if (*p == needle[0]) {
            /* Check full match */
            if (rosetta_memcmp_simd(p, needle, needle_len) == 0) {
                return (char *)p;
            }
        }
        p++;
    }

    return NULL;  /* Not found */
}

/* ============================================================================
 * String Copy
 * ============================================================================ */

/**
 * rosetta_strcpy_simd - Copy string using SIMD
 */
char *rosetta_strcpy_simd(char *dest, const char *src)
{
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dest;

    /* Align to 16-byte boundary */
    while ((uintptr_t)s & 0x0F) {
        *d++ = *s;
        if (*s == '\0') {
            return dest;
        }
        s++;
    }

    /* Copy 16 bytes at a time */
    while (1) {
        Vector128 v = v128_load(s);

        if (simd_has_zero_byte(v)) {
            /* Null byte found - copy byte by byte to end */
            for (int i = 0; i < 16; i++) {
                uint8_t byte = v128_extract_byte(v, i);
                *d++ = byte;
                if (byte == '\0') {
                    return dest;
                }
            }
        }

        v128_store(v, d);
        s += 16;
        d += 16;
    }
}

/**
 * rosetta_strncpy_simd - Copy up to n bytes of string
 */
char *rosetta_strncpy_simd(char *dest, const char *src, size_t n)
{
    if (n == 0) {
        return dest;
    }

    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dest;
    size_t copied = 0;

    while (copied < n) {
        if (*s == '\0') {
            /* Pad with zeros */
            memset(d, 0, n - copied);
            break;
        }
        *d++ = *s++;
        copied++;
    }

    return dest;
}

/* ============================================================================
 * Memory Comparison
 * ============================================================================ */

/**
 * rosetta_memcmp_simd - Compare memory regions using SIMD
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    /* Compare 16 bytes at a time */
    while (n >= 16) {
        Vector128 v1 = v128_load(p1);
        Vector128 v2 = v128_load(p2);

        Vector128 diff = v128_xor(v1, v2);
        if (diff.lo != 0 || diff.hi != 0) {
            /* Find first difference */
            for (int i = 0; i < 16; i++) {
                uint8_t b1 = v128_extract_byte(v1, i);
                uint8_t b2 = v128_extract_byte(v2, i);
                if (b1 != b2) {
                    return (int)b1 - (int)b2;
                }
            }
        }

        p1 += 16;
        p2 += 16;
        n -= 16;
    }

    /* Handle remaining bytes */
    while (n-- > 0) {
        if (*p1 != *p2) {
            return (int)*p1 - (int)*p2;
        }
        p1++;
        p2++;
    }

    return 0;  /* Equal */
}
