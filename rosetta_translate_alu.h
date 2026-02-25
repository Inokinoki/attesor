/* ============================================================================
 * Rosetta ALU Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 arithmetic and logic operations
 * to equivalent ARM64 instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_ALU_H
#define ROSETTA_TRANSLATE_ALU_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_arm64_emit.h"
#include <stdint.h>

/* ARM64 register constants */
#define X0  0
#define XZR 31

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

/**
 * Translate ADD instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_add(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate SUB instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_sub(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate AND instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_and(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate OR instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_or(CodeBuffer *code_buf, const x86_insn_t *insn,
                      uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate XOR instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_xor(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate MUL instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rm Source ARM register
 */
void translate_alu_mul(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm);

/**
 * Translate DIV instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rm Source ARM register
 */
void translate_alu_div(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm);

/**
 * Translate INC instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 */
void translate_alu_inc(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd);

/**
 * Translate DEC instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 */
void translate_alu_dec(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd);

/**
 * Translate NEG instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_neg(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate NOT instruction
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_not(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate shift/rotate instructions (SHL, SHR, SAR, ROL, ROR)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_alu_shift(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_ALU_H */
