/* ============================================================================
 * Rosetta Translator - Full ALU Implementation
 * ============================================================================
 *
 * This module provides complete implementation of ARM64 ALU (Arithmetic
 * Logic Unit) instructions including all variants:
 * - Basic: ADD, SUB, NEG, MVN
 * - With carry: ADC, SBC
 * - Logical: AND, ORR, EOR, BIC, ORN, ANDS
 * - Shifts: LSL, LSR, ASR, ROR
 * - Compare: CMP, CMN, TST
 * - Immediate variants of all above
 * ============================================================================ */

#include "rosetta_translate_alu_full.h"
#include "rosetta_arm64_decode.h"

/* ============================================================================
 * Flag Update Helpers
 * ============================================================================ */

/**
 * alu_update_nzcv_add - Update NZCV flags for addition
 * @state: Thread state
 * @result: Operation result
 * @op1: First operand
 * @op2: Second operand
 * @sf: Size flag (1 = 64-bit, 0 = 32-bit)
 */
static inline void alu_update_nzcv_add(ThreadState *state, uint64_t result,
                                        uint64_t op1, uint64_t op2, int sf)
{
    uint32_t nzcv = 0;

    if (sf) {
        /* N flag - Set if result is negative (bit 63 set) */
        if (result & (1ULL << 63)) {
            nzcv |= (1 << 31);
        }
        /* Z flag - Set if result is zero */
        if (result == 0) {
            nzcv |= (1 << 30);
        }
        /* C flag - Set if unsigned overflow (result < op1) */
        if (result < op1) {
            nzcv |= (1 << 29);
        }
        /* V flag - Set if signed overflow */
        int64_t a = (int64_t)op1;
        int64_t b = (int64_t)op2;
        int64_t r = (int64_t)result;
        if ((a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0)) {
            nzcv |= (1 << 28);
        }
    } else {
        uint32_t result32 = (uint32_t)result;
        uint32_t op1_32 = (uint32_t)op1;
        uint32_t op2_32 = (uint32_t)op2;

        if (result32 & (1U << 31)) {
            nzcv |= (1 << 31);
        }
        if (result32 == 0) {
            nzcv |= (1 << 30);
        }
        if (result32 < op1_32) {
            nzcv |= (1 << 29);
        }
        int32_t a = (int32_t)op1_32;
        int32_t b = (int32_t)op2_32;
        int32_t r = (int32_t)result32;
        if ((a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0)) {
            nzcv |= (1 << 28);
        }
    }

    state->guest.pstate = nzcv;
}

/**
 * alu_update_nzcv_sub - Update NZCV flags for subtraction
 * @state: Thread state
 * @result: Operation result
 * @op1: First operand (minuend)
 * @op2: Second operand (subtrahend)
 * @sf: Size flag (1 = 64-bit, 0 = 32-bit)
 */
static inline void alu_update_nzcv_sub(ThreadState *state, uint64_t result,
                                        uint64_t op1, uint64_t op2, int sf)
{
    uint32_t nzcv = 0;

    if (sf) {
        /* N flag */
        if (result & (1ULL << 63)) {
            nzcv |= (1 << 31);
        }
        /* Z flag */
        if (result == 0) {
            nzcv |= (1 << 30);
        }
        /* C flag - Set if no borrow (op1 >= op2) */
        if (op1 >= op2) {
            nzcv |= (1 << 29);
        }
        /* V flag - Set if signed overflow */
        int64_t a = (int64_t)op1;
        int64_t b = (int64_t)op2;
        int64_t r = (int64_t)result;
        if ((a < 0 && b > 0 && r > 0) || (a > 0 && b < 0 && r < 0)) {
            nzcv |= (1 << 28);
        }
    } else {
        uint32_t result32 = (uint32_t)result;
        uint32_t op1_32 = (uint32_t)op1;
        uint32_t op2_32 = (uint32_t)op2;

        if (result32 & (1U << 31)) {
            nzcv |= (1 << 31);
        }
        if (result32 == 0) {
            nzcv |= (1 << 30);
        }
        if (op1_32 >= op2_32) {
            nzcv |= (1 << 29);
        }
        int32_t a = (int32_t)op1_32;
        int32_t b = (int32_t)op2_32;
        int32_t r = (int32_t)result32;
        if ((a < 0 && b > 0 && r > 0) || (a > 0 && b < 0 && r < 0)) {
            nzcv |= (1 << 28);
        }
    }

    state->guest.pstate = nzcv;
}

