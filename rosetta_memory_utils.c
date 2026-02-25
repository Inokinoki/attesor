/* ============================================================================
 * Rosetta Translator - Memory Utilities Implementation
 * ============================================================================
 *
 * This module provides memory operations for the Rosetta binary
 * translation layer, including SIMD-optimized and standard functions.
 * ============================================================================ */

#include "rosetta_memory_utils.h"
#include "rosetta_refactored_helpers.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * SIMD-Optimized Memory Operations (Session 14)
 * ============================================================================ */

/**
 * SIMD-optimized memcpy for aligned data
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

/**
 * SIMD-optimized memset
 */
void *rosetta_memset_simd(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

/**
 * SIMD-optimized memory search for null bytes
 */
void *rosetta_memchr_simd(const void *ptr, long len)
{
    const uint64_t *p;
    uint64_t word0, word1;
    uint64_t shuffled_lo, shuffled_hi;
    uint8_t bytes[16];
    uint8_t min_val;

    /* Lookup tables for SIMD byte search */
    static const uint64_t SHUFFLE_MASK_LO = 0x0c0e0f0d080a0b09ULL;
    static const uint64_t SHUFFLE_MASK_HI = 0x0406070500020301ULL;

    if (len < 0) {
        /* Unlimited search (like strlen/memchr without length) */
        p = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        word1 = p[1];
        word0 = p[0];

        /* Load shuffle masks for alignment */
        shuffled_hi = *((uint64_t*)&SHUFFLE_MASK_HI + 1);
        shuffled_lo = *((uint64_t*)&SHUFFLE_MASK_LO + 1);

        /* OR bytes with shuffle mask to handle alignment */
        bytes[0]  = (uint8_t)word0 | (uint8_t)shuffled_lo;
        bytes[1]  = (uint8_t)(word0 >> 8) | (uint8_t)(shuffled_lo >> 8);
        bytes[2]  = (uint8_t)(word0 >> 16) | (uint8_t)(shuffled_lo >> 16);
        bytes[3]  = (uint8_t)(word0 >> 24) | (uint8_t)(shuffled_lo >> 24);
        bytes[4]  = (uint8_t)(word0 >> 32) | (uint8_t)(shuffled_lo >> 32);
        bytes[5]  = (uint8_t)(word0 >> 40) | (uint8_t)(shuffled_lo >> 40);
        bytes[6]  = (uint8_t)(word0 >> 48) | (uint8_t)(shuffled_lo >> 48);
        bytes[7]  = (uint8_t)(word0 >> 56) | (uint8_t)(shuffled_lo >> 56);
        bytes[8]  = (uint8_t)word1 | (uint8_t)shuffled_hi;
        bytes[9]  = (uint8_t)(word1 >> 8) | (uint8_t)(shuffled_hi >> 8);
        bytes[10] = (uint8_t)(word1 >> 16) | (uint8_t)(shuffled_hi >> 16);
        bytes[11] = (uint8_t)(word1 >> 24) | (uint8_t)(shuffled_hi >> 24);
        bytes[12] = (uint8_t)(word1 >> 32) | (uint8_t)(shuffled_hi >> 32);
        bytes[13] = (uint8_t)(word1 >> 40) | (uint8_t)(shuffled_hi >> 40);
        bytes[14] = (uint8_t)(word1 >> 48) | (uint8_t)(shuffled_hi >> 48);
        bytes[15] = (uint8_t)(word1 >> 56) | (uint8_t)(shuffled_hi >> 56);

        while (1) {
            /* NEON UMINV - find minimum byte across vector */
            min_val = neon_uminv(bytes);

            /* If minimum is 0, we found a null byte */
            if (min_val == 0)
                break;

            /* Load next 16 bytes */
            word1 = p[3];
            word0 = p[2];
            bytes[0]  = (uint8_t)word0;
            bytes[1]  = (uint8_t)(word0 >> 8);
            bytes[2]  = (uint8_t)(word0 >> 16);
            bytes[3]  = (uint8_t)(word0 >> 24);
            bytes[4]  = (uint8_t)(word0 >> 32);
            bytes[5]  = (uint8_t)(word0 >> 40);
            bytes[6]  = (uint8_t)(word0 >> 48);
            bytes[7]  = (uint8_t)(word0 >> 56);
            bytes[8]  = (uint8_t)word1;
            bytes[9]  = (uint8_t)(word1 >> 8);
            bytes[10] = (uint8_t)(word1 >> 16);
            bytes[11] = (uint8_t)(word1 >> 24);
            bytes[12] = (uint8_t)(word1 >> 32);
            bytes[13] = (uint8_t)(word1 >> 40);
            bytes[14] = (uint8_t)(word1 >> 48);
            bytes[15] = (uint8_t)(word1 >> 56);

            p += 2;
        }

        return (void *)((uint64_t)p + (min_val - (uint64_t)ptr));
    }

    if (len != 0) {
        /* Bounded search (like memchr with length) */
        p = (const uint64_t *)((uint64_t)ptr & ~0xfULL);
        word1 = p[1];
        word0 = p[0];

        uint64_t offset = len + ((uint64_t)ptr & 0xf);

        while (1) {
            min_val = neon_uminv(bytes);

            if (min_val == 0) {
                /* Found match within range */
                if (min_val <= offset) {
                    offset = min_val;
                }
                return (void *)((uint64_t)p + (offset - (uint64_t)ptr));
            }

            if (offset < 16 || offset == 16)
                break;

            word1 = p[3];
            word0 = p[2];
            offset -= 16;
            p += 2;
        }

        return (void *)((uint64_t)p + (offset - (uint64_t)ptr));
    }

    return NULL;
}

/**
 * SIMD-optimized memory search for unaligned data
 */
void *rosetta_memchr_simd_unaligned(const void *ptr, long len)
{
    const uint8_t *p = (const uint8_t *)ptr;

    /* Fallback to byte-by-byte for unaligned data */
    while (len > 0) {
        if (*p == 0) {
            return (void *)p;
        }
        p++;
        len--;
    }

    return NULL;
}

/**
 * SIMD-optimized memory compare
 */
int rosetta_memcmp_simd(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    size_t remaining = n;

    /* Handle aligned case with SIMD */
    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1;
        const uint64_t *w2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        while (remaining >= 16) {
            w1 = (const uint64_t *)p1;
            w2 = (const uint64_t *)p2;

            word2 = w2[1];
            word1 = w1[1];

            /* Compare bytes, generate mask of equal bytes */
            cmp_mask[0]  = ((uint8_t)word1 == (uint8_t)word2) ? 0 : 0xff;
            cmp_mask[1]  = ((uint8_t)(word1 >> 8) == (uint8_t)(word2 >> 8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1 >> 16) == (uint8_t)(word2 >> 16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1 >> 24) == (uint8_t)(word2 >> 24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1 >> 32) == (uint8_t)(word2 >> 32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1 >> 40) == (uint8_t)(word2 >> 40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1 >> 48) == (uint8_t)(word2 >> 48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1 >> 56) == (uint8_t)(word2 >> 56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1 >> 8) == (uint8_t)(w2[0] >> 8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1 >> 16) == (uint8_t)(w2[0] >> 16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1 >> 24) == (uint8_t)(w2[0] >> 24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1 >> 32) == (uint8_t)(w2[0] >> 32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1 >> 40) == (uint8_t)(w2[0] >> 40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1 >> 48) == (uint8_t)(w2[0] >> 48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1 >> 56) == (uint8_t)(w2[0] >> 56)) ? 0 : 0xff;

            /* Check if any bytes differ */
            if (neon_uminv(cmp_mask) != 0) {
                /* Found difference - fall back to byte-by-byte */
                break;
            }

            p1 += 16;
            p2 += 16;
            remaining -= 16;
        }
    }

    /* Byte-by-byte comparison for remainder */
    while (remaining > 0) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
        remaining--;
    }

    return 0;
}

