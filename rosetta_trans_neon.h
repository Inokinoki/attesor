/* ============================================================================
 * Rosetta Translator - NEON/SIMD Translation Header
 * ============================================================================
 *
 * This module provides NEON/SIMD instruction translation functions
 * for ARM64 to x86_64 binary translation.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_NEON_H
#define ROSETTA_TRANS_NEON_H

#include <stdint.h>
#include "rosetta_types.h"
#include "rosetta_jit_emit.h"


/* ============================================================================
 * NEON Arithmetic Translation
 * ============================================================================ */

/**
 * translate_add_vec - Translate ARM64 ADD (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_add_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_sub_vec - Translate ARM64 SUB (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sub_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_mul_vec - Translate ARM64 MUL (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mul_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_mla_vec - Translate ARM64 MLA (vector multiply-accumulate)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mla_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_mls_vec - Translate ARM64 MLS (vector multiply-subtract)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mls_vec(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Logical Translation
 * ============================================================================ */

/**
 * translate_and_vec - Translate ARM64 AND (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_and_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_orr_vec - Translate ARM64 ORR (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_orr_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_eor_vec - Translate ARM64 EOR (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_eor_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_bic_vec - Translate ARM64 BIC (vector) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bic_vec(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Shift Translation
 * ============================================================================ */

/**
 * translate_shl_vec - Translate ARM64 SHL (vector shift left)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shl_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_shr_vec - Translate ARM64 SHR/SHRN (vector shift right)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shr_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_sshr_vec - Translate ARM64 SSHR (signed shift right)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sshr_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_ushr_vec - Translate ARM64 USHR (unsigned shift right)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ushr_vec(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Compare Translation
 * ============================================================================ */

/**
 * translate_cmge_vec - Translate ARM64 CMGE (vector compare >=)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmge_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmhs_vec - Translate ARM64 CMHS (unsigned vector compare >=)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmhs_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmeq_vec - Translate ARM64 CMEQ (vector compare ==)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmeq_vec(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmgt_vec - Translate ARM64 CMGT (vector compare >)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmgt_vec(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Load/Store Translation
 * ============================================================================ */

/**
 * translate_ld1 - Translate ARM64 LD1 (load single structure)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld1(ThreadState *state, const uint8_t *insn);

/**
 * translate_st1 - Translate ARM64 ST1 (store single structure)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st1(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld2 - Translate ARM64 LD2 (load 2 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld2(ThreadState *state, const uint8_t *insn);

/**
 * translate_st2 - Translate ARM64 ST2 (store 2 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st2(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld3 - Translate ARM64 LD3 (load 3 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld3(ThreadState *state, const uint8_t *insn);

/**
 * translate_st3 - Translate ARM64 ST3 (store 3 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st3(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld4 - Translate ARM64 LD4 (load 4 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld4(ThreadState *state, const uint8_t *insn);

/**
 * translate_st4 - Translate ARM64 ST4 (store 4 structures)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st4(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Convert/Reduce Translation
 * ============================================================================ */

/**
 * translate_fcvtns - Translate ARM64 FCVTNS (float to signed int)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcvtns(ThreadState *state, const uint8_t *insn);

/**
 * translate_fcvtnu - Translate ARM64 FCVTNU (float to unsigned int)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcvtnu(ThreadState *state, const uint8_t *insn);

/**
 * translate_scvtf - Translate ARM64 SCVTF (signed int to float)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_scvtf(ThreadState *state, const uint8_t *insn);

/**
 * translate_ucvtf - Translate ARM64 UCVTF (unsigned int to float)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ucvtf(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Table Lookup Translation
 * ============================================================================ */

/**
 * translate_tbl - Translate ARM64 TBL (table lookup)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbl(ThreadState *state, const uint8_t *insn);

/**
 * translate_tbx - Translate ARM64 TBX (table lookup extended)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbx(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Zip/Unzip Translation
 * ============================================================================ */

/**
 * translate_zip - Translate ARM64 ZIP (interleave vectors)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_zip(ThreadState *state, const uint8_t *insn);

/**
 * translate_uzp - Translate ARM64 UZP (unzip vectors)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_uzp(ThreadState *state, const uint8_t *insn);

/**
 * translate_trn - Translate ARM64 TRN (transpose vectors)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_trn(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Saturating Operations
 * ============================================================================ */

/**
 * translate_sqadd - Translate ARM64 SQADD (saturating add)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sqadd(ThreadState *state, const uint8_t *insn);

/**
 * translate_sqsub - Translate ARM64 SQSUB (saturating subtract)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sqsub(ThreadState *state, const uint8_t *insn);

/**
 * translate_sqdadd - Translate ARM64 SQDADD (doubling saturating add)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sqdadd(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Reduction Operations
 * ============================================================================ */

/**
 * translate_addv - Translate ARM64 ADDV (across vector add)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_addv(ThreadState *state, const uint8_t *insn);

/**
 * translate_maxv - Translate ARM64 MAXV (across vector max)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_maxv(ThreadState *state, const uint8_t *insn);

/**
 * translate_minv - Translate ARM64 MINV (across vector min)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_minv(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_NEON_H */
