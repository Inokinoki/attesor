/* ============================================================================
 * Rosetta Special Instructions Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 special instructions including
 * CPUID, RDTSC, SHLD, SHRD, CQO, and interrupt control instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_SPECIAL_H
#define ROSETTA_TRANSLATE_SPECIAL_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_arm64_emit.h"
#include <stdint.h>

/* ============================================================================
 * Special Instruction Translation Functions
 * ============================================================================ */

/**
 * Translate CPUID (CPU identification)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_cpuid(CodeBuffer *code_buf, const x86_insn_t *insn);

/**
 * Translate RDTSC (read timestamp counter)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_rdtsc(CodeBuffer *code_buf, const x86_insn_t *insn);

/**
 * Translate SHLD (double precision shift left)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_special_shld(CodeBuffer *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate SHRD (double precision shift right)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_special_shrd(CodeBuffer *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate CQO/CWD/CDQ (sign extend RAX to RDX:RAX)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_cqo(CodeBuffer *code_buf, const x86_insn_t *insn);

/**
 * Translate CLI (clear interrupt flag)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_cli(CodeBuffer *code_buf, const x86_insn_t *insn);

/**
 * Translate STI (set interrupt flag)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_sti(CodeBuffer *code_buf, const x86_insn_t *insn);

/**
 * Translate NOP (no operation)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 */
void translate_special_nop(CodeBuffer *code_buf, const x86_insn_t *insn);

#endif /* ROSETTA_TRANSLATE_SPECIAL_H */
