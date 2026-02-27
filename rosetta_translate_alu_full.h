/* ============================================================================
 * Rosetta Translator - Full ALU Translation Header
 * ============================================================================
 *
 * This module provides complete implementation of ARM64 ALU instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_ALU_FULL_H
#define ROSETTA_TRANSLATE_ALU_FULL_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Addition Instructions
 * ============================================================================ */

/**
 * translate_add_reg - Translate ADD (register) instruction
 * ADD Rd, Rn, Rm {, shift #imm}
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_add_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_add_imm - Translate ADD (immediate) instruction
 * ADD Rd, Rn, #imm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_add_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_adc - Translate ADC (add with carry) instruction
 * ADC Rd, Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_adc(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Subtraction Instructions
 * ============================================================================ */

/**
 * translate_sub_reg - Translate SUB (register) instruction
 * SUB Rd, Rn, Rm {, shift #imm}
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_sub_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_sub_imm - Translate SUB (immediate) instruction
 * SUB Rd, Rn, #imm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_sub_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_sbc - Translate SBC (subtract with carry) instruction
 * SBC Rd, Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_sbc(ThreadState *state, const uint8_t *insn);

/**
 * translate_neg - Translate NEG (negate) instruction
 * NEG Rd, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_neg(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Logical Instructions
 * ============================================================================ */

/**
 * translate_and_reg - Translate AND (register) instruction
 * AND Rd, Rn, Rm {, shift #imm}
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_and_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_and_imm - Translate AND (immediate) instruction
 * AND Rd, Rn, #imm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_and_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_orr_reg - Translate ORR (register) instruction
 * ORR Rd, Rn, Rm {, shift #imm}
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_orr_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_eor_reg - Translate EOR (XOR, register) instruction
 * EOR Rd, Rn, Rm {, shift #imm}
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_eor_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_bic - Translate BIC (bit clear) instruction
 * BIC Rd, Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_bic(ThreadState *state, const uint8_t *insn);

/**
 * translate_mvn_reg - Translate MVN (move not, register) instruction
 * MVN Rd, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_mvn_reg(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Compare Instructions
 * ============================================================================ */

/**
 * translate_cmp_reg - Translate CMP (compare, register) instruction
 * CMP Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_cmp_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmp_imm - Translate CMP (immediate) instruction
 * CMP Rn, #imm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_cmp_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmn_reg - Translate CMN (compare negative) instruction
 * CMN Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_cmn_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_tst_reg - Translate TST (test bits) instruction
 * TST Rn, Rm
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success
 */
int translate_tst_reg(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANSLATE_ALU_FULL_H */
