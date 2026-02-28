/* ============================================================================
 * Rosetta Refactored - Vector (v128) Utilities Header
 * ============================================================================
 *
 * This header declares vector (128-bit) utility functions for the
 * Rosetta translation layer.
 *
 * Functions include:
 * - v128 load/store operations
 * - v128 arithmetic operations (add, sub, mul)
 * - v128 logical operations (and, or, eor, not)
 * - v128 shift operations (shl, shr, sar)
 * - v128 compare operations
 * - v128 conversion helpers
 * ============================================================================ */

#ifndef ROSETTA_VECTOR_UTILS_H
#define ROSETTA_VECTOR_UTILS_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * v128 Conversion Helpers
 * ============================================================================ */

/**
 * v128_from_ulong - Create v128 from unsigned long
 * @val: 64-bit value
 * Returns: v128 with value in low 64 bits
 */
vec128_t v128_from_ulong(u64 val);

/**
 * ulong_from_v128 - Extract unsigned long from v128
 * @v: v128 vector
 * Returns: Low 64 bits of vector
 */
u64 ulong_from_v128(vec128_t v);

/**
 * v128_zero - Create zero-initialized v128
 * Returns: v128 with all bits cleared
 */
vec128_t v128_zero(void);

/* ============================================================================
 * v128 Load/Store Operations
 * ============================================================================ */

/**
 * v128_load - Load v128 from memory
 * @p: Pointer to 16-byte aligned memory
 * Returns: Loaded v128 value
 */
vec128_t v128_load(const void *p);

/**
 * v128_store - Store v128 to memory
 * @p: Pointer to 16-byte aligned memory
 * @v: v128 value to store
 */
void v128_store(void *p, vec128_t v);

/* ============================================================================
 * v128 Arithmetic Operations
 * ============================================================================ */

/**
 * v128_add - Vector add (64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: a + b (per lane)
 */
vec128_t v128_add(vec128_t a, vec128_t b);

/**
 * v128_sub - Vector subtract (64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: a - b (per lane)
 */
vec128_t v128_sub(vec128_t a, vec128_t b);

/**
 * v128_mul - Vector multiply (64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: a * b (per lane)
 */
vec128_t v128_mul(vec128_t a, vec128_t b);

/* ============================================================================
 * v128 Logical Operations
 * ============================================================================ */

/**
 * v128_and - Vector bitwise AND
 * @a: First operand
 * @b: Second operand
 * Returns: a & b
 */
vec128_t v128_and(vec128_t a, vec128_t b);

/**
 * v128_orr - Vector bitwise OR
 * @a: First operand
 * @b: Second operand
 * Returns: a | b
 */
vec128_t v128_orr(vec128_t a, vec128_t b);

/**
 * v128_eor - Vector bitwise XOR
 * @a: First operand
 * @b: Second operand
 * Returns: a ^ b
 */
vec128_t v128_eor(vec128_t a, vec128_t b);

/**
 * v128_not - Vector bitwise NOT
 * @a: Operand
 * Returns: ~a
 */
vec128_t v128_not(vec128_t a);

/**
 * v128_neg - Vector negate (two's complement)
 * @a: Operand
 * Returns: -a (per lane)
 */
vec128_t v128_neg(vec128_t a);

/* ============================================================================
 * v128 Shift Operations
 * ============================================================================ */

/**
 * v128_shl - Vector shift left (logical)
 * @a: Operand
 * @s: Shift amount
 * Returns: a << s (per lane)
 */
vec128_t v128_shl(vec128_t a, int s);

/**
 * v128_shr - Vector shift right (logical)
 * @a: Operand
 * @s: Shift amount
 * Returns: a >> s (logical, per lane)
 */
vec128_t v128_shr(vec128_t a, int s);

/**
 * v128_sar - Vector shift right (arithmetic)
 * @a: Operand
 * @s: Shift amount
 * Returns: a >> s (arithmetic, per lane)
 */
vec128_t v128_sar(vec128_t a, int s);

/* ============================================================================
 * v128 Compare Operations
 * ============================================================================ */

/**
 * v128_eq - Vector compare equal (64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: Mask with lanes set where a == b
 */
vec128_t v128_eq(vec128_t a, vec128_t b);

/**
 * v128_ne - Vector compare not equal (64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: Mask with lanes set where a != b
 */
vec128_t v128_ne(vec128_t a, vec128_t b);

/**
 * v128_lt - Vector compare less than (signed, 64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: Mask with lanes set where a < b
 */
vec128_t v128_lt(vec128_t a, vec128_t b);

/**
 * v128_gt - Vector compare greater than (signed, 64-bit lanes)
 * @a: First operand
 * @b: Second operand
 * Returns: Mask with lanes set where a > b
 */
vec128_t v128_gt(vec128_t a, vec128_t b);

/* ============================================================================
 * v128 Pack/Unpack Operations
 * ============================================================================ */

/**
 * v128_pack_lo - Pack low 64 bits from two vectors
 * @a: First operand (provides low 64 bits)
 * @b: Second operand (provides high 64 bits)
 * Returns: Combined v128
 */
vec128_t v128_pack_lo(vec128_t a, vec128_t b);

/**
 * v128_pack_hi - Pack high 64 bits from two vectors
 * @a: First operand (provides low 64 bits)
 * @b: Second operand (provides high 64 bits)
 * Returns: Combined v128 with high lanes
 */
vec128_t v128_pack_hi(vec128_t a, vec128_t b);

/**
 * v128_unpack_lo - Unpack low 64 bits interleaved
 * @a: First operand
 * @b: Second operand
 * Returns: Interleaved low lanes
 */
vec128_t v128_unpack_lo(vec128_t a, vec128_t b);

/**
 * v128_unpack_hi - Unpack high 64 bits interleaved
 * @a: First operand
 * @b: Second operand
 * Returns: Interleaved high lanes
 */
vec128_t v128_unpack_hi(vec128_t a, vec128_t b);

#endif /* ROSETTA_VECTOR_UTILS_H */
