/* ============================================================================
 * Rosetta Refactored - SIMD String/Memory Operations Implementation
 * ============================================================================
 *
 * This module implements SIMD-optimized string and memory operations
 * for the Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_simd_string.h"
#include <string.h>

/* ============================================================================
 * NEON Vector Helpers
 * ============================================================================ */

/**
 * neon_uminv - NEON UMINV - unsigned minimum across vector
 */
u8 neon_uminv(const u8 vec[16])
{
    u8 min_val = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] < min_val) {
            min_val = vec[i];
        }
    }
    return min_val;
}

/**
 * neon_umaxv - NEON UMAXV - unsigned maximum across vector
 */
u8 neon_umaxv(const u8 vec[16])
{
    u8 max_val = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] > max_val) {
            max_val = vec[i];
        }
    }
    return max_val;
}

/**
 * has_zero_byte - Check if 64-bit word contains a zero byte
 *
 * Uses the classic "haszero" algorithm:
 * (x - 0x0101010101010101) & ~x & 0x8080808080808080
 */
u64 has_zero_byte(u64 x)
{
    return (x - 0x0101010101010101ULL) & ~x & 0x8080808080808080ULL;
}

/* ============================================================================
 * SIMD Memory Search
 * ============================================================================ */

/**
 * memchr_simd - SIMD-optimized memory search for zero byte
 *
 * Searches for the first zero byte in a memory block using SIMD operations.
 * The len parameter can be -1 for unlimited search.
 */
void *memchr_simd(const void *ptr, long len)
{
    const u64 *p;
    u64 w0, w1;
    u8 bytes[16];
    u8 min_val;

    if (len < 0) {
        /* Unlimited search */
        p = (const u64 *)((u64)ptr & ~0xfULL);
        w1 = p[1];
        w0 = p[0];

        bytes[0]  = (u8)w0;
        bytes[1]  = (u8)(w0 >> 8);
        bytes[2]  = (u8)(w0 >> 16);
        bytes[3]  = (u8)(w0 >> 24);
        bytes[4]  = (u8)(w0 >> 32);
        bytes[5]  = (u8)(w0 >> 40);
        bytes[6]  = (u8)(w0 >> 48);
        bytes[7]  = (u8)(w0 >> 56);
        bytes[8]  = (u8)w1;
        bytes[9]  = (u8)(w1 >> 8);
        bytes[10] = (u8)(w1 >> 16);
        bytes[11] = (u8)(w1 >> 24);
        bytes[12] = (u8)(w1 >> 32);
        bytes[13] = (u8)(w1 >> 40);
        bytes[14] = (u8)(w1 >> 48);
        bytes[15] = (u8)(w1 >> 56);

        while (1) {
            min_val = neon_uminv(bytes);
            if (min_val == 0) break;

            w1 = p[3];
            w0 = p[2];
            bytes[0]  = (u8)w0;
            bytes[8]  = (u8)w1;
            p += 2;
        }
        return (void *)((u64)p + (min_val - (u64)ptr));
    }

    if (len != 0) {
        /* Bounded search */
        p = (const u64 *)((u64)ptr & ~0xfULL);
        size_t offset = (size_t)len + ((u64)ptr & 0xf);

        while (1) {
            min_val = neon_uminv(bytes);
            if (min_val == 0) {
                if (min_val <= offset) offset = min_val;
                return (void *)((u64)p + (offset - (u64)ptr));
            }
            if (offset < 16) break;
            offset -= 16;
            p += 2;
        }
        return (void *)((u64)p + (offset - (u64)ptr));
    }
    return NULL;
}

/**
 * memchr_simd_unaligned - SIMD-optimized search for unaligned addresses
 */
void *memchr_simd_unaligned(const void *ptr)
{
    const u64 *p = (const u64 *)((u64)ptr & ~0xfULL);
    u64 w0 = p[0], w1 = p[1];
    u8 bytes[16];
    u8 min_val;

    bytes[0]  = (u8)w0;
    bytes[8]  = (u8)w1;

    while (1) {
        min_val = neon_uminv(bytes);
        if (min_val == 0) break;
        w0 = p[2];
        w1 = p[3];
        bytes[0]  = (u8)w0;
        bytes[8]  = (u8)w1;
        p += 2;
    }
    return (void *)((u64)p + (min_val - (u64)ptr));
}

/* ============================================================================
 * SIMD String Compare
 * ============================================================================ */

/**
 * strcmp_simd - SIMD-optimized string compare
 */
int strcmp_simd(const char *s1, const char *s2)
{
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    while (*p1 == *p2 && *p1 != 0) {
        p1++;
        p2++;
    }
    return *p1 - *p2;
}

/**
 * strncmp_simd - SIMD-optimized bounded string compare
 */
int strncmp_simd(const char *s1, const char *s2, size_t n)
{
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    while (n > 0 && *p1 && *p1 == *p2) {
        p1++;
        p2++;
        n--;
    }
    if (n == 0) return 0;
    return *p1 - *p2;
}

/* ============================================================================
 * SIMD Memory Operations
 * ============================================================================ */

/**
 * memcmp_simd - SIMD-optimized memory compare
 */
int memcmp_simd(const void *s1, const void *s2, size_t n)
{
    const u8 *p1 = (const u8 *)s1;
    const u8 *p2 = (const u8 *)s2;

    while (n > 0 && *p1 == *p2) {
        p1++;
        p2++;
        n--;
    }
    if (n == 0) return 0;
    return *p1 - *p2;
}

/**
 * memset_simd - SIMD-optimized memory set
 *
 * Currently wraps standard library memset.
 * Can be enhanced with SIMD instructions for large buffers.
 */
void *memset_simd(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

/**
 * memcpy_simd - SIMD-optimized memory copy
 *
 * Currently wraps standard library memcpy.
 * Can be enhanced with SIMD instructions for large buffers.
 */
void *memcpy_simd(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}
