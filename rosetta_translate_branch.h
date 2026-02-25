/* ============================================================================
 * Rosetta Branch Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 control flow instructions
 * including conditional branches, jumps, calls, returns, and conditional moves.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BRANCH_H
#define ROSETTA_TRANSLATE_BRANCH_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_arm64_emit.h"
#include <stdint.h>

/* ARM64 register constants */
#define X30 30
#define XZR 31

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

/**
 * Translate conditional branch (Jcc)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param block_pc Current block PC (for offset calculation)
 * @return 1 if block ends, 0 otherwise
 */
int translate_branch_jcc(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc);

/**
 * Translate unconditional jump (JMP)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param block_pc Current block PC (for offset calculation)
 * @return 1 if block ends, 0 otherwise
 */
int translate_branch_jmp(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint64_t block_pc);

/**
 * Translate CALL instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param block_pc Current block PC (for offset calculation)
 * @return 1 if block ends, 0 otherwise
 */
int translate_branch_call(CodeBuffer *code_buf, const x86_insn_t *insn,
                          uint64_t block_pc);

/**
 * Translate RET instruction
 * @param code_buf Code buffer for emission
 * @return 1 if block ends, 0 otherwise
 */
int translate_branch_ret(CodeBuffer *code_buf);

/**
 * Translate conditional move (CMOVcc)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_branch_cmov(CodeBuffer *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate SETcc instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 */
void translate_branch_setcc(CodeBuffer *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd);

/**
 * Translate XCHG instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd First ARM register
 * @param arm_rm Second ARM register
 */
void translate_branch_xchg(CodeBuffer *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_BRANCH_H */
