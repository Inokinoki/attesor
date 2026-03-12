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

void translate_alu_add(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* Check if this is an 8-bit operation */
    int is_8bit = (insn->opcode >= 0x00 && insn->opcode <= 0x05);

    if (insn->has_modrm && insn->mod == 3) {
        /* Register ADD */
        if (is_8bit) {
            /* 8-bit ADD: use 32-bit operations + mask */
            emit_add_reg(code_buf, arm_rd, arm_rd, arm_rm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            /* 32/64-bit ADD */
            emit_add_reg(code_buf, arm_rd, arm_rd, arm_rm);
        }
    } else if (insn->imm_size > 0) {
        /* Immediate ADD - handle different sizes */
        u64 imm = (u64)insn->imm;
        if (is_8bit) {
            /* 8-bit immediate ADD */
            emit_add_imm(code_buf, arm_rd, arm_rd, (u16)imm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else if (imm <= 0xFFFF) {
            emit_add_imm(code_buf, arm_rd, arm_rd, (u16)imm);
        } else {
            /* Large immediate: load into temp register then add */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (u16)imm, 0);
            if (imm > 0xFFFF) {
                emit_movk(code_buf, tmp, (u16)(imm >> 16), 1);
                if (imm > 0xFFFFFFFF) {
                    emit_movk(code_buf, tmp, (u16)(imm >> 32), 2);
                    if (imm > 0xFFFFFFFFFFFF) {
                        emit_movk(code_buf, tmp, (u16)(imm >> 48), 3);
                    }
                }
            }
            emit_add_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        /* Memory operand: already loaded in arm_rm */
        emit_add_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_sub(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* Check if this is an 8-bit operation */
    int is_8bit = (insn->opcode >= 0x28 && insn->opcode <= 0x2D);

    if (insn->has_modrm && insn->mod == 3) {
        /* Register SUB */
        if (is_8bit) {
            /* 8-bit SUB: use 32-bit operations + mask */
            emit_sub_reg(code_buf, arm_rd, arm_rd, arm_rm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            /* 32/64-bit SUB */
            emit_sub_reg(code_buf, arm_rd, arm_rd, arm_rm);
        }
    } else if (insn->imm_size > 0) {
        /* Immediate SUB */
        u64 imm = (u64)insn->imm;
        if (is_8bit) {
            /* 8-bit immediate SUB */
            emit_sub_imm(code_buf, arm_rd, arm_rd, (u16)imm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else if (imm <= 0xFFFF) {
            emit_sub_imm(code_buf, arm_rd, arm_rd, (u16)imm);
        } else {
            /* Large immediate: load into temp register then subtract */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (u16)imm, 0);
            if (imm > 0xFFFF) {
                emit_movk(code_buf, tmp, (u16)(imm >> 16), 1);
                if (imm > 0xFFFFFFFF) {
                    emit_movk(code_buf, tmp, (u16)(imm >> 32), 2);
                    if (imm > 0xFFFFFFFFFFFF) {
                        emit_movk(code_buf, tmp, (u16)(imm >> 48), 3);
                    }
                }
            }
            emit_sub_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        /* Memory operand */
        emit_sub_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_and(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* Check if this is an 8-bit operation */
    int is_8bit = (insn->opcode >= 0x20 && insn->opcode <= 0x25);

    if (insn->has_modrm && insn->mod == 3) {
        /* Register AND */
        if (is_8bit) {
            /* 8-bit AND: use 32-bit operations + mask */
            emit_and_reg(code_buf, arm_rd, arm_rd, arm_rm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            /* 32/64-bit AND */
            emit_and_reg(code_buf, arm_rd, arm_rd, arm_rm);
        }
    } else if (insn->imm_size > 0) {
        /* Immediate AND */
        u64 imm = (u64)insn->imm;
        if (is_8bit) {
            /* 8-bit immediate AND - mask to 8 bits */
            emit_and_imm(code_buf, arm_rd, arm_rd, (u16)imm & 0xFF);
        } else if (imm <= 0xFFFF) {
            emit_and_imm(code_buf, arm_rd, arm_rd, (u16)imm);
        } else {
            /* Large immediate: load into temp register then AND */
            uint8_t tmp = 16;
            emit_movz(code_buf, tmp, (u16)imm, 0);
            if (imm > 0xFFFF) {
                emit_movk(code_buf, tmp, (u16)(imm >> 16), 1);
                if (imm > 0xFFFFFFFF) {
                    emit_movk(code_buf, tmp, (u16)(imm >> 32), 2);
                    if (imm > 0xFFFFFFFFFFFF) {
                        emit_movk(code_buf, tmp, (u16)(imm >> 48), 3);
                    }
                }
            }
            emit_and_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        emit_and_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_or(code_buffer_t *code_buf, const x86_insn_t *insn,
                      uint8_t arm_rd, uint8_t arm_rm)
{
    /* Check if this is an 8-bit operation */
    int is_8bit = (insn->opcode >= 0x08 && insn->opcode <= 0x0D);

    if (insn->has_modrm && insn->mod == 3) {
        /* Register OR */
        if (is_8bit) {
            /* 8-bit OR: use 32-bit operations + mask */
            emit_orr_reg(code_buf, arm_rd, arm_rd, arm_rm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            /* 32/64-bit OR */
            emit_orr_reg(code_buf, arm_rd, arm_rd, arm_rm);
        }
    } else if (insn->imm_size > 0) {
        /* Immediate OR - load into temp then OR */
        u64 imm = (u64)insn->imm;
        uint8_t tmp = 16;
        emit_movz(code_buf, tmp, (u16)imm, 0);
        if (is_8bit) {
            /* 8-bit immediate OR - mask result to 8 bits */
            emit_orr_reg(code_buf, arm_rd, arm_rd, tmp);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            if (imm > 0xFFFF) {
                emit_movk(code_buf, tmp, (u16)(imm >> 16), 1);
                if (imm > 0xFFFFFFFF) {
                    emit_movk(code_buf, tmp, (u16)(imm >> 32), 2);
                    if (imm > 0xFFFFFFFFFFFF) {
                        emit_movk(code_buf, tmp, (u16)(imm >> 48), 3);
                    }
                }
            }
            emit_orr_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        emit_orr_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_xor(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    /* Check if this is an 8-bit operation */
    int is_8bit = (insn->opcode >= 0x30 && insn->opcode <= 0x35);

    /* Special case: XOR reg, reg -> clear register */
    if (insn->has_modrm && insn->mod == 3 && insn->reg == insn->rm) {
        if (is_8bit) {
            /* 8-bit XOR reg, reg: clear and mask to 8 bits */
            emit_movz(code_buf, arm_rd, 0, 0);
        } else {
            emit_movz(code_buf, arm_rd, 0, 0);
        }
        return;
    }

    if (insn->has_modrm && insn->mod == 3) {
        /* Register XOR */
        if (is_8bit) {
            /* 8-bit XOR: use 32-bit operations + mask */
            emit_eor_reg(code_buf, arm_rd, arm_rd, arm_rm);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            /* 32/64-bit XOR */
            emit_eor_reg(code_buf, arm_rd, arm_rd, arm_rm);
        }
    } else if (insn->imm_size > 0) {
        /* Immediate XOR - load into temp then XOR */
        u64 imm = (u64)insn->imm;
        uint8_t tmp = 16;
        emit_movz(code_buf, tmp, (u16)imm, 0);
        if (is_8bit) {
            /* 8-bit immediate XOR - mask result to 8 bits */
            emit_eor_reg(code_buf, arm_rd, arm_rd, tmp);
            emit_and_imm(code_buf, arm_rd, arm_rd, 0xFF);
        } else {
            if (imm > 0xFFFF) {
                emit_movk(code_buf, tmp, (u16)(imm >> 16), 1);
                if (imm > 0xFFFFFFFF) {
                    emit_movk(code_buf, tmp, (u16)(imm >> 32), 2);
                    if (imm > 0xFFFFFFFFFFFF) {
                        emit_movk(code_buf, tmp, (u16)(imm >> 48), 3);
                    }
                }
            }
            emit_eor_reg(code_buf, arm_rd, arm_rd, tmp);
        }
    } else {
        emit_eor_reg(code_buf, arm_rd, arm_rd, arm_rm);
    }
}

void translate_alu_mul(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm)
{
    (void)insn;
    /* MUL: RAX = RAX * operand */
    emit_mul_reg(code_buf, X0, X0, arm_rm);
}

void translate_alu_div(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rm)
{
    (void)insn;
    (void)arm_rm;
    /* DIV/IDIV: RAX = RDX:RAX / operand */
    /* Requires RDX:RAX setup before and emits SDIV */
    emit_sdiv_reg(code_buf, X0, X0, arm_rm);
}

void translate_alu_inc(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd)
{
    (void)insn;
    /* INC: dst = dst + 1 */
    emit_inc_reg(code_buf, arm_rd);
}

void translate_alu_dec(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd)
{
    (void)insn;
    /* DEC: dst = dst - 1 */
    emit_dec_reg(code_buf, arm_rd);
}

void translate_alu_neg(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* NEG: dst = 0 - src (two's complement negation) */
    emit_neg_reg(code_buf, arm_rd, arm_rm);
}

void translate_alu_not(code_buffer_t *code_buf, const x86_insn_t *insn,
                       uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* NOT: dst = ~src (bitwise inversion) */
    emit_not_reg(code_buf, arm_rd, arm_rm);
}

void translate_alu_shift(code_buffer_t *code_buf, const x86_insn_t *insn,
                         uint8_t arm_rd, uint8_t arm_rm)
{
    (void)arm_rm;
    u8 shift = (insn->imm_size > 0) ? (u8)insn->imm : 1;

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
        /* SHR: logical shift right (zero-extended) */
        emit_shr_reg_imm(code_buf, arm_rd, arm_rd, shift);
    } else if (x86_is_sar(insn)) {
        /* SAR: arithmetic shift right (sign-extended) */
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
