/* ============================================================================
 * Rosetta Refactored - Vector Operations Header
 * ============================================================================
 *
 * This module provides 128-bit vector operations for SIMD emulation
 * and data processing.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_VECTOR_H
#define ROSETTA_REFACTORED_VECTOR_H

#include <stdint.h>
#include <stdbool.h>

/* 128-bit vector type - only define if not already defined */
#ifndef VECTOR128_DEFINED
#ifdef ROSETTA_TYPES_H
typedef v128_t Vector128;
#else
typedef struct {
    uint64_t lo;
    uint64_t hi;
} Vector128;
#endif
#define VECTOR128_DEFINED
#endif

/* ============================================================================
 * Vector Arithmetic Operations
 * ============================================================================ */

/**
 * Vector add - Element-wise addition
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a + b)
 */
Vector128 v128_add(Vector128 a, Vector128 b);

/**
 * Vector subtract - Element-wise subtraction
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a - b)
 */
Vector128 v128_sub(Vector128 a, Vector128 b);

/**
 * Vector multiply - Element-wise multiplication
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a * b)
 */
Vector128 v128_mul(Vector128 a, Vector128 b);

/**
 * Vector NEG - Negate vector (two's complement)
 * @param a Vector to negate
 * @return Result vector (-a)
 */
Vector128 v128_neg(Vector128 a);

/* ============================================================================
 * Vector Logical Operations
 * ============================================================================ */

/**
 * Vector AND - Bitwise AND
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a & b)
 */
Vector128 v128_and(Vector128 a, Vector128 b);

/**
 * Vector ORR - Bitwise OR
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a | b)
 */
Vector128 v128_orr(Vector128 a, Vector128 b);

/**
 * Vector XOR - Bitwise exclusive OR
 * @param a First vector operand
 * @param b Second vector operand
 * @return Result vector (a ^ b)
 */
Vector128 v128_xor(Vector128 a, Vector128 b);

/**
 * Vector NOT - Bitwise NOT
 * @param a Vector to invert
 * @return Result vector (~a)
 */
Vector128 v128_not(Vector128 a);

/* ============================================================================
 * Vector Shift Operations
 * ============================================================================ */

/**
 * Vector SHL - Shift left logical
 * @param a Vector to shift
 * @param shift Amount to shift (0-127)
 * @return Result vector (a << shift)
 */
Vector128 v128_shl(Vector128 a, int shift);

/**
 * Vector SHR - Shift right logical (zero-extended)
 * @param a Vector to shift
 * @param shift Amount to shift (0-127)
 * @return Result vector (a >> shift)
 */
Vector128 v128_shr(Vector128 a, int shift);

/**
 * Vector SAR - Shift right arithmetic (sign-extended)
 * @param a Vector to shift
 * @param shift Amount to shift (0-127)
 * @return Result vector (a >> shift with sign extension)
 */
Vector128 v128_sar(Vector128 a, int shift);

/* ============================================================================
 * Vector Compare Operations
 * ============================================================================ */

/**
 * Vector EQ - Compare for equality
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where equal
 */
Vector128 v128_eq(Vector128 a, Vector128 b);

/**
 * Vector NEQ - Compare for inequality
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where not equal
 */
Vector128 v128_neq(Vector128 a, Vector128 b);

/**
 * Vector LT - Unsigned less than comparison
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where a < b
 */
Vector128 v128_lt(Vector128 a, Vector128 b);

/**
 * Vector GT - Unsigned greater than comparison
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where a > b
 */
Vector128 v128_gt(Vector128 a, Vector128 b);

/**
 * Vector LTE - Unsigned less than or equal comparison
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where a <= b
 */
Vector128 v128_lte(Vector128 a, Vector128 b);

/**
 * Vector GTE - Unsigned greater than or equal comparison
 * @param a First vector
 * @param b Second vector
 * @return Mask vector with 0xFF per byte where a >= b
 */
Vector128 v128_gte(Vector128 a, Vector128 b);

/* ============================================================================
 * Vector Reduce Operations
 * ============================================================================ */

/**
 * Vector UMIN - Unsigned minimum of two vectors
 * @param a First vector operand
 * @param b Second vector operand
 * @return Element-wise minimum
 */
