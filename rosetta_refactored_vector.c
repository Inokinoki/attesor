/* ============================================================================
 * Rosetta Refactored - Vector Operations Implementation
 * ============================================================================
 *
 * This module implements 128-bit vector operations for SIMD emulation
 * and data processing.
 * ============================================================================ */

#include "rosetta_refactored_vector.h"
#include <string.h>

/* ============================================================================
 * Vector Arithmetic Operations
 * ============================================================================ */

Vector128 v128_add(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi;
    return result;
}

Vector128 v128_sub(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi;
    return result;
}

Vector128 v128_mul(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo * b.lo;
    result.hi = a.hi * b.hi;
    return result;
}

Vector128 v128_neg(Vector128 a)
{
    Vector128 result;
    result.lo = -a.lo;
    result.hi = -a.hi;
    return result;
}

/* ============================================================================
 * Vector Logical Operations
 * ============================================================================ */

Vector128 v128_and(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo & b.lo;
    result.hi = a.hi & b.hi;
    return result;
}

Vector128 v128_orr(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo | b.lo;
    result.hi = a.hi | b.hi;
    return result;
}

Vector128 v128_xor(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo ^ b.lo;
    result.hi = a.hi ^ b.hi;
    return result;
}

Vector128 v128_not(Vector128 a)
{
    Vector128 result;
    result.lo = ~a.lo;
    result.hi = ~a.hi;
    return result;
}

/* ============================================================================
 * Vector Shift Operations
 * ============================================================================ */

Vector128 v128_shl(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 128) {
        result.lo = 0;
        result.hi = 0;
    } else if (shift >= 64) {
        result.hi = a.lo << (shift - 64);
        result.lo = 0;
    } else if (shift == 0) {
        result = a;
    } else {
        result.hi = (a.hi << shift) | (a.lo >> (64 - shift));
        result.lo = a.lo << shift;
    }
    return result;
}

Vector128 v128_shr(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 128) {
        result.lo = 0;
        result.hi = 0;
    } else if (shift >= 64) {
        result.lo = a.hi >> (shift - 64);
        result.hi = 0;
    } else if (shift == 0) {
        result = a;
    } else {
        result.lo = (a.lo >> shift) | (a.hi << (64 - shift));
        result.hi = a.hi >> shift;
    }
    return result;
}

Vector128 v128_sar(Vector128 a, int shift)
{
    /* For arithmetic shift, we need sign extension */
    /* This is a simplified version treating the whole 128-bit as signed */
    Vector128 result;
    if (shift >= 128) {
        /* All bits shifted out - result is sign bit replicated */
        uint64_t sign_mask = -(a.hi >> 63);
        result.lo = sign_mask;
        result.hi = sign_mask;
    } else if (shift == 0) {
        result = a;
    } else {
        /* Simplified: treat as two 64-bit signed values */
        result.hi = (int64_t)a.hi >> shift;
        if (shift < 64) {
            result.lo = (a.lo >> shift) | (a.hi << (64 - shift));
        } else {
            result.lo = (int64_t)a.hi >> (shift - 64);
        }
    }
    return result;
}

/* ============================================================================
 * Vector Compare Operations
 * ============================================================================ */

Vector128 v128_eq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = ~(a.lo ^ b.lo);  /* XOR then invert - equal bits are 1 */
    uint64_t cmp_hi = ~(a.hi ^ b.hi);
    /* Spread each bit to fill a byte */
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

Vector128 v128_neq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = a.lo ^ b.lo;  /* XOR - different bits are 1 */
    uint64_t cmp_hi = a.hi ^ b.hi;
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

Vector128 v128_lt(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    /* Byte-wise unsigned comparison */
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            result.lo |= (0xFFULL << (i * 8));
        }
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            result.hi |= (0xFFULL << (i * 8));
        }
    }
    return result;
}

Vector128 v128_gt(Vector128 a, Vector128 b)
{
    return v128_lt(b, a);
}

Vector128 v128_lte(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t eq_mask_lo = ~(a.lo ^ b.lo);
    uint64_t eq_mask_hi = ~(a.hi ^ b.hi);
    uint64_t lt_only_lo = 0, lt_only_hi = 0;

    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            lt_only_lo |= (0xFFULL << (i * 8));
        }
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        if (a_byte < b_byte) {
            lt_only_hi |= (0xFFULL << (i * 8));
        }
    }

    result.lo = eq_mask_lo | lt_only_lo;
    result.hi = eq_mask_hi | lt_only_hi;
    return result;
}

Vector128 v128_gte(Vector128 a, Vector128 b)
{
    return v128_lte(b, a);
}

/* ============================================================================
 * Vector Reduce Operations
 * ============================================================================ */

Vector128 v128_umin(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = (a.lo < b.lo) ? a.lo : b.lo;
    result.hi = (a.hi < b.hi) ? a.hi : b.hi;
    return result;
}

Vector128 v128_umax(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = (a.lo > b.lo) ? a.lo : b.lo;
    result.hi = (a.hi > b.hi) ? a.hi : b.hi;
    return result;
}

Vector128 v128_smin(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = ((int64_t)a.lo < (int64_t)b.lo) ? a.lo : b.lo;
    result.hi = ((int64_t)a.hi < (int64_t)b.hi) ? a.hi : b.hi;
    return result;
}

