/* ============================================================================
 * Rosetta Translator - Conditional Instruction Translation Header
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 conditional select
 * and conditional operation instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_COND_H
#define ROSETTA_TRANS_COND_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Condition Code Helper
 * ============================================================================ */

/**
 * Check if condition code is satisfied
 * @param cond Condition code (0-15)
 * @param pstate Processor state (NZCV flags)
 * @return true if condition is true, false otherwise
 */
bool check_condition_code(uint8_t cond, uint64_t pstate);

/**
 * Get inverted condition code
 * @param cond Original condition code
 * @return Inverted condition code
 */
uint8_t get_inverted_condition(uint8_t cond);

/* ============================================================================
 * Conditional Select (CSEL) and Variants
 * ============================================================================ */

/**
 * translate_csel - Translate CSEL (conditional select) instruction
 * CSEL Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csel(ThreadState *state, const uint8_t *insn);

/**
 * translate_csinc - Translate CSINC (conditional select increment) instruction
 * CSINC Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm + 1;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csinc(ThreadState *state, const uint8_t *insn);

/**
 * translate_csinv - Translate CSINV (conditional select invert) instruction
 * CSINV Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = ~Rm;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csinv(ThreadState *state, const uint8_t *insn);

/**
 * translate_csneg - Translate CSNEG (conditional select negate) instruction
 * CSNEG Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = -Rm;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csneg(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Condition Set Instructions
 * ============================================================================ */

/**
 * translate_cset - Translate CSET (condition set) instruction
 * CSET Rd, cond  ->  Rd = (cond) ? 1 : 0;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cset(ThreadState *state, const uint8_t *insn);

/**
 * translate_csetm - Translate CSETM (condition set mask) instruction
 * CSETM Rd, cond  ->  Rd = (cond) ? 0xFFFFFFFFFFFFFFFF : 0;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csetm(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Conditional Operation Instructions (aliases)
 * ============================================================================ */

/**
 * translate_cinc - Translate CINC (conditional increment) instruction
 * CINC Rd, Rn, cond  ->  if (cond) Rd = Rn + 1; else Rd = Rn;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cinc(ThreadState *state, const uint8_t *insn);

/**
 * translate_cinvert - Translate CINVERT (conditional invert) instruction
 * CINVERT Rd, Rn, cond  ->  if (cond) Rd = ~Rn; else Rd = Rn;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cinvert(ThreadState *state, const uint8_t *insn);

/**
 * translate_cneg - Translate CNEG (conditional negate) instruction
 * CNEG Rd, Rn, cond  ->  if (cond) Rd = -Rn; else Rd = Rn;
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cneg(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Vector Conditional Operations
 * ============================================================================ */

/**
 * translate_sel_z - Translate SEL (vector select) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sel_z(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_COND_H */
