/* ============================================================================
 * Rosetta Translator - Compare Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 compare instructions to x86_64.
 * Supported instructions: CMP, CMN, TST
 * ============================================================================ */

#include "rosetta_translate_compare.h"

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/**
 * translate_set_flag_n - Set/clear Negative flag
 * @pstate: Pointer to PSTATE
 * @set: 1 to set, 0 to clear
 */
void translate_set_flag_n(uint64_t *pstate, int set)
{
    if (set) {
        *pstate |= FLAG_N_MASK;
    } else {
        *pstate &= ~FLAG_N_MASK;
    }
}

/**
 * translate_set_flag_z - Set/clear Zero flag
 * @pstate: Pointer to PSTATE
 * @set: 1 to set, 0 to clear
 */
void translate_set_flag_z(uint64_t *pstate, int set)
{
    if (set) {
        *pstate |= FLAG_Z_MASK;
    } else {
        *pstate &= ~FLAG_Z_MASK;
    }
}

/**
 * translate_set_flag_c - Set/clear Carry flag
 * @pstate: Pointer to PSTATE
 * @set: 1 to set, 0 to clear
 */
void translate_set_flag_c(uint64_t *pstate, int set)
{
    if (set) {
        *pstate |= FLAG_C_MASK;
    } else {
        *pstate &= ~FLAG_C_MASK;
    }
}

/**
 * translate_set_flag_v - Set/clear Overflow flag
 * @pstate: Pointer to PSTATE
 * @set: 1 to set, 0 to clear
 */
void translate_set_flag_v(uint64_t *pstate, int set)
{
    if (set) {
        *pstate |= FLAG_V_MASK;
    } else {
        *pstate &= ~FLAG_V_MASK;
    }
}

/**
 * translate_compare_cmp - Translate CMP (compare) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_compare_cmp(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t op1 = guest_state[rn];
    uint64_t op2 = guest_state[rm];
    uint64_t result = op1 - op2;

    /* Clear and update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;

    /* N flag: result is negative (MSB set) */
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);

    /* Z flag: result is zero */
    translate_set_flag_z(pstate, result == 0);

    /* C flag: unsigned borrow (op1 >= op2) */
    translate_set_flag_c(pstate, op1 >= op2);

    /* V flag: signed overflow */
    /* Overflow occurs when subtracting numbers with different signs */
    /* and the result has a different sign than op1 */
    {
        int op1_sign = (op1 >> 63) & 1;
        int op2_sign = (op2 >> 63) & 1;
        int result_sign = (result >> 63) & 1;
        int overflow = (op1_sign != op2_sign) && (result_sign != op1_sign);
        translate_set_flag_v(pstate, overflow);
    }

    /* Emit x86_64 code */
    emit_x86_cmp_reg_reg(code_buf, x86_rn, x86_rm);

    return 0;
}

/**
 * translate_compare_cmn - Translate CMN (compare negative) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_compare_cmn(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t op1 = guest_state[rn];
    uint64_t op2 = guest_state[rm];
    uint64_t result = op1 + op2;

    /* Clear and update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;

    /* N flag: result is negative (MSB set) */
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);

    /* Z flag: result is zero */
    translate_set_flag_z(pstate, result == 0);

    /* C flag: unsigned carry (result < op1) */
    translate_set_flag_c(pstate, result < op1);

    /* V flag: signed overflow */
    {
        int op1_sign = (op1 >> 63) & 1;
        int op2_sign = (op2 >> 63) & 1;
        int result_sign = (result >> 63) & 1;
        int overflow = (op1_sign == op2_sign) && (result_sign != op1_sign);
        translate_set_flag_v(pstate, overflow);
    }

    /* Emit x86_64 code - use ADD and discard result */
    emit_x86_mov_reg_reg(code_buf, x86_rn, x86_rn);  /* Dummy move to preserve register */
    emit_x86_add_reg_reg(code_buf, x86_rn, x86_rm);

    return 0;
}

/**
 * translate_compare_tst - Translate TST (test bits) instruction
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 on success
 */
int translate_compare_tst(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    uint64_t result = guest_state[rn] & guest_state[rm];

    /* Clear and update NZCV flags */
    *pstate &= ~FLAG_NZCV_MASK;

    /* N flag: result is negative (MSB set) */
    translate_set_flag_n(pstate, (result & (1ULL << 63)) != 0);

    /* Z flag: result is zero */
    translate_set_flag_z(pstate, result == 0);

    /* C and V flags are unaffected by TST */

    /* Emit x86_64 code */
    emit_x86_test_reg_reg(code_buf, x86_rn, x86_rm);

    return 0;
}

/**
 * translate_compare_dispatch - Dispatch compare instruction based on encoding
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @guest_state: Guest register state (x[32])
 * @pstate: Pointer to PSTATE flags
 * Returns: 0 if instruction handled, -1 otherwise
 */
int translate_compare_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate)
{
    if ((encoding & CMP_MASK) == CMP_VAL) {
        return translate_compare_cmp(encoding, code_buf, guest_state, pstate);
    } else if ((encoding & CMN_MASK) == CMN_VAL) {
        return translate_compare_cmn(encoding, code_buf, guest_state, pstate);
    } else if ((encoding & TST_MASK) == TST_VAL) {
        return translate_compare_tst(encoding, code_buf, guest_state, pstate);
    }

    return -1;  /* Not a compare instruction */
}