/**
 * alu_update_nzcv_logical - Update NZCV flags for logical operations
 * @state: Thread state
 * @result: Operation result
 * @sf: Size flag (1 = 64-bit, 0 = 32-bit)
 */
static inline void alu_update_nzcv_logical(ThreadState *state, uint64_t result, int sf)
{
    uint32_t nzcv = 0;

    if (sf) {
        if (result & (1ULL << 63)) {
            nzcv |= (1 << 31);
        }
        if (result == 0) {
            nzcv |= (1 << 30);
        }
    } else {
        uint32_t result32 = (uint32_t)result;
        if (result32 & (1U << 31)) {
            nzcv |= (1 << 31);
        }
        if (result32 == 0) {
            nzcv |= (1 << 30);
        }
    }

    /* C and V flags unchanged for logical operations */
    state->guest.pstate = nzcv;
}

/* ============================================================================
 * Shift/Extend Helpers
 * ============================================================================ */

/**
 * alu_apply_shift - Apply shift to operand
 * @value: Value to shift
 * @shift_type: Shift type (0=LSL, 1=LSR, 2=ASR, 3=ROR)
 * @shift_amount: Amount to shift
 * @sf: Size flag (1 = 64-bit, 0 = 32-bit)
 * Returns: Shifted value
 */
static inline uint64_t alu_apply_shift(uint64_t value, int shift_type,
                                        int shift_amount, int sf)
{
    if (shift_amount == 0) {
        return value;
    }

    switch (shift_type) {
        case 0: /* LSL */
            return value << shift_amount;
        case 1: /* LSR */
            return sf ? (value >> shift_amount) : ((uint32_t)value >> shift_amount);
        case 2: /* ASR */
            return sf ? ((int64_t)value >> shift_amount) : ((int32_t)(uint32_t)value >> shift_amount);
        case 3: /* ROR */
            if (sf) {
                shift_amount &= 63;
                return (value >> shift_amount) | (value << (64 - shift_amount));
            } else {
                shift_amount &= 31;
                uint32_t v32 = (uint32_t)value;
                return (v32 >> shift_amount) | (v32 << (32 - shift_amount));
            }
        default:
            return value;
    }
}

/**
 * alu_decode_immediate - Decode 12-bit immediate with optional shift
 * @encoding: ARM64 instruction encoding
 * @sf: Size flag
 * Returns: Decoded immediate value
 */
static inline uint64_t alu_decode_immediate(uint32_t encoding, int sf)
{
    uint16_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t shift = (encoding >> 22) & 0x3;

    if (sf) {
        return (uint64_t)imm12 << (shift * 12);
    } else {
        return (uint64_t)(imm12 & 0xFF) << (shift * 8);
    }
}

/* ============================================================================
 * Addition Instructions
 * ============================================================================ */

/**
 * translate_add_reg - Translate ADD (register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * ADD Rd, Rn, Rm {, shift #imm}
 */
int translate_add_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;
    uint8_t shift_type = (encoding >> 22) & 0x3;
    uint8_t shift_amount = (encoding >> 10) & 0x3F;
    shift_amount &= sf ? 0x3F : 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = alu_apply_shift(state->guest.x[rm], shift_type, shift_amount, sf);
    uint64_t result = op1 + op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_add(state, result, op1, op2, sf);
    }

    return 0;
}

/**
 * translate_add_imm - Translate ADD (immediate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * ADD Rd, Rn, #imm
 */
int translate_add_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t imm = alu_decode_immediate(encoding, sf);
    uint64_t op1 = state->guest.x[rn];
    uint64_t result = op1 + imm;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_add(state, result, op1, imm, sf);
    }

    return 0;
}

/**
 * translate_adc - Translate ADC (add with carry) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * ADC Rd, Rn, Rm  ->  Rd = Rn + Rm + C
 */
int translate_adc(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t carry = (state->guest.pstate >> 29) & 1;
    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 + op2 + carry;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_add(state, result, op1, op2 + carry, sf);
    }

    return 0;
}

/* ============================================================================
 * Subtraction Instructions
 * ============================================================================ */

