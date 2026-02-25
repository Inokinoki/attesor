/* ============================================================================
 * Rosetta Translator - ALU Instruction Translation
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 arithmetic and logic
 * instructions (ADD, SUB, AND, ORR, EOR, MUL, DIV, MVN, etc.)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_ALU_H
#define ROSETTA_TRANS_ALU_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Flag Update Helpers
 * ============================================================================ */

/**
 * Update NZCV flags after arithmetic operation
 * @param state Thread state
 * @param result Operation result
 * @param op1 First operand
 * @param op2 Second operand
 * @param is_add True for addition, false for subtraction
 * @param is_logical True for logical operation (no C/V flags)
 */
void update_nzcv_flags(ThreadState *state, uint64_t result, uint64_t op1,
                       uint64_t op2, bool is_add, bool is_logical);

/**
 * Update NZCV flags after logical operation (AND, ORR, EOR)
 * @param state Thread state
 * @param result Operation result
 */
void update_nzcv_flags_and(ThreadState *state, uint64_t result);

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

/**
 * translate_add - Translate/emulate ARM64 ADD instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_add(ThreadState *state, const uint8_t *insn);

/**
 * translate_sub - Translate/emulate ARM64 SUB instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sub(ThreadState *state, const uint8_t *insn);

/**
 * translate_and - Translate/emulate ARM64 AND instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_and(ThreadState *state, const uint8_t *insn);

/**
 * translate_orr - Translate/emulate ARM64 ORR instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_orr(ThreadState *state, const uint8_t *insn);

/**
 * translate_eor - Translate/emulate ARM64 EOR instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_eor(ThreadState *state, const uint8_t *insn);

/**
 * translate_mul - Translate/emulate ARM64 MUL instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mul(ThreadState *state, const uint8_t *insn);

/**
 * translate_div - Translate/emulate ARM64 UDIV/SDIV instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_div(ThreadState *state, const uint8_t *insn);

/**
 * translate_mvn - Translate/emulate ARM64 MVN instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mvn(ThreadState *state, const uint8_t *insn);

/**
 * translate_add_imm - Translate/emulate ADD (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_add_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_sub_imm - Translate/emulate SUB (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sub_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_and_imm - Translate/emulate AND (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_and_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_orr_imm - Translate/emulate ORR (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_orr_imm(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_ALU_H */
