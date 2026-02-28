/* ============================================================================
 * Rosetta Refactored - NEON Load/Store Operations Header
 * ============================================================================
 *
 * This header declares functions for NEON/SIMD load/store instruction
 * translation.
 * ============================================================================ */

#ifndef ROSETTA_NEON_LOADSTORE_H
#define ROSETTA_NEON_LOADSTORE_H

#include "rosetta_types.h"
#include "rosetta_emit_x86.h"
#include "rosetta_refactored.h"
#include <stdint.h>

/* ============================================================================
 * NEON Load Single Structure
 * ============================================================================ */

/**
 * translate_neon_ld1 - Translate ARM64 LD1 (load single structure) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @vec_regs: Vector register state
 * @x_regs: General purpose register state
 * @base_addr: Base address for load
 * Returns: 0 on success, -1 on failure
 */
int translate_neon_ld1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld1_lane - Translate ARM64 LD1 (single lane) instruction
 * LD1 {Vt.<T>}[<index>], [Xn]
 */
int translate_neon_ld1_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld1_rep - Translate ARM64 LD1 (multiple registers) instruction
 * LD1 {Vt1.<T>-Vt4.<T>}, [Xn]
 */
int translate_neon_ld1_rep(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs, uint64_t base_addr);

/* ============================================================================
 * NEON Store Single Structure
 * ============================================================================ */

/**
 * translate_neon_st1 - Translate ARM64 ST1 (store single structure) instruction
 * ST1 {Vt.<T>}, [Xn]
 */
int translate_neon_st1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st1_lane - Translate ARM64 ST1 (single lane) instruction
 * ST1 {Vt.<T>}[<index>], [Xn]
 */
int translate_neon_st1_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st1_rep - Translate ARM64 ST1 (multiple registers) instruction
 * ST1 {Vt1.<T>-Vt4.<T>}, [Xn]
 */
int translate_neon_st1_rep(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs, uint64_t base_addr);

/* ============================================================================
 * NEON Load Multiple Structures
 * ============================================================================ */

/**
 * translate_neon_ld2 - Translate ARM64 LD2 (load two structures) instruction
 * LD2 {Vt1.<T>, Vt2.<T>}, [Xn]
 */
int translate_neon_ld2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld2_lane - Translate ARM64 LD2 (single lane) instruction
 * LD2 {Vt1.<T>, Vt2.<T>}[<index>], [Xn]
 */
int translate_neon_ld2_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld3 - Translate ARM64 LD3 (load three structures) instruction
 * LD3 {Vt1.<T>, Vt2.<T>, Vt3.<T>}, [Xn]
 */
int translate_neon_ld3(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_ld4 - Translate ARM64 LD4 (load four structures) instruction
 * LD4 {Vt1.<T>, Vt2.<T>, Vt3.<T>, Vt4.<T>}, [Xn]
 */
int translate_neon_ld4(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/* ============================================================================
 * NEON Store Multiple Structures
 * ============================================================================ */

/**
 * translate_neon_st2 - Translate ARM64 ST2 (store two structures) instruction
 * ST2 {Vt1.<T>, Vt2.<T>}, [Xn]
 */
int translate_neon_st2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st3 - Translate ARM64 ST3 (store three structures) instruction
 * ST3 {Vt1.<T>, Vt2.<T>, Vt3.<T>}, [Xn]
 */
int translate_neon_st3(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/**
 * translate_neon_st4 - Translate ARM64 ST4 (store four structures) instruction
 * ST4 {Vt1.<T>, Vt2.<T>, Vt3.<T>, Vt4.<T>}, [Xn]
 */
int translate_neon_st4(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr);

/* ============================================================================
 * NEON Load/Store Register (Vector)
 * ============================================================================ */

/**
 * translate_neon_ldr - Translate ARM64 LDR (vector) - load vector register
 * LDR Vt, [Xn|SP, #imm]
 */
int translate_neon_ldr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/**
 * translate_neon_str - Translate ARM64 STR (vector) - store vector register
 * STR Vt, [Xn|SP, #imm]
 */
int translate_neon_str(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/**
 * translate_neon_ldr_lane - Translate ARM64 LDR (vector) with lane indexing
 */
int translate_neon_ldr_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs);

/**
 * translate_neon_str_lane - Translate ARM64 STR (vector) with lane indexing
 */
int translate_neon_str_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs);

/* ============================================================================
 * NEON Load/Store Pair (Vector)
 * ============================================================================ */

/**
 * translate_neon_ldp - Translate ARM64 LDP (vector pair) instruction
 * LDP Vt1, Vt2, [Xn|SP, #imm]
 */
int translate_neon_ldp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/**
 * translate_neon_stp - Translate ARM64 STP (vector pair) instruction
 * STP Vt1, Vt2, [Xn|SP, #imm]
 */
int translate_neon_stp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs);

/* ============================================================================
 * NEON Load/Store with Post-Index
 * ============================================================================ */

/**
 * translate_neon_ldr_post - Translate ARM64 LDR (vector) post-indexed
 * LDR Vt, [Xn], #imm
 */
int translate_neon_ldr_post(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs);

/**
 * translate_neon_str_post - Translate ARM64 STR (vector) post-indexed
 * STR Vt, [Xn], #imm
 */
int translate_neon_str_post(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs);

/* ============================================================================
 * NEON Load/Store with Pre-Index
 * ============================================================================ */

/**
 * translate_neon_ldr_pre - Translate ARM64 LDR (vector) pre-indexed
 * LDR Vt, [Xn, #imm]!
 */
int translate_neon_ldr_pre(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs);

/**
 * translate_neon_str_pre - Translate ARM64 STR (vector) pre-indexed
 * STR Vt, [Xn, #imm]!
 */
int translate_neon_str_pre(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs);

#endif /* ROSETTA_NEON_LOADSTORE_H */