/* ============================================================================
 * Standard Memory Operations (Session 14)
 * ============================================================================ */

/**
 * Copy memory region
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
 * Copy memory region with overlap handling
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
 * Compare memory regions
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
 * Set memory region
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
 * Find byte in memory
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

    return NULL;
}

/* ============================================================================
 * Memory Utilities Extended (Session 24)
 * ============================================================================ */

/**
 * Zero-fill memory region
 */
void rosetta_memzero(void *s, size_t n)
{
    memset(s, 0, n);
}

/**
 * Copy memory until character found
 */
void *rosetta_memccpy(void *dest, const void *src, int c, size_t n)
{
    return memccpy(dest, src, c, n);
}

/**
 * Find first byte NOT equal to c
 */
void *rosetta_memchr_inv(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s;
    unsigned char uc = (unsigned char)c;

    while (n-- > 0) {
        if (*p != uc)
            return (void *)p;
        p++;
    }

    return NULL;
}

/**
 * Constant-time memory compare (security-sensitive)
 */
int rosetta_memcmp_consttime(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    unsigned char result = 0;

    for (size_t i = 0; i < n; i++) {
        result |= p1[i] ^ p2[i];
    }

    return (int)result;
}

/**
 * Find first byte equal to any in mask
 */
void *rosetta_memchr_eq(const void *s, const uint8_t mask[16], size_t n)
{
    const uint8_t *p = (const uint8_t *)s;
    size_t i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < 16; j++) {
            if (p[i] == mask[j]) {
                return (void *)&p[i];
            }
        }
    }

    return NULL;
}

/**
 * Optimized memcpy for non-overlapping regions
 */
void *rosetta_memcpy_nonoverlapping(void *dest, const void *src, size_t n)
{
    return rosetta_memcpy(dest, src, n);
}

/**
 * Safe memory move with overlap detection
 */
void *rosetta_memmove_safe(void *dest, const void *src, size_t n)
{
    return rosetta_memmove(dest, src, n);
}

/**
 * Swap contents of two memory regions
 */
void rosetta_memswap(void *a, void *b, size_t n)
{
    uint8_t *pa = (uint8_t *)a;
    uint8_t *pb = (uint8_t *)b;
    uint8_t tmp;
    size_t i;

    for (i = 0; i < n; i++) {
        tmp = pa[i];
        pa[i] = pb[i];
        pb[i] = tmp;
    }
}

/**
 * Fill memory with word pattern
 */
void *rosetta_memfill_word(void *dest, uint64_t word, size_t n)
{
    uint8_t *p = (uint8_t *)dest;
    size_t i;

    for (i = 0; i < n; i++) {
        p[i] = ((uint8_t *)&word)[i % sizeof(word)];
    }

    return dest;
}

/* ============================================================================
 * Memory Management Helpers (Session 24)
 * ============================================================================ */

/**
 * Anonymous memory mapping
 */
void *rosetta_mmap_anonymous(size_t size, int prot)
{
    return mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/**
 * Unmap memory region
 */
int rosetta_munmap_region(void *addr, size_t size)
{
    return munmap(addr, size);
}

/**
 * Change protection on memory region
 */
int rosetta_mprotect_region(void *addr, size_t size, int prot)
{
    return mprotect(addr, size, prot);
}

/**
 * Aligned memory allocation
 */
void *rosetta_memalign(size_t alignment, size_t size)
{
    void *ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}
