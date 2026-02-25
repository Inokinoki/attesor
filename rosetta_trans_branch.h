/* ============================================================================
 * Rosetta Translator - Branch Instruction Translation
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 branch instructions
 * (B, BL, BR, B.cond, CBZ, CBNZ, TBZ, TBNZ, RET, etc.)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_BRANCH_H
#define ROSETTA_TRANS_BRANCH_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

/**
 * translate_b - Translate/emulate B (unconditional branch) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_b(ThreadState *state, const uint8_t *insn);

/**
 * translate_bl - Translate/emulate BL (branch with link) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bl(ThreadState *state, const uint8_t *insn);

/**
 * translate_br - Translate/emulate BR (branch to register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_br(ThreadState *state, const uint8_t *insn);

/**
 * translate_bcond - Translate/emulate B.cond (conditional branch) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_bcond(ThreadState *state, const uint8_t *insn);

/**
 * translate_cbz - Translate/emulate CBZ (compare and branch if zero) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cbz(ThreadState *state, const uint8_t *insn);

/**
 * translate_cbnz - Translate/emulate CBNZ (compare and branch if non-zero) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cbnz(ThreadState *state, const uint8_t *insn);

/**
 * translate_tbz - Translate/emulate TBZ (test bit and branch if zero) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbz(ThreadState *state, const uint8_t *insn);

/**
 * translate_tbnz - Translate/emulate TBNZ (test bit and branch if non-zero) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tbnz(ThreadState *state, const uint8_t *insn);

/**
 * translate_ret - Translate/emulate RET (return) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ret(ThreadState *state, const uint8_t *insn);

/**
 * translate_blr - Translate/emulate BLR (branch with link to register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_blr(ThreadState *state, const uint8_t *insn);

/**
 * translate_csel - Translate/emulate CSEL (conditional select) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_csel(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmp - Translate/emulate CMP (compare) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmp(ThreadState *state, const uint8_t *insn);

/**
 * translate_cmn - Translate/emulate CMN (compare negative) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cmn(ThreadState *state, const uint8_t *insn);

/**
 * translate_tst - Translate/emulate TST (test) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_tst(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_BRANCH_H */
