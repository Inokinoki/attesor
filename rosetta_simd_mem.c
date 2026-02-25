/*
 * Rosetta SIMD-Optimized Memory Operations
 *
 * This module provides SIMD-accelerated memory and string operations
 * using ARM64 NEON instructions for parallel processing.
 */

#include "rosetta_types.h"
#include "rosetta_simd_mem.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Internal Helper Macros
 * ============================================================================ */

/* NEON UMINV - Unsigned Minimum Across Vector (emulated) */
static inline uint8_t neon_uminv(const uint8_t vec[16]) {
    uint8_t min = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] < min) min = vec[i];
    }
    return min;
}

/* ============================================================================
 * Lookup Tables for SIMD Operations
 * ============================================================================ */

/* Lookup tables for SIMD byte search */
static const uint64_t SHUFFLE_MASK_LO = 0x0c0e0f0d080a0b09ULL;
static const uint64_t SHUFFLE_MASK_HI = 0x0406070500020301ULL;

/* ============================================================================
 * SIMD Memory Search Operations
 * ============================================================================ */

/**
 * SIMD-optimized memory search
 *
 * Searches for a null byte (or specific pattern) in memory using NEON
 * instructions for parallel comparison. This is significantly faster
 * than byte-by-byte comparison for large buffers.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search (negative = unlimited)
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd(const void *ptr, long len)
{
    const uint64_t *p;
    uint64_t word0, word1;
    uint64_t shuffled_lo, shuffled_hi;
    uint8_t bytes[16];
    uint8_t min_val;

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
 *
 * Searches for a null byte (or specific pattern) in potentially
 * unaligned memory using NEON instructions.
 *
 * @param ptr Pointer to memory to search
 * @param len Maximum length to search
 * @return Pointer to found byte, or NULL if not found
 */
void *rosetta_memchr_simd_unaligned(const void *ptr, long len)
{
    const uint8_t *p = (const uint8_t *)ptr;
    uint8_t min_val;
    uint8_t bytes[16];

    /* Handle unaligned start by processing byte-by-byte until aligned */
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned_addr = (addr + 15) & ~15ULL;
    size_t prefix_len = aligned_addr - addr;

    if (prefix_len > 0 && prefix_len <= (size_t)len) {
        /* Search prefix byte-by-byte */
        for (size_t i = 0; i < prefix_len; i++) {
            if (p[i] == 0) {
                return (void *)&p[i];
            }
        }
        p += prefix_len;
        len -= prefix_len;
    }

    /* Now p is aligned, use SIMD search */
    while (len >= 16) {
        /* Load 16 bytes */
        for (int i = 0; i < 16; i++) {
            bytes[i] = p[i];
        }

        /* Find minimum byte */
        min_val = neon_uminv(bytes);

        if (min_val == 0) {
            /* Found null byte - locate exact position */
            for (int i = 0; i < 16; i++) {
                if (bytes[i] == 0) {
                    return (void *)&p[i];
                }
            }
        }

        p += 16;
        len -= 16;
    }

    /* Handle remaining bytes */
    while (len > 0) {
        if (*p == 0) {
            return (void *)p;
        }
        p++;
        len--;
    }

    return NULL;
}

/* ============================================================================
 * SIMD String Compare Operations
 * ============================================================================ */

