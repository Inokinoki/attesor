/* ============================================================================
 * Rosetta Bit Manipulation Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 bit manipulation instructions
 * including BSF, BSR, POPCNT, and BT/BTS/BTR/BTC operations.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_BIT_H
#define ROSETTA_TRANSLATE_BIT_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_codegen.h"
#include <stdint.h>

/* ============================================================================
 * Bit Manipulation Translation Functions
 * ============================================================================ */

/**
 * Translate BSF (bit scan forward)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_bsf(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate BSR (bit scan reverse)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_bsr(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate POPCNT (population count)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_popcnt(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate BT (bit test)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_bt(code_buffer_t *code_buf, const x86_insn_t *insn,
                      uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate BTS (bit test and set)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_bts(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate BTR (bit test and reset)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_btr(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

/**
 * Translate BTC (bit test and complement)
 * @param code_buf Code buffer for emission
 * @param insn Decoded x86 instruction
 * @param arm_rd Destination ARM register
 * @param arm_rm Source ARM register
 */
void translate_bit_btc(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_BIT_H */
