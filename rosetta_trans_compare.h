/* ============================================================================
 * Rosetta Translator - Compare Instruction Translation Header
 * ============================================================================
 *
 * This header defines the interface for translating ARM64 compare
 * instructions (CMP, CMN, TST) to x86_64.
 * ============================================================================ */

#ifndef ROSETTA_TRANS_COMPARE_H
#define ROSETTA_TRANS_COMPARE_H

#include "rosetta_types.h"

/**
 * translate_cmp - Translate ARM64 CMP (compare) instruction
 * @state: Thread state containing register values
 * @insn: Pointer to ARM64 instruction bytes
 *
 * CMP subtracts two values and updates NZCV flags without storing result.
 * Returns: 0 on success, -1 on error
 */
int translate_cmp(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmp_imm - Translate ARM64 CMP immediate instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * CMP register, #imm12
 */
int translate_cmp_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmn - Translate ARM64 CMN (compare negative) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * CMN adds two values and updates NZCV flags.
 */
int translate_cmn(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmn_imm - Translate ARM64 CMN immediate instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 */
int translate_cmn_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_tst - Translate ARM64 TST (test) instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 *
 * TST performs AND and updates NZ flags.
 */
int translate_tst(ThreadState *state, const uint8_t *insn);

/**
 * translate_tst_imm - Translate ARM64 TST immediate instruction
 * @state: Thread state
 * @insn: Pointer to instruction bytes
 */
int translate_tst_imm(ThreadState *state, const uint8_t *insn);

/**
 * update_nzcv_flags_sub - Update NZCV flags after subtraction
 * @state: Thread state
 * @result: Result of subtraction
 * @op1: First operand (minuend)
 * @op2: Second operand (subtrahend)
 */
void update_nzcv_flags_sub(ThreadState *state, uint64_t result,
                           uint64_t op1, uint64_t op2);

/**
 * update_nzcv_flags_add - Update NZCV flags after addition
 * @state: Thread state
 * @result: Result of addition
 * @op1: First operand
 * @op2: Second operand
 */
void update_nzcv_flags_add(ThreadState *state, uint64_t result,
                           uint64_t op1, uint64_t op2);

/**
 * update_nzcv_flags_and - Update NZ flags after AND operation
 * @state: Thread state
 * @result: Result of AND operation
 */
void update_nzcv_flags_and(ThreadState *state, uint64_t result);

#endif /* ROSETTA_TRANS_COMPARE_H */
