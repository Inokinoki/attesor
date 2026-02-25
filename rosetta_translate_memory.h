/* ============================================================================
 * Rosetta Memory Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 memory operations including
 * MOV, MOVZX, MOVSX, LEA, PUSH, and POP instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_MEMORY_H
#define ROSETTA_TRANSLATE_MEMORY_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_codegen.h"

/* ============================================================================
 * Memory Translation Functions
 * ============================================================================ */

/**
 * Translate MOV instruction (register and immediate)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_memory_mov(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate MOV with zero-extend (MOVZX)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_memory_movzx(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate MOV with sign-extend (MOVSX)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_memory_movsx(code_buffer_t *code_buf, const x86_insn_t *insn,
                            uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate MOV with sign-extend to doubleword (MOVSXD)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_memory_movsxd(code_buffer_t *code_buf, const x86_insn_t *insn,
                             uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate LEA (load effective address)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 */
void translate_memory_lea(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd);

/**
 * Translate PUSH instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Source ARM register
 */
void translate_memory_push(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd);

/**
 * Translate POP instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 */
void translate_memory_pop(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd);

/**
 * Translate CMP instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd First operand ARM register
 * @param arm_rm Second operand ARM register
 */
void translate_memory_cmp(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate TEST instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd First operand ARM register
 * @param arm_rm Second operand ARM register
 */
void translate_memory_test(code_buffer_t *code_buf, const x86_insn_t *insn,
                           uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_MEMORY_H */
