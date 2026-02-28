/* ============================================================================
 * Rosetta Translator - ALU Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 ALU instructions to x86_64.
 * Supported instructions: ADD, SUB, AND, ORR, EOR, MVN, MUL
 * ============================================================================ */

#include "rosetta_translate_alu_main.h"
#include "rosetta_translate_compare.h"
#include "rosetta_refactored.h"
#include <string.h>

/* Global register mapping (shared with rosetta_refactored.c) */
static const uint8_t g_arm64_to_x86_map[32] = {
    0, 1, 2, 3, 4, 5, 6, 7,     /* X0-X7 -> RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI */
    8, 9, 10, 11, 12, 13, 14, 15, /* X8-X15 -> R8-R15 */
    0, 1, 2, 3, 4, 5, 6, 7,     /* X16-X23 -> reuse RAX-RDI (caller saved) */
    8, 9, 10, 11, 12, 13, 14, 15  /* X24-X31 -> reuse R8-R15 */
};

/**
 * translate_get_x86_reg - Map ARM64 register to x86_64 register
 * @arm_reg: ARM64 register number (0-30)
 * Returns: x86_64 register number
 */
uint8_t translate_get_x86_reg(uint8_t arm_reg)
{
    return g_arm64_to_x86_map[arm_reg & 0x1F];
}

/**
 * translate_alu_add - Translate ADD instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_add(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] + guest_state[rm];

    /* Emit x86_64 code: first move rn to rd, then add rm */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_add_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_sub - Translate SUB instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_sub(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] - guest_state[rm];

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_sub_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_adc - Translate ADC (add with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * ADC Rd, Rn, Rm  ; Rd = Rn + Rm + C
 */
int translate_alu_adc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* ARM64 carry flag: C flag is inverted (1 = carry occurred, 0 = borrow) */
    /* Extract carry from PSTATE (bit 29) */
    uint64_t carry = (guest_state[32] >> 29) & 1;

    /* Update guest state: Rd = Rn + Rm + C */
    guest_state[rd] = guest_state[rn] + guest_state[rm] + carry;

    /* Emit x86_64 code:
     * MOV rd, rn
     * ADD rd, rm    ; This sets CF
     * If carry was set, we need to add 1 more
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_add_reg_reg(code_buf, x86_rd, x86_rm);

    /* If carry was set in ARM64, add 1 */
    if (carry) {
        emit_x86_add_reg_imm32(code_buf, x86_rd, 1);
    }

    return 0;
}

/**
 * translate_alu_sbc - Translate SBC (subtract with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * SBC Rd, Rn, Rm  ; Rd = Rn - Rm - NOT(C)
 */
int translate_alu_sbc(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* ARM64 carry flag is inverted for subtraction */
    /* C=1 means no borrow, C=0 means borrow */
    uint64_t not_carry = ~((guest_state[32] >> 29) & 1) & 1;

    /* Update guest state: Rd = Rn - Rm - NOT(C) */
    guest_state[rd] = guest_state[rn] - guest_state[rm] - not_carry;

    /* Emit x86_64 code:
     * MOV rd, rn
     * SUB rd, rm
     * If borrow needed, subtract 1 more
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_sub_reg_reg(code_buf, x86_rd, x86_rm);

    /* If borrow was needed (NOT(C) = 1), subtract 1 */
    if (not_carry) {
        emit_x86_sub_reg_imm32(code_buf, x86_rd, 1);
    }

    return 0;
}

/**
 * translate_alu_and - Translate AND instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_and(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] & guest_state[rm];

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_and_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_orr - Translate ORR instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_orr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] | guest_state[rm];

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_or_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_eor - Translate EOR (XOR) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_eor(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] ^ guest_state[rm];

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_xor_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_bic - Translate BIC (bit clear) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * BIC Rd, Rn, Rm  ; Rd = Rn AND NOT(Rm)
 */
