/* ============================================================================
 * Rosetta ALU Translation Module
 * ============================================================================
 *
 * This module handles translation of x86_64 arithmetic and logic operations
 * to equivalent ARM64 instructions.
 * ============================================================================ */

#include "rosetta_translate_alu.h"
#include <stdint.h>

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

void translate_alu_add(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    if (insn->has_modrm && insn->mod == 3) {
        /* Register ADD: r64, r/m64 */
        emit_add_reg(code_buf, arm_rd, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Immediate ADD - handle different sizes */
        uint64_t imm = (uint64_t)insn->imm;
        if (imm <= 0xFFFF) {
            emit_add_imm(code_buf, arm_rd, arm_rd, (uint16_t)imm);
        } else {
            /* Large immediate: load into temp register then add */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
            if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
            if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
            if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
            emit_add_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        /* Memory operand: already loaded in arm_rm */
        emit_add_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_sub(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    if (insn->has_modrm && insn->mod == 3) {
        /* Register SUB: r64, r/m64 */
        emit_sub_reg(code_buf, arm_rd, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Immediate SUB */
        uint64_t imm = (uint64_t)insn->imm;
        if (imm <= 0xFFFF) {
            emit_sub_imm(code_buf, arm_rd, arm_rd, (uint16_t)imm);
        } else {
            /* Large immediate: load into temp register then subtract */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
            if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
            if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
            if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
            emit_sub_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        /* Memory operand */
        emit_sub_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_and(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    if (insn->has_modrm && insn->mod == 3) {
        /* Register AND: r64, r/m64 */
        emit_and_reg(code_buf, arm_rd, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Immediate AND */
        uint64_t imm = (uint64_t)insn->imm;
        if (imm <= 0xFFFF) {
            emit_and_imm(code_buf, arm_rd, arm_rd, (uint16_t)imm);
        } else {
            /* Large immediate: load into temp register then AND */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
            if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
            if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
            if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
            emit_and_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        emit_and_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_or(CodeBuffer *code_buf, const x86_insn_t *insn,
                      uint8_t arm_rd, uint8_t arm_rm)
{
    if (insn->has_modrm && insn->mod == 3) {
        /* Register OR: r64, r/m64 */
        emit_orr_reg(code_buf, arm_rd, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Immediate OR - load into temp then OR */
        uint64_t imm = (uint64_t)insn->imm;
        uint8_t tmp = 16;
        emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
        if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
        if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
        if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
        emit_orr_reg(code_buf, arm_rd, arm_rd, tmp);
    } else {
        emit_orr_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_xor(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* Special case: XOR EAX, EAX or RAX, RAX -> clear register */
    if (insn->has_modrm && insn->mod == 3 && insn->reg == insn->rm) {
        emit_mov_reg(code_buf, arm_rd, XZR);
        return;
    }

    if (insn->has_modrm && insn->mod == 3) {
        /* Register XOR: r64, r/m64 */
        emit_eor_reg(code_buf, arm_rd, arm_rd, arm_rm);
    } else if (insn->imm_size > 0) {
        /* Immediate XOR - load into temp then XOR */
        uint64_t imm = (uint64_t)insn->imm;
        uint8_t tmp = 16;
        emit_movz(code_buf, tmp, (uint16_t)(imm & 0xFFFF), 0);
        if (imm >> 16) emit_movk(code_buf, tmp, (uint16_t)((imm >> 16) & 0xFFFF), 1);
        if (imm >> 32) emit_movk(code_buf, tmp, (uint16_t)((imm >> 32) & 0xFFFF), 2);
        if (imm >> 48) emit_movk(code_buf, tmp, (uint16_t)((imm >> 48) & 0xFFFF), 3);
        emit_eor_reg(code_buf, arm_rd, arm_rd, tmp);
    } else {
        emit_eor_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_mul(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm)
{
    (void)insn;
    /* MUL: RAX = RAX * operand
     * Note: Full implementation should handle:
     * - Unsigned vs signed (IMUL)
     * - 128-bit result in RDX:RAX
     * - Overflow flag setting
     */
    emit_mul_reg(code_buf, X0, X0, arm_rm);
}

void translate_alu_div(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm)
{
    (void)insn;
    /* DIV/IDIV: RAX = RDX:RAX / operand
     * Note: Full implementation should handle:
     * - Unsigned vs signed division
     * - Remainder in RDX
     * - Division by zero check
     */
    emit_sdiv_reg(code_buf, X0, X0, arm_rm);
}

void translate_alu_inc(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd)
{
    (void)insn;
    /* INC: dst = dst + 1
     * Note: Does not affect carry flag
     */
    emit_inc_reg(code_buf, arm_rd);
}

void translate_alu_dec(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd)
{
    (void)insn;
    /* DEC: dst = dst - 1
     * Note: Does not affect carry flag
     */
    emit_dec_reg(code_buf, arm_rd);
}

void translate_alu_neg(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* NEG: dst = 0 - src (two's complement negation)
     * Sets flags based on result
     */
    emit_neg_reg(code_buf, arm_rd, arm_rm);
}

void translate_alu_not(CodeBuffer *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* NOT: dst = ~src (bitwise inversion)
     * Does not affect flags
     */
    emit_not_reg(code_buf, arm_rd, arm_rm);
}

void translate_alu_shift(CodeBuffer *code_buf, const x86_insn_t *insn,
                         uint8_t arm_rd, uint8_t arm_rm)
{
    (void)arm_rm;
    uint8_t shift = (insn->imm_size > 0) ? (uint8_t)insn->imm : 1;

    /* Handle CL-based shifts (shift by CX register) */
    if (insn->imm_size == 0 && insn->opcode == 0xD3) {
        /* Shift by CL - would need special handling */
        /* For now, use a fixed shift of 1 */
        shift = 1;
    }

    if (x86_is_shl(insn)) {
        /* SHL: logical shift left (same as SAL) */
        emit_shl_reg_imm(code_buf, arm_rd, arm_rd, shift);
    } else if (x86_is_shr(insn)) {
        /* SHR: logical shift right */
        emit_shr_reg_imm(code_buf, arm_rd, arm_rd, shift);
    } else if (x86_is_sar(insn)) {
        /* SAR: arithmetic shift right (sign-extending) */
        emit_sar_reg_imm(code_buf, arm_rd, arm_rd, shift);
    } else if (x86_is_rol(insn)) {
        /* ROL: rotate left */
        emit_rol_reg_imm(code_buf, arm_rd, arm_rd, shift);
    } else if (x86_is_ror(insn)) {
        /* ROR: rotate right */
        emit_ror_reg_imm(code_buf, arm_rd, arm_rd, shift);
    }
}

/* End of rosetta_translate_alu.c */
