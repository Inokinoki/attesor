/* ============================================================================
 * Rosetta Translator - Floating Point Translation
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 floating point
 * and NEON/SIMD instructions (FMOV, FADD, FSUB, FMUL, FDIV, FSQRT, etc.)
 * ============================================================================ */

#ifndef ROSETTA_FP_TRANSLATE_H
#define ROSETTA_FP_TRANSLATE_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Floating Point Translation Functions
 * ============================================================================ */

/**
 * translate_fmov - Translate/emulate FMOV (floating-point move) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fmov(ThreadState *state, const uint8_t *insn);

/**
 * translate_fadd - Translate/emulate FADD (floating-point add) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fadd(ThreadState *state, const uint8_t *insn);

/**
 * translate_fsub - Translate/emulate FSUB (floating-point subtract) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fsub(ThreadState *state, const uint8_t *insn);

/**
 * translate_fmul - Translate/emulate FMUL (floating-point multiply) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fmul(ThreadState *state, const uint8_t *insn);

/**
 * translate_fdiv - Translate/emulate FDIV (floating-point divide) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fdiv(ThreadState *state, const uint8_t *insn);

/**
 * translate_fsqrt - Translate/emulate FSQRT (floating-point square root) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fsqrt(ThreadState *state, const uint8_t *insn);

/**
 * translate_fcmp - Translate/emulate FCMP (floating-point compare) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcmp(ThreadState *state, const uint8_t *insn);

/**
 * translate_fcvt - Translate/emulate FCVT (floating-point convert) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcvt(ThreadState *state, const uint8_t *insn);

/**
 * translate_fcsel - Translate/emulate FCSEL (floating-point conditional select) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_fcsel(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON/SIMD Translation Functions
 * ============================================================================ */

/**
 * translate_ld1 - Translate/emulate LD1 (load single structure) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld1(ThreadState *state, const uint8_t *insn);

/**
 * translate_st1 - Translate/emulate ST1 (store single structure) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st1(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld2 - Translate/emulate LD2 (load pair of structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld2(ThreadState *state, const uint8_t *insn);

/**
 * translate_st2 - Translate/emulate ST2 (store pair of structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st2(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld3 - Translate/emulate LD3 (load three structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld3(ThreadState *state, const uint8_t *insn);

/**
 * translate_st3 - Translate/emulate ST3 (store three structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st3(ThreadState *state, const uint8_t *insn);

/**
 * translate_ld4 - Translate/emulate LD4 (load four structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ld4(ThreadState *state, const uint8_t *insn);

/**
 * translate_st4 - Translate/emulate ST4 (store four structures) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_st4(ThreadState *state, const uint8_t *insn);

/**
 * translate_dup - Translate/emulate DUP (duplicate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_dup(ThreadState *state, const uint8_t *insn);

/**
 * translate_ext - Translate/emulate EXT (extract) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ext(ThreadState *state, const uint8_t *insn);

/**
 * translate_tbl - Translate/emulate TBL (table lookup) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbl(ThreadState *state, const uint8_t *insn);

/**
 * translate_tbx - Translate/emulate TBX (table lookup extension) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbx(ThreadState *state, const uint8_t *insn);

/**
 * translate_ushr - Translate/emulate USHR (unsigned shift right) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ushr(ThreadState *state, const uint8_t *insn);

/**
 * translate_sshr - Translate/emulate SSHR (signed shift right) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sshr(ThreadState *state, const uint8_t *insn);

/**
 * translate_shl - Translate/emulate SHL (shift left) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shl(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_FP_TRANSLATE_H */
