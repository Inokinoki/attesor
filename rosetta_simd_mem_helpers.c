/* ============================================================================
 * Rosetta Translator - SIMD Memory Helpers Implementation
 * ============================================================================
 *
 * SIMD-optimized memory and string operations.
 * ============================================================================ */

#include "rosetta_simd_mem_helpers.h"
#include <string.h>

/* NEON UMINV helper - finds minimum byte in 16-byte vector */
static inline uint8_t neon_uminv(const uint8_t *bytes)
{
    uint8_t min_val = bytes[0];
    for (int i = 1; i < 16; i++) {
        if (bytes[i] < min_val) {
            min_val = bytes[i];
        }
    }
    return min_val;
}

/* Lookup tables for SIMD byte search */
static const uint64_t SHUFFLE_MASK_LO = 0x0c0e0f0d080a0b09ULL;
static const uint64_t SHUFFLE_MASK_HI = 0x0406070500020301ULL;

/* ============================================================================
 * SIMD Memory Search Functions
 * ============================================================================ */

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
 * SIMD Memory Copy and Set
 * ============================================================================ */

void *rosetta_memcpy_simd(void *dest, const void *src, size_t n)
{
    /* For small copies, use standard memcpy */
    if (n < 64) {
        return memcpy(dest, src, n);
    }

    /* For large copies, could use SIMD instructions */
    /* This is a placeholder - full implementation would use NEON load/store */
    return memcpy(dest, src, n);
}

void *rosetta_memset_simd(void *s, int c, size_t n)
{
    /* For small fills, use standard memset */
    if (n < 64) {
        return memset(s, c, n);
    }

    /* For large fills, could use SIMD instructions */
    /* This is a placeholder - full implementation would use NEON dup + store */
    return memset(s, c, n);
}