/**
 * SIMD-optimized string comparison
 *
 * Compares two strings using NEON instructions for parallel byte comparison.
 * Returns when a difference is found or null terminator is reached.
 *
 * @param s1 First string
 * @param s2 Second string
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strcmp_simd(const char *s1, const char *s2)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    /* Handle aligned case */
    if (((uint64_t)s1 & 0xf) == 0 && ((uint64_t)s2 & 0xf) == 0) {
        const uint64_t *w1;
        const uint64_t *w2;
        uint64_t word1, word2;
        uint8_t cmp_mask[16];

        do {
            w1 = (const uint64_t *)p1;
            w2 = (const uint64_t *)p2;

            word2 = w2[1];
            word1 = w1[1];

            /* Compare bytes, generate mask of equal bytes */
            cmp_mask[0]  = ((uint8_t)word1 == (uint8_t)word2) ? 0 : 0xff;
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

            p1 += 16;
            p2 += 16;
        } while (neon_uminv(cmp_mask) != 0);

        /* Fall back to byte-by-byte for final comparison */
        while (*p1 == *p2 && *p1 != 0) {
            p1++;
            p2++;
        }

        return *p1 - *p2;
    }

    /* Unaligned case - byte by byte */
    while (*p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

/**
 * SIMD-optimized string comparison with length limit
 *
 * Compares up to n bytes of two strings using NEON instructions
 * for parallel byte comparison.
 *
 * @param s1 First string
 * @param s2 Second string
 * @param n Maximum number of bytes to compare
 * @return Difference between first differing bytes, or 0 if equal
 */
int rosetta_strncmp_simd(const char *s1, const char *s2, size_t n)
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
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

            /* Check if any bytes differ */
            if (neon_uminv(cmp_mask) != 0) {
                /* Found difference - fall back to byte-by-byte to find exact position */
                break;
            }

            p1 += 16;
            p2 += 16;
            remaining -= 16;
        }
    }

    /* Byte-by-byte comparison for remainder or unaligned case */
    while (remaining > 0 && *p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
        remaining--;
    }

    if (remaining == 0) {
        return 0;  /* Reached limit, strings equal up to n bytes */
    }

    return *p1 - *p2;
}

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
            cmp_mask[1]  = ((uint8_t)(word1>>8) == (uint8_t)(word2>>8)) ? 0 : 0xff;
            cmp_mask[2]  = ((uint8_t)(word1>>16) == (uint8_t)(word2>>16)) ? 0 : 0xff;
            cmp_mask[3]  = ((uint8_t)(word1>>24) == (uint8_t)(word2>>24)) ? 0 : 0xff;
            cmp_mask[4]  = ((uint8_t)(word1>>32) == (uint8_t)(word2>>32)) ? 0 : 0xff;
            cmp_mask[5]  = ((uint8_t)(word1>>40) == (uint8_t)(word2>>40)) ? 0 : 0xff;
            cmp_mask[6]  = ((uint8_t)(word1>>48) == (uint8_t)(word2>>48)) ? 0 : 0xff;
            cmp_mask[7]  = ((uint8_t)(word1>>56) == (uint8_t)(word2>>56)) ? 0 : 0xff;
            cmp_mask[8]  = ((uint8_t)word2 == (uint8_t)w2[0]) ? 0 : 0xff;
            cmp_mask[9]  = ((uint8_t)(word1>>8) == (uint8_t)(w2[0]>>8)) ? 0 : 0xff;
            cmp_mask[10] = ((uint8_t)(word1>>16) == (uint8_t)(w2[0]>>16)) ? 0 : 0xff;
            cmp_mask[11] = ((uint8_t)(word1>>24) == (uint8_t)(w2[0]>>24)) ? 0 : 0xff;
            cmp_mask[12] = ((uint8_t)(word1>>32) == (uint8_t)(w2[0]>>32)) ? 0 : 0xff;
            cmp_mask[13] = ((uint8_t)(word1>>40) == (uint8_t)(w2[0]>>40)) ? 0 : 0xff;
            cmp_mask[14] = ((uint8_t)(word1>>48) == (uint8_t)(w2[0]>>48)) ? 0 : 0xff;
            cmp_mask[15] = ((uint8_t)(word1>>56) == (uint8_t)(w2[0]>>56)) ? 0 : 0xff;

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
 * SIMD Memory Copy Operations
 * ============================================================================ */

/**
 * SIMD-optimized memcpy for aligned memory regions
 * @param dest Destination buffer
 * @param src Source buffer
 * @param n Number of bytes to copy
 * @return Destination pointer
 */
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n)
{
    uint64_t *d64 = (uint64_t *)dest;
    const uint64_t *s64 = (const uint64_t *)src;

    /* Copy 16 bytes at a time using SIMD-style operations */
    while (n >= 16) {
        d64[0] = s64[0];
        d64[1] = s64[1];
        d64 += 2;
        s64 += 2;
        n -= 16;
    }

    /* Copy 8 bytes at a time */
    while (n >= 8) {
        *d64++ = *s64++;
        n -= 8;
    }

    /* Copy remaining bytes */
    uint8_t *d8 = (uint8_t *)d64;
    const uint8_t *s8 = (const uint8_t *)s64;
    while (n--) {
        *d8++ = *s8++;
    }

    return dest;
}

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
void *rosetta_memset_simd(void *s, int c, size_t n)
{
    uint64_t *p64;
    uint8_t *p8;
    uint64_t pattern;
    size_t i;

    /* Build 8-byte pattern for faster filling */
    pattern = (uint8_t)c;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    p64 = (uint64_t *)s;

    /* Set 8 bytes at a time */
    i = n / 8;
    while (i--) {
        *p64++ = pattern;
    }

    /* Set remaining bytes */
    p8 = (uint8_t *)p64;
    i = n % 8;
    while (i--) {
        *p8++ = (uint8_t)c;
    }

    return s;
}
