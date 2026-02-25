/* ============================================================================
 * Rosetta Bit Manipulation Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 bit manipulation instructions
 * including BSF, BSR, POPCNT, and BT/BTS/BTR/BTC operations.
 * ============================================================================ */

#include "rosetta_translate_bit.h"
#include <stdint.h>

/* ============================================================================
 * Bit Manipulation Translation Functions
 * ============================================================================ */

void translate_bit_bsf(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* BSF: bit scan forward */
    emit_bsf_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_bsr(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* BSR: bit scan reverse */
    emit_bsr_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_popcnt(code_buffer_t *code_buf, const x86_insn_t *insn,
                          uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* POPCNT: population count */
    emit_popcnt_reg(code_buf, arm_rd, arm_rm);
}

void translate_bit_bt(code_buffer_t *code_buf, const x86_insn_t *insn,
                      uint8_t arm_rd, uint8_t arm_rm)
{
    /* BT: bit test - test bit and return value */
    uint8_t bit = (insn->imm_size > 0) ? (uint8_t)insn->imm : 0;
    emit_bt_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_bts(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* BTS: bit test and set */
    uint8_t bit = (insn->imm_size > 0) ? (uint8_t)insn->imm : 0;
    emit_bts_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_btr(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* BTR: bit test and reset */
    uint8_t bit = (insn->imm_size > 0) ? (uint8_t)insn->imm : 0;
    emit_btr_reg(code_buf, arm_rd, arm_rm, bit);
}

void translate_bit_btc(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* BTC: bit test and complement */
    uint8_t bit = (insn->imm_size > 0) ? (uint8_t)insn->imm : 0;
    emit_btc_reg(code_buf, arm_rd, arm_rm, bit);
}

/* End of rosetta_translate_bit.c */