int translate_alu_bic(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state: Rd = Rn AND NOT(Rm) */
    guest_state[rd] = guest_state[rn] & ~guest_state[rm];

    /* Emit x86_64 code:
     * MOV rd, rn
     * NOT rm (temporary)
     * AND rd, rm
     * NOT rm (restore)
     *
     * Better approach: use ANDN if available, or:
     * MOV rd, rn
     * MOV temp, rm
     * NOT temp
     * AND rd, temp
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* Use a temporary register (RCX) to hold the inverted value */
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rm);
    emit_x86_not_reg(code_buf, EMIT_RCX);
    emit_x86_and_reg_reg(code_buf, x86_rd, EMIT_RCX);

    return 0;
}

/**
 * translate_alu_mvn - Translate MVN (bitwise NOT) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_mvn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = ~guest_state[rm];

    /* Emit x86_64 code: move rm to rd, then NOT */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);
    emit_x86_not_reg(code_buf, x86_rd);

    return 0;
}

/**
 * translate_alu_mul - Translate MUL instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_mul(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    guest_state[rd] = guest_state[rn] * guest_state[rm];

    /* Emit x86_64 code: IMUL rd, rn, rm */
    emit_x86_imul_reg_reg_reg(code_buf, x86_rd, x86_rn, x86_rm);

    return 0;
}

/**
 * translate_alu_udiv - Translate UDIV (unsigned divide) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_udiv(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    if (guest_state[rm] == 0) {
        guest_state[rd] = 0;  /* ARM64: division by zero returns 0 */
    } else {
        guest_state[rd] = guest_state[rn] / guest_state[rm];
    }

    /* Emit x86_64 code for unsigned division */
    /* For DIV, we need to:
     * 1. Clear RDX (high part of dividend)
     * 2. Move dividend to RAX
     * 3. DIV by divisor
     * 4. Move result from RAX to destination
     */

    /* XOR rdx, rdx (clear high part) */
    emit_x86_xor_reg_reg(code_buf, EMIT_RDX, EMIT_RDX);

    /* MOV rax, rn */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rn);

    /* DIV rm */
    emit_x86_div_reg(code_buf, x86_rm);

    /* MOV rd, rax */
    emit_x86_mov_reg_reg(code_buf, x86_rd, EMIT_RAX);

    return 0;
}

/**
 * translate_alu_sdiv - Translate SDIV (signed divide) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_sdiv(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state */
    if (guest_state[rm] == 0) {
        guest_state[rd] = 0;  /* ARM64: division by zero returns 0 */
    } else {
        guest_state[rd] = (int64_t)guest_state[rn] / (int64_t)guest_state[rm];
    }

    /* Emit x86_64 code for signed division */
    /* For IDIV, we need to:
     * 1. Sign-extend RAX into RDX:RAX (CQO)
     * 2. Move dividend to RAX
     * 3. IDIV by divisor
     * 4. Move result from RAX to destination
     */

    /* MOV rax, rn */
    emit_x86_mov_reg_reg(code_buf, EMIT_RAX, x86_rn);

    /* CQO (sign-extend RAX into RDX:RAX) - 48 99 */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x99);

    /* IDIV rm */
    emit_x86_idiv_reg(code_buf, x86_rm);

    /* MOV rd, rax */
    emit_x86_mov_reg_reg(code_buf, x86_rd, EMIT_RAX);

    return 0;
}

/**
 * translate_alu_lsl - Translate LSL (logical shift left) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_lsl(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);
    uint64_t shift_amount;

    /* For register-controlled shift, use low 6 bits of rm */
    shift_amount = guest_state[rm] & 0x3F;
    guest_state[rd] = guest_state[rn] << shift_amount;

    /* Emit x86_64 code:
     * MOV rd, rn
     * MOV cl, rm (low 8 bits for shift count)
     * SHL rd, cl
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* MOV cl, rm (move shift amount to CL) */
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rm);

    /* SHL rd, cl */
    emit_x86_shl_reg_cl(code_buf, x86_rd);

    return 0;
}

/**
 * translate_alu_lsr - Translate LSR (logical shift right) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_lsr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* For register-controlled shift, use low 6 bits of rm */
    uint64_t shift_amount = guest_state[rm] & 0x3F;
    guest_state[rd] = guest_state[rn] >> shift_amount;

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rm);
    emit_x86_shr_reg_cl(code_buf, x86_rd);

    return 0;
}

