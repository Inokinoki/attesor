/* ============================================================================
 * Rosetta Translator - Memory Instruction Translation
 * ============================================================================
 *
 * This module handles translation/emulation of ARM64 load/store instructions
 * (LDR, STR, LDP, STP, LDRB, STRB, LDRH, STRH, etc.)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_MEM_H
#define ROSETTA_TRANS_MEM_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Memory Translation Functions
 * ============================================================================ */

/**
 * translate_ldr - Translate/emulate LDR (load register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr(ThreadState *state, const uint8_t *insn);

/**
 * translate_str - Translate/emulate STR (store register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldp - Translate/emulate LDP (load pair) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldp(ThreadState *state, const uint8_t *insn);

/**
 * translate_stp - Translate/emulate STP (store pair) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stp(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldrb - Translate/emulate LDRB (load register byte) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrb(ThreadState *state, const uint8_t *insn);

/**
 * translate_strb - Translate/emulate STRB (store register byte) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strb(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldrh - Translate/emulate LDRH (load register halfword) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrh(ThreadState *state, const uint8_t *insn);

/**
 * translate_strh - Translate/emulate STRH (store register halfword) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_strh(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldrsb - Translate/emulate LDRSB (load register signed byte) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsb(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldrsh - Translate/emulate LDRSH (load register signed halfword) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsh(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldrsw - Translate/emulate LDRSW (load register signed word) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldrsw(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldr_reg - Translate/emulate LDR (register offset) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_str_reg - Translate/emulate STR (register offset) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_reg(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldr_pre - Translate/emulate LDR (pre-index) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_pre(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldr_post - Translate/emulate LDR (post-index) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldr_post(ThreadState *state, const uint8_t *insn);

/**
 * translate_str_pre - Translate/emulate STR (pre-index) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_pre(ThreadState *state, const uint8_t *insn);

/**
 * translate_str_post - Translate/emulate STR (post-index) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_str_post(ThreadState *state, const uint8_t *insn);

/**
 * translate_ldp_imm - Translate/emulate LDP (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_ldp_imm(ThreadState *state, const uint8_t *insn);

/**
 * translate_stp_imm - Translate/emulate STP (immediate) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_stp_imm(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * System Register Translation Functions
 * ============================================================================ */

/**
 * translate_mrs - Translate/emulate MRS (move from system register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_mrs(ThreadState *state, const uint8_t *insn);

/**
 * translate_msr - Translate/emulate MSR (move to system register) instruction
 * @param state Thread state
 * @param insn ARM64 instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_msr(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_MEM_H */
