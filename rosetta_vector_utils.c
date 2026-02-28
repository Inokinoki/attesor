/* ============================================================================
 * Rosetta Refactored - Vector (v128) Utilities Implementation
 * ============================================================================
 *
 * This module implements vector (128-bit) utility functions for the
 * Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_vector_utils.h"
#include <string.h>

/* ============================================================================
 * v128 Conversion Helpers
 * ============================================================================ */

vec128_t v128_from_ulong(u64 val)
{
    vec128_t result;
    result.u64[0] = val;
    result.u64[1] = 0;
    return result;
}

u64 ulong_from_v128(vec128_t v)
{
    return v.u64[0];
}

vec128_t v128_zero(void)
{
    vec128_t result;
    result.u64[0] = 0;
    result.u64[1] = 0;
    return result;
}

/* ============================================================================
 * v128 Load/Store Operations
 * ============================================================================ */

vec128_t v128_load(const void *p)
{
    return *(const vec128_t *)p;
}

void v128_store(void *p, vec128_t v)
{
    *(vec128_t *)p = v;
}

/* ============================================================================
 * v128 Arithmetic Operations
 * ============================================================================ */

vec128_t v128_add(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] + b.u64[0];
    result.u64[1] = a.u64[1] + b.u64[1];
    return result;
}

vec128_t v128_sub(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] - b.u64[0];
    result.u64[1] = a.u64[1] - b.u64[1];
    return result;
}

vec128_t v128_mul(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] * b.u64[0];
    result.u64[1] = a.u64[1] * b.u64[1];
    return result;
}

/* ============================================================================
 * v128 Logical Operations
 * ============================================================================ */

vec128_t v128_and(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] & b.u64[0];
    result.u64[1] = a.u64[1] & b.u64[1];
    return result;
}

vec128_t v128_orr(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] | b.u64[0];
    result.u64[1] = a.u64[1] | b.u64[1];
    return result;
}

vec128_t v128_eor(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0] ^ b.u64[0];
    result.u64[1] = a.u64[1] ^ b.u64[1];
    return result;
}

vec128_t v128_not(vec128_t a)
{
    vec128_t result;
    result.u64[0] = ~a.u64[0];
    result.u64[1] = ~a.u64[1];
    return result;
}

vec128_t v128_neg(vec128_t a)
{
    vec128_t result;
    result.u64[0] = (u64)-(s64)a.u64[0];
    result.u64[1] = (u64)-(s64)a.u64[1];
    return result;
}

/* ============================================================================
 * v128 Shift Operations
 * ============================================================================ */

vec128_t v128_shl(vec128_t a, int s)
{
    vec128_t result;
    if (s >= 64) {
        result.u64[1] = (s >= 64) ? (a.u64[0] << (s - 64)) : 0;
        result.u64[0] = 0;
    } else if (s > 0) {
        result.u64[1] = (a.u64[1] << s) | (a.u64[0] >> (64 - s));
        result.u64[0] = a.u64[0] << s;
    } else {
        result = a;
    }
    return result;
}

vec128_t v128_shr(vec128_t a, int s)
{
    vec128_t result;
    if (s >= 64) {
        result.u64[0] = (s >= 64) ? (a.u64[1] >> (s - 64)) : 0;
        result.u64[1] = 0;
    } else if (s > 0) {
        result.u64[0] = (a.u64[0] >> s) | (a.u64[1] << (64 - s));
        result.u64[1] = a.u64[1] >> s;
    } else {
        result = a;
    }
    return result;
}

vec128_t v128_sar(vec128_t a, int s)
{
    vec128_t result;
    if (s >= 64) {
        result.u64[0] = (s64)a.u64[1] >> 63;  /* Sign extend */
        result.u64[1] = (s64)a.u64[1] >> 63;
    } else if (s > 0) {
        result.u64[0] = (a.u64[0] >> s) | (a.u64[1] << (64 - s));
        result.u64[1] = (u64)((s64)a.u64[1] >> s);
    } else {
        result = a;
    }
    return result;
}

/* ============================================================================
 * v128 Compare Operations
 * ============================================================================ */

vec128_t v128_eq(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = (a.u64[0] == b.u64[0]) ? ~0ULL : 0ULL;
    result.u64[1] = (a.u64[1] == b.u64[1]) ? ~0ULL : 0ULL;
    return result;
}

vec128_t v128_ne(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = (a.u64[0] != b.u64[0]) ? ~0ULL : 0ULL;
    result.u64[1] = (a.u64[1] != b.u64[1]) ? ~0ULL : 0ULL;
    return result;
}

vec128_t v128_lt(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = ((s64)a.u64[0] < (s64)b.u64[0]) ? ~0ULL : 0ULL;
    result.u64[1] = ((s64)a.u64[1] < (s64)b.u64[1]) ? ~0ULL : 0ULL;
    return result;
}

vec128_t v128_gt(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = ((s64)a.u64[0] > (s64)b.u64[0]) ? ~0ULL : 0ULL;
    result.u64[1] = ((s64)a.u64[1] > (s64)b.u64[1]) ? ~0ULL : 0ULL;
    return result;
}

/* ============================================================================
 * v128 Pack/Unpack Operations
 * ============================================================================ */

vec128_t v128_pack_lo(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0];
    result.u64[1] = b.u64[0];
    return result;
}

vec128_t v128_pack_hi(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[1];
    result.u64[1] = b.u64[1];
    return result;
}

vec128_t v128_unpack_lo(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[0];
    result.u64[1] = b.u64[0];
    return result;
}

vec128_t v128_unpack_hi(vec128_t a, vec128_t b)
{
    vec128_t result;
    result.u64[0] = a.u64[1];
    result.u64[1] = b.u64[1];
    return result;
}