/**
 * translate_alu_asr - Translate ASR (arithmetic shift right) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_asr(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* For register-controlled shift, use low 6 bits of rm */
    uint64_t shift_amount = guest_state[rm] & 0x3F;
    guest_state[rd] = (uint64_t)((int64_t)guest_state[rn] >> shift_amount);

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rm);
    emit_x86_sar_reg_cl(code_buf, x86_rd);

    return 0;
}

/**
 * translate_alu_ror - Translate ROR (rotate right) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 */
int translate_alu_ror(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* For register-controlled rotate, use low 6 bits of rm */
    uint64_t rotate_amount = guest_state[rm] & 0x3F;
    guest_state[rd] = (guest_state[rn] >> rotate_amount) |
                      (guest_state[rn] << (64 - rotate_amount));

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rm);
    emit_x86_ror_reg_cl(code_buf, x86_rd);

    return 0;
}

/**
 * translate_update_flags_nzc - Update N, Z, C flags based on result
 * @pstate: Pointer to PSTATE
 * @result: Operation result
 * @carry: Carry flag value (for ADDS) or 0 for ANDS
 */
void translate_update_flags_nzc(uint64_t *pstate, uint64_t result, int carry)
{
    /* N flag: result is negative (MSB set) */
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);

    /* Z flag: result is zero */
    translate_set_flag_z(pstate, result == 0);

    /* C flag: as provided */
    translate_set_flag_c(pstate, carry);

    /* V flag: cleared for ANDS, set based on overflow for ADDS/SUBS */
    translate_set_flag_v(pstate, 0);
}

/**
 * translate_alu_adds - Translate ADDS (add with flags) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_alu_adds(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t op1 = guest_state[rn];
    uint64_t op2 = guest_state[rm];
    uint64_t result = op1 + op2;

    /* Update guest state */
    guest_state[rd] = result;

    /* Update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);
    translate_set_flag_z(pstate, result == 0);
    translate_set_flag_c(pstate, result < op1);  /* Carry if unsigned overflow */
    /* V flag: signed overflow - both operands same sign, result different */
    {
        int op1_sign = (op1 >> 63) & 1;
        int op2_sign = (op2 >> 63) & 1;
        int result_sign = (result >> 63) & 1;
        int overflow = (op1_sign == op2_sign) && (result_sign != op1_sign);
        translate_set_flag_v(pstate, overflow);
    }

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_add_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_subs - Translate SUBS (subtract with flags) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_alu_subs(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t op1 = guest_state[rn];
    uint64_t op2 = guest_state[rm];
    uint64_t result = op1 - op2;

    /* Update guest state */
    guest_state[rd] = result;

    /* Update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);
    translate_set_flag_z(pstate, result == 0);
    translate_set_flag_c(pstate, op1 >= op2);  /* Carry = no borrow for subtraction */
    /* V flag: signed overflow */
    {
        int op1_sign = (op1 >> 63) & 1;
        int op2_sign = (op2 >> 63) & 1;
        int result_sign = (result >> 63) & 1;
        int overflow = (op1_sign != op2_sign) && (result_sign != op1_sign);
        translate_set_flag_v(pstate, overflow);
    }

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_sub_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_ands - Translate ANDS (AND with flags) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_alu_ands(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t result = guest_state[rn] & guest_state[rm];

    /* Update guest state */
    guest_state[rd] = result;

    /* Update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);
    translate_set_flag_z(pstate, result == 0);
    /* C flag: may be affected by shifter operand, cleared here for simplicity */
    translate_set_flag_c(pstate, 0);
    /* V flag: unchanged by ANDS */
    translate_set_flag_v(pstate, 0);

    /* Emit x86_64 code */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_and_reg_reg(code_buf, x86_rd, x86_rm);

    return 0;
}

/**
 * translate_alu_madd - Translate MADD (multiply-add) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * MADD Rd, Rn, Rm, Ra  ; Rd = Rn * Rm + Ra
 */
int translate_alu_madd(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t ra = (encoding >> 10) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);
    uint8_t x86_ra = translate_get_x86_reg(ra);

    /* Update guest state: Rd = Rn * Rm + Ra */
    guest_state[rd] = guest_state[rn] * guest_state[rm] + guest_state[ra];

    /* Emit x86_64 code:
     * MOV rd, rn
     * IMUL rd, rm  (rd = rn * rm)
     * ADD rd, ra   (rd = rd + ra)
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);
    emit_x86_imul_reg_reg_reg(code_buf, x86_rd, x86_rd, x86_rm);
    emit_x86_add_reg_reg(code_buf, x86_rd, x86_ra);

    return 0;
}

/**
 * translate_alu_msub - Translate MSUB (multiply-subtract) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * Returns: 0 on success
 *
 * MSUB Rd, Rn, Rm, Ra  ; Rd = Ra - (Rn * Rm)
 */
int translate_alu_msub(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t ra = (encoding >> 10) & 0x1F;
    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);
    uint8_t x86_ra = translate_get_x86_reg(ra);

    /* Update guest state: Rd = Ra - (Rn * Rm) */
    guest_state[rd] = guest_state[ra] - (guest_state[rn] * guest_state[rm]);

    /* Emit x86_64 code:
     * MOV rd, ra
     * MOV temp, rn
     * IMUL temp, rm  (temp = rn * rm)
     * SUB rd, temp   (rd = ra - temp)
     */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_ra);
    emit_x86_mov_reg_reg(code_buf, EMIT_RCX, x86_rn);
    emit_x86_imul_reg_reg_reg(code_buf, EMIT_RCX, EMIT_RCX, x86_rm);
    emit_x86_sub_reg_reg(code_buf, x86_rd, EMIT_RCX);

    return 0;
}

