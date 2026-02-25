/* ============================================================================
 * Rosetta Translator - Special Instructions Translation
 * ============================================================================
 *
 * This module handles translation/emulation of special instructions
 * (CPUID, RDTSC, SHLD, SHRD, CQO, CLI, STI, NOP, HLT, etc.)
 * ============================================================================ */

#ifndef ROSETTA_TRANS_SPECIAL_H
#define ROSETTA_TRANS_SPECIAL_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * Special Instruction Translation Functions
 * ============================================================================ */

/**
 * translate_cpuid - Translate/emulate CPUID instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cpuid(ThreadState *state, const uint8_t *insn);

/**
 * translate_rdtsc - Translate/emulate RDTSC instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_rdtsc(ThreadState *state, const uint8_t *insn);

/**
 * translate_shld - Translate/emulate SHLD (double precision shift left) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shld(ThreadState *state, const uint8_t *insn);

/**
 * translate_shrd - Translate/emulate SHRD (double precision shift right) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_shrd(ThreadState *state, const uint8_t *insn);

/**
 * translate_cqo - Translate/emulate CQO (convert quadword to octword) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cqo(ThreadState *state, const uint8_t *insn);

/**
 * translate_cwd - Translate/emulate CWD (convert word to doubleword) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cwd(ThreadState *state, const uint8_t *insn);

/**
 * translate_cdq - Translate/emulate CDQ (convert doubleword to quadword) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cdq(ThreadState *state, const uint8_t *insn);

/**
 * translate_cli - Translate/emulate CLI (clear interrupt flag) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_cli(ThreadState *state, const uint8_t *insn);

/**
 * translate_sti - Translate/emulate STI (set interrupt flag) instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sti(ThreadState *state, const uint8_t *insn);

/**
 * translate_nop - Translate/emulate NOP instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_nop(ThreadState *state, const uint8_t *insn);

/**
 * translate_hlt - Translate/emulate HLT instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_hlt(ThreadState *state, const uint8_t *insn);

/**
 * translate_syscall - Translate/emulate SYSCALL instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_syscall(ThreadState *state, const uint8_t *insn);

/**
 * translate_sysenter - Translate/emulate SYSENTER instruction
 * @param state Thread state
 * @param insn Instruction bytes
 * @return 0 on success, -1 on failure
 */
int translate_sysenter(ThreadState *state, const uint8_t *insn);

#endif /* ROSETTA_TRANS_SPECIAL_H */
