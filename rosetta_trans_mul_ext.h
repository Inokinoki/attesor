/* ============================================================================
 * Rosetta Translator - Multiply/Divide Extension Translation Header
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 multiply and divide
 * extension instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_MUL_EXT_H
#define ROSETTA_TRANS_MUL_EXT_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Multiply-Accumulate Instructions
 * ============================================================================ */

/**
 * translate_madd - Translate MADD (multiply add) instruction
 * MADD Rd, Rn, Rm, Ra  ->  Rd = Rn * Rm + Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_madd(ThreadState *state, const uint8_t *insn);

/**
 * translate_msub - Translate MSUB (multiply subtract) instruction
 * MSUB Rd, Rn, Rm, Ra  ->  Rd = Rn * Rm - Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msub(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Long Multiply-Accumulate Instructions
 * ============================================================================ */

/**
 * translate_smaddl - Translate SMADDL (signed multiply add long) instruction
 * SMADDL Rd, Rn, Rm, Ra  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0]) + Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smaddl(ThreadState *state, const uint8_t *insn);

/**
 * translate_smsubl - Translate SMSUBL (signed multiply subtract long) instruction
 * SMSUBL Rd, Rn, Rm, Ra  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0]) - Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smsubl(ThreadState *state, const uint8_t *insn);

/**
 * translate_umaddl - Translate UMADDL (unsigned multiply add long) instruction
 * UMADDL Rd, Rn, Rm, Ra  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0]) + Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_umaddl(ThreadState *state, const uint8_t *insn);

/**
 * translate_umsubl - Translate UMSUBL (unsigned multiply subtract long) instruction
 * UMSUBL Rd, Rn, Rm, Ra  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0]) - Ra
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_umsubl(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Basic Multiply Instructions
 * ============================================================================ */

/**
 * translate_mul - Translate MUL (multiply) instruction
 * MUL Rd, Rn, Rm  ->  Rd = Rn * Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mul(ThreadState *state, const uint8_t *insn);

/**
 * translate_smull - Translate SMULL (signed multiply long) instruction
 * SMULL Rd, Rn, Rm  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0])
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smull(ThreadState *state, const uint8_t *insn);

/**
 * translate_umull - Translate UMULL (unsigned multiply long) instruction
 * UMULL Rd, Rn, Rm  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0])
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_umull(ThreadState *state, const uint8_t *insn);

/**
 * translate_smulh - Translate SMULH (signed multiply high) instruction
 * SMULH Rd, Rn, Rm  ->  Rd = (sext(Rn) * sext(Rm)) >> 64
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smulh(ThreadState *state, const uint8_t *insn);

/**
 * translate_umulh - Translate UMULH (unsigned multiply high) instruction
 * UMULH Rd, Rn, Rm  ->  Rd = (zext(Rn) * zext(Rm)) >> 64
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_umulh(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Division Instructions
 * ============================================================================ */

/**
 * translate_sdiv - Translate SDIV (signed divide) instruction
 * SDIV Rd, Rn, Rm  ->  Rd = sext(Rn) / sext(Rm)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sdiv(ThreadState *state, const uint8_t *insn);

/**
 * translate_udiv - Translate UDIV (unsigned divide) instruction
 * UDIV Rd, Rn, Rm  ->  Rd = zext(Rn) / zext(Rm)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_udiv(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Remainder Instructions
 * ============================================================================ */

/**
 * translate_smod - Translate SMOD (signed remainder) instruction
 * SMOD Rd, Rn, Rm  ->  Rd = sext(Rn) % sext(Rm)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_smod(ThreadState *state, const uint8_t *insn);

/**
 * translate_umod - Translate UMOD (unsigned remainder) instruction
 * UMOD Rd, Rn, Rm  ->  Rd = zext(Rn) % zext(Rm)
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_umod(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_MUL_EXT_H */
