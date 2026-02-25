/*
 * Rosetta Vector Operations Header
 *
 * This module provides 128-bit vector operations for NEON/SIMD
 * emulation and manipulation.
 */

#ifndef ROSETTA_VECTOR_H
#define ROSETTA_VECTOR_H

#include <stdint.h>

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

/**
 * Convert unsigned long to vector
 */
Vector128 v128_from_ulong(uint64_t val);

/**
 * Convert vector to ulong (low 64 bits)
 */
uint64_t ulong_from_v128(Vector128 v);

/**
 * Zero vector
 */
Vector128 v128_zero(void);

/**
 * Load vector from memory
 */
Vector128 v128_load(const void *addr);

/**
 * Store vector to memory
 */
void v128_store(Vector128 v, void *addr);

/**
 * Duplicate byte value across vector
 */
Vector128 v128_dup(uint8_t val);

/**
 * Duplicate 32-bit word across vector
 */
Vector128 v128_dupw(uint32_t val);

/**
 * Extract byte from vector
 */
uint8_t v128_extract_byte(Vector128 v, int index);

/**
 * Insert byte into vector
 */
Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val);

/**
 * Extract 32-bit word from vector
 */
uint32_t v128_extract_word(Vector128 v, int index);

/**
 * Insert 32-bit word into vector
 */
Vector128 v128_insert_word(Vector128 v, int index, uint32_t val);

/**
 * Extract 64-bit dword from vector
 */
uint64_t v128_extract_dword(Vector128 v, int index);

/**
 * Insert 64-bit dword into vector
 */
Vector128 v128_insert_dword(Vector128 v, int index, uint64_t val);

/* ============================================================================
 * Vector Arithmetic Operations
 * ============================================================================ */

Vector128 v128_add(Vector128 a, Vector128 b);
Vector128 v128_sub(Vector128 a, Vector128 b);
Vector128 v128_mul(Vector128 a, Vector128 b);
Vector128 v128_and(Vector128 a, Vector128 b);
Vector128 v128_orr(Vector128 a, Vector128 b);
Vector128 v128_xor(Vector128 a, Vector128 b);
Vector128 v128_not(Vector128 a);
Vector128 v128_neg(Vector128 a);
Vector128 v128_shl(Vector128 a, int shift);
Vector128 v128_shr(Vector128 a, int shift);
Vector128 v128_sar(Vector128 a, int shift);

/* ============================================================================
 * Vector Compare Operations
 * ============================================================================ */

Vector128 v128_eq(Vector128 a, Vector128 b);
Vector128 v128_neq(Vector128 a, Vector128 b);
Vector128 v128_lt(Vector128 a, Vector128 b);
Vector128 v128_gt(Vector128 a, Vector128 b);
Vector128 v128_lte(Vector128 a, Vector128 b);
Vector128 v128_gte(Vector128 a, Vector128 b);

/* ============================================================================
 * Vector Reduce Operations
 * ============================================================================ */

Vector128 v128_umin(Vector128 a, Vector128 b);
Vector128 v128_umax(Vector128 a, Vector128 b);
Vector128 v128_smin(Vector128 a, Vector128 b);
Vector128 v128_smax(Vector128 a, Vector128 b);

/* Vector reduce across elements */
uint8_t v128_uminv(Vector128 a);
uint8_t v128_umaxv(Vector128 a);
int8_t v128_sminv(Vector128 a);
int8_t v128_smaxv(Vector128 a);
uint64_t v128_addv(Vector128 a);

/* ============================================================================
 * Vector Permutation Operations
 * ============================================================================ */

Vector128 v128_zip_lo(Vector128 a, Vector128 b);
Vector128 v128_zip_hi(Vector128 a, Vector128 b);
Vector128 v128_mov(Vector128 v);

#endif /* ROSETTA_VECTOR_H */
