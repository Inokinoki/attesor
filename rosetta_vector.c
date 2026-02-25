/*
 * Rosetta Vector Operations
 *
 * This module provides 128-bit vector operations for NEON/SIMD
 * emulation and manipulation.
 */

#include "rosetta_refactored.h"
#include <stdint.h>

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

/**
 * Convert unsigned long to vector
 */
Vector128 v128_from_ulong(uint64_t val)
{
    Vector128 result;
    result.lo = val;
    result.hi = val;
    return result;
}

/**
 * Convert vector to ulong (low 64 bits)
 */
uint64_t ulong_from_v128(Vector128 v)
{
    return v.lo;
}

/**
 * Zero vector
 */
Vector128 v128_zero(void)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
    return result;
}

/**
 * Load vector from memory
 */
Vector128 v128_load(const void *addr)
{
    Vector128 result;
    const uint64_t *p = (const uint64_t *)addr;
    result.lo = p[0];
    result.hi = p[1];
    return result;
}

/**
 * Store vector to memory
 */
void v128_store(Vector128 v, void *addr)
{
    uint64_t *p = (uint64_t *)addr;
    p[0] = v.lo;
    p[1] = v.hi;
}

/**
 * Duplicate byte value across vector
 */
Vector128 v128_dup(uint8_t val)
{
    Vector128 result;
    uint64_t pattern = val;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;
    result.lo = pattern;
    result.hi = pattern;
    return result;
}

/**
 * Duplicate 32-bit word across vector
 */
Vector128 v128_dupw(uint32_t val)
{
    Vector128 result;
    result.lo = ((uint64_t)val << 32) | val;
    result.hi = ((uint64_t)val << 32) | val;
    return result;
}

/**
 * Extract byte from vector
 */
uint8_t v128_extract_byte(Vector128 v, int index)
{
    if (index < 0 || index > 15) {
        return 0;
    }
    return ((uint8_t *)&v)[index];
}

/**
 * Insert byte into vector
 */
Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val)
{
    if (index >= 0 && index <= 15) {
        ((uint8_t *)&v)[index] = val;
    }
    return v;
}

/**
 * Extract 32-bit word from vector
 */
uint32_t v128_extract_word(Vector128 v, int index)
{
    if (index < 0 || index > 3) {
        return 0;
    }
    return ((uint32_t *)&v)[index];
}

/**
 * Insert 32-bit word into vector
 */
Vector128 v128_insert_word(Vector128 v, int index, uint32_t val)
{
    if (index >= 0 && index <= 3) {
        ((uint32_t *)&v)[index] = val;
    }
    return v;
}

/**
 * Extract 64-bit dword from vector
 */
uint64_t v128_extract_dword(Vector128 v, int index)
{
    if (index < 0 || index > 1) {
        return 0;
    }
    return ((uint64_t *)&v)[index];
}

/**
 * Insert 64-bit dword into vector
 */
Vector128 v128_insert_dword(Vector128 v, int index, uint64_t val)
{
    if (index >= 0 && index <= 1) {
        ((uint64_t *)&v)[index] = val;
    }
    return v;
}

/* ============================================================================
 * Vector Arithmetic Operations
 * ============================================================================ */

/**
 * Vector add - Add two 128-bit vectors
 */
Vector128 v128_add(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi;
    return result;
}

/**
 * Vector subtract - Subtract two 128-bit vectors
 */
Vector128 v128_sub(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi;
    return result;
}

/**
 * Vector multiply - Multiply two 128-bit vectors
 */
Vector128 v128_mul(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo * b.lo;
    result.hi = a.hi * b.hi;
    return result;
}

/**
 * Vector AND - Bitwise AND of two 128-bit vectors
 */
Vector128 v128_and(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo & b.lo;
    result.hi = a.hi & b.hi;
    return result;
}

/**
 * Vector ORR - Bitwise OR of two 128-bit vectors
 */
Vector128 v128_orr(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo | b.lo;
    result.hi = a.hi | b.hi;
    return result;
}