Vector128 v128_smax(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = ((int64_t)a.lo > (int64_t)b.lo) ? a.lo : b.lo;
    result.hi = ((int64_t)a.hi > (int64_t)b.hi) ? a.hi : b.hi;
    return result;
}

uint8_t v128_uminv(Vector128 a)
{
    uint8_t min = (uint8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    return min;
}

uint8_t v128_umaxv(Vector128 a)
{
    uint8_t max = (uint8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (uint8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    return max;
}

int8_t v128_sminv(Vector128 a)
{
    int8_t min = (int8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte < min) min = byte;
    }
    return min;
}

int8_t v128_smaxv(Vector128 a)
{
    int8_t max = (int8_t)(a.lo & 0xFF);
    uint64_t tmp = a.lo;
    for (int i = 1; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        int8_t byte = (int8_t)(tmp >> (i * 8));
        if (byte > max) max = byte;
    }
    return max;
}

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

Vector128 v128_from_ulong(uint64_t val)
{
    Vector128 result;
    result.lo = val;
    result.hi = val;
    return result;
}

uint64_t ulong_from_v128(Vector128 v)
{
    return v.lo;
}

Vector128 v128_zero(void)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    return result;
}

Vector128 v128_load(const void *addr)
{
    Vector128 result;
    const uint64_t *ptr = (const uint64_t *)addr;
    result.lo = ptr[0];
    result.hi = ptr[1];
    return result;
}

void v128_store(Vector128 v, void *addr)
{
    uint64_t *ptr = (uint64_t *)addr;
    ptr[0] = v.lo;
    ptr[1] = v.hi;
}

uint64_t v128_addv(Vector128 a)
{
    uint64_t sum = 0;
    uint64_t tmp = a.lo;
    for (int i = 0; i < 8; i++) {
        sum += (tmp >> (i * 8)) & 0xFF;
    }
    tmp = a.hi;
    for (int i = 0; i < 8; i++) {
        sum += (tmp >> (i * 8)) & 0xFF;
    }
    return sum;
}

/* ============================================================================
 * Vector Manipulation (NEON-style)
 * ============================================================================ */

Vector128 v128_dup(uint8_t val)
{
    Vector128 result;
    uint64_t pattern = (uint64_t)val * 0x0101010101010101ULL;
    result.lo = pattern;
    result.hi = pattern;
    return result;
}

uint8_t v128_extract_byte(Vector128 v, int index)
{
    if (index < 0 || index > 15) {
        return 0;  /* Out of bounds */
    }
    if (index < 8) {
        return (v.lo >> (index * 8)) & 0xFF;
    } else {
        return (v.hi >> ((index - 8) * 8)) & 0xFF;
    }
}

Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val)
{
    if (index < 0 || index > 15) {
        return v;  /* Out of bounds */
    }
    uint64_t mask = ~(0xFFULL << (index < 8 ? index * 8 : (index - 8) * 8));
    uint64_t byte_val = (uint64_t)val << (index < 8 ? index * 8 : (index - 8) * 8);

    if (index < 8) {
        v.lo = (v.lo & mask) | byte_val;
    } else {
        v.hi = (v.hi & mask) | byte_val;
    }
    return v;
}

Vector128 v128_zip_lo(Vector128 a, Vector128 b)
{
    /* Interleave bytes from low halves of a and b */
    Vector128 result;
    uint8_t *a_bytes = (uint8_t *)&a.lo;
    uint8_t *b_bytes = (uint8_t *)&b.lo;
    uint8_t *r_bytes = (uint8_t *)&result.lo;

    for (int i = 0; i < 4; i++) {
        r_bytes[i * 2] = a_bytes[i];
        r_bytes[i * 2 + 1] = b_bytes[i];
    }
    /* High half is zero */
    result.hi = 0;
    return result;
}

Vector128 v128_zip_hi(Vector128 a, Vector128 b)
{
    /* Interleave bytes from high halves of a and b */
    Vector128 result;
    uint8_t *a_bytes = (uint8_t *)&a.hi;
    uint8_t *b_bytes = (uint8_t *)&b.hi;
    uint8_t *r_bytes = (uint8_t *)&result.lo;

    for (int i = 0; i < 4; i++) {
        r_bytes[i * 2] = a_bytes[i];
        r_bytes[i * 2 + 1] = b_bytes[i];
    }
    /* High half is zero */
    result.hi = 0;
    return result;
}

/* ============================================================================
 * CRC32 Functions
 * ============================================================================ */

/* CRC32 polynomial (reversed) */
#define CRC32_POLY 0xEDB88320

uint32_t crc32_byte(uint32_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int i = 0; i < 8; i++) {
        crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
    }
    return crc;
}

uint32_t crc32_word(uint32_t crc, uint32_t word)
{
    crc = crc32_byte(crc, (uint8_t)(word & 0xFF));
    crc = crc32_byte(crc, (uint8_t)((word >> 8) & 0xFF));
    crc = crc32_byte(crc, (uint8_t)((word >> 16) & 0xFF));
    crc = crc32_byte(crc, (uint8_t)((word >> 24) & 0xFF));
    return crc;
}