/**
 * translate_alu_dispatch - Dispatch ALU instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_alu_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    /* ALU with flags update (check first as they have unique encodings) */
    if ((encoding & ALU_ADDS_MASK) == ALU_ADDS_VAL) {
        return translate_alu_adds(encoding, code_buf, guest_state, pstate);
    } else if ((encoding & ALU_SUBS_MASK) == ALU_SUBS_VAL) {
        return translate_alu_subs(encoding, code_buf, guest_state, pstate);
    } else if ((encoding & ALU_ANDS_MASK) == ALU_ANDS_VAL) {
        return translate_alu_ands(encoding, code_buf, guest_state, pstate);
    }
    /* Regular ALU instructions */
    else if ((encoding & ALU_ADD_MASK) == ALU_ADD_VAL) {
        return translate_alu_add(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_SUB_MASK) == ALU_SUB_VAL) {
        return translate_alu_sub(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_ADC_MASK) == ALU_ADC_VAL) {
        return translate_alu_adc(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_SBC_MASK) == ALU_SBC_VAL) {
        return translate_alu_sbc(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_AND_MASK) == ALU_AND_VAL) {
        return translate_alu_and(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_ORR_MASK) == ALU_ORR_VAL) {
        return translate_alu_orr(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_EOR_MASK) == ALU_EOR_VAL) {
        return translate_alu_eor(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_BIC_MASK) == ALU_BIC_VAL) {
        return translate_alu_bic(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_MVN_MASK) == ALU_MVN_VAL) {
        return translate_alu_mvn(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_MUL_MASK) == ALU_MUL_VAL) {
        return translate_alu_mul(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_MADD_MASK) == ALU_MADD_VAL) {
        return translate_alu_madd(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_MSUB_MASK) == ALU_MSUB_VAL) {
        return translate_alu_msub(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_UDIV_MASK) == ALU_UDIV_VAL) {
        return translate_alu_udiv(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_SDIV_MASK) == ALU_SDIV_VAL) {
        return translate_alu_sdiv(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_LSL_MASK) == ALU_LSL_VAL) {
        return translate_alu_lsl(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_LSR_MASK) == ALU_LSR_VAL) {
        return translate_alu_lsr(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_ASR_MASK) == ALU_ASR_VAL) {
        return translate_alu_asr(encoding, code_buf, guest_state);
    } else if ((encoding & ALU_ROR_MASK) == ALU_ROR_VAL) {
        return translate_alu_ror(encoding, code_buf, guest_state);
    }

    return -1;  /* Not an ALU instruction */
}
