/* ============================================================================
 * Rosetta Translator - MOV Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 MOV family instructions to x86_64.
 * Supported instructions: MOVZ, MOVK, MOVN, MOV (register)
 * ============================================================================ */

#include "rosetta_translate_mov.h"

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/**
 * translate_mov_movz - Translate MOVZ (move wide with zero) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mov_movz(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint16_t imm16 = (encoding >> 5) & 0xFFFF;
    uint8_t shift = ((encoding >> 21) & 0x03) * 16;
    uint8_t x86_rd = translate_get_x86_reg(rd);

    /* Update guest state */
    guest_state[rd] = (uint64_t)imm16 << shift;

    /* Emit x86_64 code */
    emit_x86_mov_reg_imm64(code_buf, x86_rd, guest_state[rd]);

    return 0;
}

/**
 * translate_mov_movk - Translate MOVK (move wide with keep) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mov_movk(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint16_t imm16 = (encoding >> 5) & 0xFFFF;
    uint8_t shift = ((encoding >> 21) & 0x03) * 16;
    uint8_t x86_rd = translate_get_x86_reg(rd);

    /* Update guest state - clear bits at shift position and insert new value */
    guest_state[rd] = (guest_state[rd] & ~(0xFFFFULL << shift)) | ((uint64_t)imm16 << shift);

    /* Emit x86_64 code */
    emit_x86_mov_reg_imm64(code_buf, x86_rd, guest_state[rd]);

    return 0;
}

/**
 * translate_mov_movn - Translate MOVN (move wide with NOT) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mov_movn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint16_t imm16 = (encoding >> 5) & 0xFFFF;
    uint8_t shift = ((encoding >> 21) & 0x03) * 16;
    uint8_t x86_rd = translate_get_x86_reg(rd);

    /* Update guest state */
    guest_state[rd] = ~((uint64_t)imm16 << shift);

    /* Emit x86_64 code */
    emit_x86_mov_reg_imm64(code_buf, x86_rd, guest_state[rd]);

    return 0;
}

/**
 * translate_mov_reg - Translate MOV (register alias for ORR) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_mov_reg(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Update guest state */
    guest_state[rd] = guest_state[rn];

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    return 0;
}

/**
 * translate_mov_dispatch - Dispatch MOV instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_mov_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    if ((encoding & MOV_MOVZ_MASK) == MOV_MOVZ_VAL) {
        return translate_mov_movz(encoding, code_buf, guest_state);
    } else if ((encoding & MOV_MOVK_MASK) == MOV_MOVK_VAL) {
        return translate_mov_movk(encoding, code_buf, guest_state);
    } else if ((encoding & MOV_MOVN_MASK) == MOV_MOVN_VAL) {
        return translate_mov_movn(encoding, code_buf, guest_state);
    } else if ((encoding & MOV_MOV_REG_MASK) == MOV_MOV_REG_VAL) {
        return translate_mov_reg(encoding, code_buf, guest_state);
    }

    return -1;  /* Not a MOV instruction */
}