/**
 * Vector XOR - Bitwise XOR of two 128-bit vectors
 */
Vector128 v128_xor(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo ^ b.lo;
    result.hi = a.hi ^ b.hi;
    return result;
}

/**
 * Vector NOT - Bitwise NOT of a 128-bit vector
 */
Vector128 v128_not(Vector128 a)
{
    Vector128 result;
    result.lo = ~a.lo;
    result.hi = ~a.hi;
    return result;
}

/**
 * Vector NEG - Negate a 128-bit vector (two's complement)
 */
Vector128 v128_neg(Vector128 a)
{
    Vector128 result;
    result.lo = -a.lo;
    result.hi = -a.hi;
    return result;
}

/**
 * Vector SHL - Shift left logical
 */
Vector128 v128_shl(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 64) {
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

/**
 * Vector SHR - Shift right logical (zero-extended)
 */
Vector128 v128_shr(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 64) {
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

/**
 * Vector SAR - Shift right arithmetic (sign-extended)
 */
Vector128 v128_sar(Vector128 a, int shift)
{
    Vector128 result;
    if (shift >= 128) {
        uint64_t sign_mask = -(int64_t)(a.hi >> 63);
        result.lo = sign_mask;
        result.hi = sign_mask;
    } else if (shift == 0) {
        result = a;
    } else {
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

/**
 * Vector EQ - Compare for equality
 */
Vector128 v128_eq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = ~(a.lo ^ b.lo);
    uint64_t cmp_hi = ~(a.hi ^ b.hi);
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

/**
 * Vector NEQ - Compare for inequality
 */
Vector128 v128_neq(Vector128 a, Vector128 b)
{
    Vector128 result;
    uint64_t cmp_lo = a.lo ^ b.lo;
    uint64_t cmp_hi = a.hi ^ b.hi;
    result.lo = (cmp_lo & 0x0101010101010101ULL) * 0xFF;
    result.hi = (cmp_hi & 0x0101010101010101ULL) * 0xFF;
    return result;
}

/**
 * Vector LT - Unsigned less than comparison
 */
Vector128 v128_lt(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = 0;
    result.hi = 0;
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

/**
 * Vector GT - Unsigned greater than comparison
 */
Vector128 v128_gt(Vector128 a, Vector128 b)
{
    return v128_lt(b, a);
}

/**
 * Vector LTE - Unsigned less than or equal comparison
 */
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

/**
 * Vector GTE - Unsigned greater than or equal comparison
 */
Vector128 v128_gte(Vector128 a, Vector128 b)
{
    return v128_lte(b, a);
}

/* ============================================================================
 * Vector Reduce Operations
 * ============================================================================ */

/**
 * Vector UMIN - Unsigned minimum of two vectors
 */
Vector128 v128_umin(Vector128 a, Vector128 b)
{
    Vector128 result;
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        uint8_t min = (a_byte < b_byte) ? a_byte : b_byte;
        result.lo |= ((uint64_t)min << (i * 8));
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        uint8_t min = (a_byte < b_byte) ? a_byte : b_byte;
        result.hi |= ((uint64_t)min << (i * 8));
    }
    return result;
}

/**
 * Vector UMAX - Unsigned maximum of two vectors
 */
Vector128 v128_umax(Vector128 a, Vector128 b)
{
    Vector128 result;
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.lo >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.lo >> (i * 8)) & 0xFF;
        uint8_t max = (a_byte > b_byte) ? a_byte : b_byte;
        result.lo |= ((uint64_t)max << (i * 8));
    }
    for (int i = 0; i < 8; i++) {
        uint8_t a_byte = (a.hi >> (i * 8)) & 0xFF;
        uint8_t b_byte = (b.hi >> (i * 8)) & 0xFF;
        uint8_t max = (a_byte > b_byte) ? a_byte : b_byte;
        result.hi |= ((uint64_t)max << (i * 8));
    }
    return result;
}

/**
 * Vector SMIN - Signed minimum of two vectors
 */
Vector128 v128_smin(Vector128 a, Vector128 b)
{
    Vector128 result;
    for (int i = 0; i < 8; i++) {
        int8_t a_byte = (int8_t)((a.lo >> (i * 8)) & 0xFF);
        int8_t b_byte = (int8_t)((b.lo >> (i * 8)) & 0xFF);
        int8_t min = (a_byte < b_byte) ? a_byte : b_byte;
        result.lo |= ((uint64_t)(uint8_t)min << (i * 8));
    }
    for (int i = 0; i < 8; i++) {
        int8_t a_byte = (int8_t)((a.hi >> (i * 8)) & 0xFF);
        int8_t b_byte = (int8_t)((b.hi >> (i * 8)) & 0xFF);
        int8_t min = (a_byte < b_byte) ? a_byte : b_byte;
        result.hi |= ((uint64_t)(uint8_t)min << (i * 8));
    }
    return result;
}

/**
 * Vector SMAX - Signed maximum of two vectors
 */
Vector128 v128_smax(Vector128 a, Vector128 b)
{
    Vector128 result;
    for (int i = 0; i < 8; i++) {
        int8_t a_byte = (int8_t)((a.lo >> (i * 8)) & 0xFF);
        int8_t b_byte = (int8_t)((b.lo >> (i * 8)) & 0xFF);
        int8_t max = (a_byte > b_byte) ? a_byte : b_byte;
        result.lo |= ((uint64_t)(uint8_t)max << (i * 8));
    }
    for (int i = 0; i < 8; i++) {
        int8_t a_byte = (int8_t)((a.hi >> (i * 8)) & 0xFF);
        int8_t b_byte = (int8_t)((b.hi >> (i * 8)) & 0xFF);
        int8_t max = (a_byte > b_byte) ? a_byte : b_byte;
        result.hi |= ((uint64_t)(uint8_t)max << (i * 8));
    }
    return result;
}

/**
 * Vector UMINV - Unsigned minimum across vector (reduce to scalar)
 */
uint8_t v128_uminv(Vector128 a)
{
    uint8_t min = 0xFF;
    for (int i = 0; i < 16; i++) {
        uint8_t byte = ((uint8_t *)&a)[i];
        if (byte < min) min = byte;
    }
    return min;
}

/**
 * Vector UMAXV - Unsigned maximum across vector (reduce to scalar)
 */
uint8_t v128_umaxv(Vector128 a)
{
    uint8_t max = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t byte = ((uint8_t *)&a)[i];
        if (byte > max) max = byte;
    }
    return max;
}

/**
 * Vector SMINV - Signed minimum across vector (reduce to scalar)
 */
int8_t v128_sminv(Vector128 a)
{
    int8_t min = 0x7F;
    for (int i = 0; i < 16; i++) {
        int8_t byte = (int8_t)((uint8_t *)&a)[i];
        if (byte < min) min = byte;
    }
    return min;
}

/**
 * Vector SMAXV - Signed maximum across vector (reduce to scalar)
 */
int8_t v128_smaxv(Vector128 a)
{
    int8_t max = 0x80;
    for (int i = 0; i < 16; i++) {
        int8_t byte = (int8_t)((uint8_t *)&a)[i];
        if (byte > max) max = byte;
    }
    return max;
}

/**
 * Vector ADDV - Add across vector (reduce to scalar)
 */
uint64_t v128_addv(Vector128 a)
{
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += ((uint8_t *)&a)[i];
    }
    return sum;
}

/* ============================================================================
 * Vector Permutation Operations
 * ============================================================================ */

/**
 * Vector ZIP_LO - Zip low halves of two vectors
 */
Vector128 v128_zip_lo(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.lo;
    result.hi = b.lo;
    return result;
}

/**
 * Vector ZIP_HI - Zip high halves of two vectors
 */
Vector128 v128_zip_hi(Vector128 a, Vector128 b)
{
    Vector128 result;
    result.lo = a.hi;
    result.hi = b.hi;
    return result;
}

/**
 * Vector MOV - Move vector (copy)
 */
Vector128 v128_mov(Vector128 v)
{
    return v;
}