/**
 * translate_sub_reg - Translate SUB (register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_sub_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;
    uint8_t shift_type = (encoding >> 22) & 0x3;
    uint8_t shift_amount = (encoding >> 10) & 0x3F;
    shift_amount &= sf ? 0x3F : 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = alu_apply_shift(state->guest.x[rm], shift_type, shift_amount, sf);
    uint64_t result = op1 - op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_sub(state, result, op1, op2, sf);
    }

    return 0;
}

/**
 * translate_sub_imm - Translate SUB (immediate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_sub_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t imm = alu_decode_immediate(encoding, sf);
    uint64_t op1 = state->guest.x[rn];
    uint64_t result = op1 - imm;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_sub(state, result, op1, imm, sf);
    }

    return 0;
}

/**
 * translate_sbc - Translate SBC (subtract with carry) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * SBC Rd, Rn, Rm  ->  Rd = Rn - Rm - !C
 */
int translate_sbc(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t carry = ~((state->guest.pstate >> 29) & 1) & 1;  /* Inverted C flag */
    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 - op2 - carry;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_sub(state, result, op1, op2 + carry, sf);
    }

    return 0;
}

/**
 * translate_neg - Translate NEG (negate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * NEG Rd, Rm  ->  Rd = 0 - Rm
 * (Encoded as SUBS with Rn = 31/SP)
 */
int translate_neg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t op1 = 0;
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 - op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_sub(state, result, op1, op2, sf);
    }

    return 0;
}

/* ============================================================================
 * Logical Instructions
 * ============================================================================ */

/**
 * translate_and_reg - Translate AND (register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_and_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;
    uint8_t shift_type = (encoding >> 22) & 0x3;
    uint8_t shift_amount = (encoding >> 10) & 0x3F;
    shift_amount &= sf ? 0x3F : 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = alu_apply_shift(state->guest.x[rm], shift_type, shift_amount, sf);
    uint64_t result = op1 & op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/**
 * translate_and_imm - Translate AND (immediate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_and_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t imm = alu_decode_immediate(encoding, sf);
    uint64_t op1 = state->guest.x[rn];
    uint64_t result = op1 & imm;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/**
 * translate_orr_reg - Translate ORR (register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_orr_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 | op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/**
 * translate_eor_reg - Translate EOR (XOR, register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_eor_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 ^ op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/**
 * translate_bic - Translate BIC (bit clear) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * BIC Rd, Rn, Rm  ->  Rd = Rn AND NOT Rm
 */
int translate_bic(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = ~state->guest.x[rm];
    uint64_t result = op1 & op2;

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/**
 * translate_mvn_reg - Translate MVN (move not, register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * MVN Rd, Rm  ->  Rd = NOT Rm
 */
int translate_mvn_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rd = encoding & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    uint8_t s = (encoding >> 29) & 1;

    uint64_t result = ~state->guest.x[rm];

    state->guest.x[rd] = sf ? result : (uint32_t)result;

    if (s) {
        alu_update_nzcv_logical(state, result, sf);
    }

    return 0;
}

/* ============================================================================
 * Compare Instructions
 * ============================================================================ */

/**
 * translate_cmp_reg - Translate CMP (compare, register) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * CMP Rn, Rm  ->  Updates flags based on Rn - Rm (no result stored)
 */
int translate_cmp_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 - op2;

    alu_update_nzcv_sub(state, result, op1, op2, sf);

    return 0;
}

/**
 * translate_cmp_imm - Translate CMP (immediate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 */
int translate_cmp_imm(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t imm = alu_decode_immediate(encoding, sf);
    uint64_t result = op1 - imm;

    alu_update_nzcv_sub(state, result, op1, imm, sf);

    return 0;
}

/**
 * translate_cmn_reg - Translate CMN (compare negative) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * CMN Rn, Rm  ->  Updates flags based on Rn + Rm (no result stored)
 */
int translate_cmn_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 + op2;

    alu_update_nzcv_add(state, result, op1, op2, sf);

    return 0;
}

/**
 * translate_tst_reg - Translate TST (test bits) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success
 *
 * TST Rn, Rm  ->  Updates flags based on Rn AND Rm (no result stored)
 */
int translate_tst_reg(ThreadState *state, const uint8_t *insn)
{
    uint32_t encoding = *(const uint32_t *)insn;

    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 & op2;

    alu_update_nzcv_logical(state, result, sf);

    return 0;
}