Vector128 v128_umin(Vector128 a, Vector128 b);

/**
 * Vector UMAX - Unsigned maximum of two vectors
 * @param a First vector operand
 * @param b Second vector operand
 * @return Element-wise maximum
 */
Vector128 v128_umax(Vector128 a, Vector128 b);

/**
 * Vector SMIN - Signed minimum of two vectors
 * @param a First vector operand
 * @param b Second vector operand
 * @return Element-wise signed minimum
 */
Vector128 v128_smin(Vector128 a, Vector128 b);

/**
 * Vector SMAX - Signed maximum of two vectors
 * @param a First vector operand
 * @param b Second vector operand
 * @return Element-wise signed maximum
 */
Vector128 v128_smax(Vector128 a, Vector128 b);

/**
 * Vector UMINV - Unsigned minimum across all elements
 * @param a Input vector
 * @return Minimum unsigned byte value
 */
uint8_t v128_uminv(Vector128 a);

/**
 * Vector UMAXV - Unsigned maximum across all elements
 * @param a Input vector
 * @return Maximum unsigned byte value
 */
uint8_t v128_umaxv(Vector128 a);

/**
 * Vector SMINV - Signed minimum across all elements
 * @param a Input vector
 * @return Minimum signed byte value
 */
int8_t v128_sminv(Vector128 a);

/**
 * Vector SMAXV - Signed maximum across all elements
 * @param a Input vector
 * @return Maximum signed byte value
 */
int8_t v128_smaxv(Vector128 a);

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

/**
 * v128_from_ulong - Create vector from unsigned long
 * @param val Value to broadcast
 * @return Vector with low/high set to val
 */
Vector128 v128_from_ulong(uint64_t val);

/**
 * ulong_from_v128 - Extract low 64 bits from vector
 * @param v Input vector
 * @return Low 64 bits
 */
uint64_t ulong_from_v128(Vector128 v);

/**
 * v128_zero - Create a zero vector
 * @return Zero vector
 */
Vector128 v128_zero(void);

/**
 * v128_load - Load vector from memory
 * @param addr Memory address
 * @return Loaded vector
 */
Vector128 v128_load(const void *addr);

/**
 * v128_store - Store vector to memory
 * @param v Vector to store
 * @param addr Memory address
 */
void v128_store(Vector128 v, void *addr);

/**
 * v128_addv - Sum across all vector elements
 * @param a Input vector
 * @return Sum of all bytes
 */
uint64_t v128_addv(Vector128 a);

/* ============================================================================
 * Vector Manipulation (NEON-style)
 * ============================================================================ */

/**
 * v128_dup - Create vector with all bytes set to same value
 * @param val Byte value to broadcast
 * @return Vector with all 16 bytes set to val
 */
Vector128 v128_dup(uint8_t val);

/**
 * v128_extract_byte - Extract byte from vector
 * @param v Input vector
 * @param index Byte index (0-15)
 * @return Extracted byte value
 */
uint8_t v128_extract_byte(Vector128 v, int index);

/**
 * v128_insert_byte - Insert byte into vector
 * @param v Input vector
 * @param index Byte index (0-15)
 * @param val Value to insert
 * @return Modified vector
 */
Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val);

/**
 * v128_zip_lo - Zip/interleave low halves
 * @param a First vector
 * @param b Second vector
 * @return Interleaved low halves
 */
Vector128 v128_zip_lo(Vector128 a, Vector128 b);

/**
 * v128_zip_hi - Zip/interleave high halves
 * @param a First vector
 * @param b Second vector
 * @return Interleaved high halves
 */
Vector128 v128_zip_hi(Vector128 a, Vector128 b);

/* ============================================================================
 * CRC32 Functions
 * ============================================================================ */

/**
 * crc32_byte - Compute CRC32 of a single byte
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC32 value
 */
uint32_t crc32_byte(uint32_t crc, uint8_t byte);

/**
 * crc32_word - Compute CRC32 of a 32-bit word
 * @param crc Current CRC value
 * @param word Word to process
 * @return Updated CRC32 value
 */
uint32_t crc32_word(uint32_t crc, uint32_t word);

#endif /* ROSETTA_REFACTORED_VECTOR_H */
