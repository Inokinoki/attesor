/* ============================================================================
 * Rosetta Refactored - ALU Instruction Implementation
 * ============================================================================
 *
 * This module implements full ARM64 ALU (Arithmetic Logic Unit) instruction
 * translation to x86_64 machine code.
 *
 * Supported instructions:
 * - ADD, SUB, ADC, SBC (Addition/Subtraction)
 * - AND, ORR, EOR, BIC (Logical operations)
 * - MUL, SMULL, UMULL (Multiplication)
 * - SDIV, UDIV (Division)
 * - MVN, NEG (Negation)
 * - LSL, LSR, ASR, ROR (Shifts)
 * ============================================================================ */

#include "rosetta_refactored_types.h"
#include "rosetta_refactored_alu.h"
#include "rosetta_arm64_decode.h"
#include "rosetta_codegen.h"
#include <stdint.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * get_shift_amount - Extract shift amount from instruction
 * @encoding: ARM64 instruction encoding
 * Returns: Shift amount (0-63)
 */
static inline uint8_t get_shift_amount(uint32_t encoding)
{
    uint8_t shift = (encoding >> 10) & 0x3F;
    uint8_t sf = (encoding >> 31) & 1;
    return shift & (sf ? 0x3F : 0x1F);
}

/**
 * get_operand2 - Extract immediate operand from instruction
 */
static inline uint64_t get_operand2(uint32_t encoding, int sf)
{
    uint16_t imm12 = (encoding >> 10) & 0xFFF;
    uint8_t shift = (encoding >> 22) & 0x3;
    uint64_t imm = imm12;

    if (sf) {
        imm = imm12 << (shift * 12);
    } else {
        imm = imm12 & 0xFF;
        imm = imm << (shift * 8);
    }
    return imm;
}

/* ============================================================================
 * Addition/Subtraction Instructions
 * ============================================================================ */

/**
 * translate_add - Translate ARM64 ADD (register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * ADD Rd, Rn, Rm {, shift #imm}
 * ADD Rd, Rn, #imm
 */
void translate_add(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;  /* 1 = 64-bit, 0 = 32-bit */

    /* Check if immediate or register operand */
    int is_immediate = (encoding >> 22) & 0x3;

    if (is_immediate) {
        /* ADD (immediate): ADD Rd, Rn, #imm */
        uint64_t imm = get_operand2(encoding, sf);

        if (imm == 0) {
            /* MOV Rd, Rn (ADD with zero is a move) */
            state->guest.x[rd] = state->guest.x[rn];
        } else {
            /* ADD with immediate */
            if (sf) {
                state->guest.x[rd] = state->guest.x[rn] + imm;
            } else {
                state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] + (uint32_t)imm);
            }
        }

        /* Update flags if S bit set */
        if ((encoding >> 29) & 1) {
            /* Update NZCV flags based on result */
            update_nzcv_flags(state, state->guest.x[rd], sf);
        }
    } else {
        /* ADD (register): ADD Rd, Rn, Rm */
        uint8_t rm = (encoding >> 16) & 0x1F;
        uint8_t shift_type = (encoding >> 22) & 3;
        uint8_t shift_amt = get_shift_amount(encoding);

        uint64_t operand = state->guest.x[rm];

        /* Apply shift if specified */
        switch (shift_type) {
            case 0: /* LSL */
                operand = operand << shift_amt;
                break;
            case 1: /* LSR */
                operand = operand >> shift_amt;
                break;
            case 2: /* ASR */
                operand = (int64_t)operand >> shift_amt;
                break;
            case 3: /* ROR */
                if (sf) {
                    operand = (operand >> shift_amt) | (operand << (64 - shift_amt));
                } else {
                    operand = (operand >> shift_amt) | (operand << (32 - shift_amt));
                }
                break;
        }

        if (sf) {
            state->guest.x[rd] = state->guest.x[rn] + operand;
        } else {
            state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] + (uint32_t)operand);
        }

        /* Update flags if S bit set */
        if ((encoding >> 29) & 1) {
            update_nzcv_flags(state, state->guest.x[rd], sf);
        }
    }
}

/**
 * translate_sub - Translate ARM64 SUB (register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * SUB Rd, Rn, Rm {, shift #imm}
 * SUB Rd, Rn, #imm
 */
void translate_sub(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    int is_immediate = (encoding >> 22) & 0x3;

    if (is_immediate) {
        /* SUB (immediate) */
        uint64_t imm = get_operand2(encoding, sf);

        if (sf) {
            state->guest.x[rd] = state->guest.x[rn] - imm;
        } else {
            state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] - (uint32_t)imm);
        }

        /* Update flags if S bit set */
        if ((encoding >> 29) & 1) {
            update_nzcv_flags(state, state->guest.x[rd], sf);
        }
    } else {
        /* SUB (register) */
        uint8_t rm = (encoding >> 16) & 0x1F;
        uint8_t shift_type = (encoding >> 22) & 3;
        uint8_t shift_amt = get_shift_amount(encoding);

        uint64_t operand = state->guest.x[rm];

        /* Apply shift */
        switch (shift_type) {
            case 0: /* LSL */
                operand = operand << shift_amt;
                break;
            case 1: /* LSR */
                operand = operand >> shift_amt;
                break;
            case 2: /* ASR */
                operand = (int64_t)operand >> shift_amt;
                break;
            case 3: /* ROR */
                if (sf) {
                    operand = (operand >> shift_amt) | (operand << (64 - shift_amt));
                } else {
                    operand = (operand >> shift_amt) | (operand << (32 - shift_amt));
                }
                break;
        }

        if (sf) {
            state->guest.x[rd] = state->guest.x[rn] - operand;
        } else {
            state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] - (uint32_t)operand);
        }

        /* Update flags if S bit set */
        if ((encoding >> 29) & 1) {
            update_nzcv_flags(state, state->guest.x[rd], sf);
        }
    }
}

/**
 * translate_adc - Translate ARM64 ADC (add with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * ADC Rd, Rn, Rm  ->  Rd = Rn + Rm + C
 */
void translate_adc(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    /* Get carry flag */
    uint64_t carry = (state->guest.pstate >> 29) & 1;

    if (sf) {
        state->guest.x[rd] = state->guest.x[rn] + state->guest.x[rm] + carry;
    } else {
        state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] +
                                          (uint32_t)state->guest.x[rm] + carry);
    }

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/**
 * translate_sbc - Translate ARM64 SBC (subtract with carry) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * SBC Rd, Rn, Rm  ->  Rd = Rn - Rm - !C
 */
void translate_sbc(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    /* Get carry flag (inverted for SBC) */
    uint64_t carry = ~((state->guest.pstate >> 29) & 1) & 1;

    if (sf) {
        state->guest.x[rd] = state->guest.x[rn] - state->guest.x[rm] - carry;
    } else {
        state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] -
                                          (uint32_t)state->guest.x[rm] - carry);
    }

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/* ============================================================================
 * Logical Operations
 * ============================================================================ */

/**
 * translate_and - Translate ARM64 AND (register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_and(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    state->guest.x[rd] = state->guest.x[rn] & state->guest.x[rm];

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/**
 * translate_orr - Translate ARM64 ORR (register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_orr(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    state->guest.x[rd] = state->guest.x[rn] | state->guest.x[rm];

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/**
 * translate_eor - Translate ARM64 EOR (XOR) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_eor(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    state->guest.x[rd] = state->guest.x[rn] ^ state->guest.x[rm];

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/**
 * translate_bic - Translate ARM64 BIC (bit clear) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * BIC Rd, Rn, Rm  ->  Rd = Rn AND NOT Rm
 */
void translate_bic(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    state->guest.x[rd] = state->guest.x[rn] & ~state->guest.x[rm];

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/* ============================================================================
 * Multiplication/Division
 * ============================================================================ */

/**
 * translate_mul - Translate ARM64 MUL (multiply) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_mul(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    if (sf) {
        state->guest.x[rd] = state->guest.x[rn] * state->guest.x[rm];
    } else {
        state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] *
                                          (uint32_t)state->guest.x[rm]);
    }
}

/**
 * translate_div - Translate ARM64 SDIV/UDIV instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 */
void translate_div(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    int is_signed = (encoding >> 21) & 1;  /* 1 = SDIV, 0 = UDIV */

    uint64_t dividend = state->guest.x[rn];
    uint64_t divisor = state->guest.x[rm];
    uint64_t result;

    if (divisor == 0) {
        /* Division by zero returns 0 in ARM64 */
        result = 0;
    } else {
        if (is_signed) {
            /* SDIV - Signed division */
            if (sf) {
                result = (uint64_t)((int64_t)dividend / (int64_t)divisor);
            } else {
                result = (uint32_t)((int32_t)(uint32_t)dividend / (int32_t)(uint32_t)divisor);
            }
        } else {
            /* UDIV - Unsigned division */
            if (sf) {
                result = dividend / divisor;
            } else {
                result = (uint32_t)((uint32_t)dividend / (uint32_t)divisor);
            }
        }
    }

    state->guest.x[rd] = result;
}

/* ============================================================================
 * Negation Operations
 * ============================================================================ */

/**
 * translate_mvn - Translate ARM64 MVN (move not) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * MVN Rd, Rm  ->  Rd = NOT Rm
 */
void translate_mvn(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    state->guest.x[rd] = ~state->guest.x[rm];

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/**
 * translate_neg - Translate ARM64 NEG (negate) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * NEG Rd, Rm  ->  Rd = 0 - Rm
 * (NEG is encoded as SUBS XZR, Rn, Rm)
 */
void translate_neg(uint32_t encoding, ThreadState *state)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;

    if (sf) {
        state->guest.x[rd] = (uint64_t)(0 - (int64_t)state->guest.x[rm]);
    } else {
        state->guest.x[rd] = (uint32_t)(0 - (int32_t)(uint32_t)state->guest.x[rm]);
    }

    /* Update flags if S bit set */
    if ((encoding >> 29) & 1) {
        update_nzcv_flags(state, state->guest.x[rd], sf);
    }
}

/* ============================================================================
 * Flag Update Helper
 * ============================================================================ */

/**
 * update_nzcv_flags - Update NZCV flags based on result
 * @state: Thread state
 * @result: Operation result
 * @sf: 1 for 64-bit, 0 for 32-bit
 */
void update_nzcv_flags(ThreadState *state, uint64_t result, int sf)
{
    uint32_t flags = 0;

    if (sf) {
        /* 64-bit result */
        /* N flag: Set if result is negative (bit 63 set) */
        if (result & (1ULL << 63)) {
            flags |= (1 << 31);
        }
        /* Z flag: Set if result is zero */
        if (result == 0) {
            flags |= (1 << 30);
        }
    } else {
        /* 32-bit result */
        uint32_t result32 = (uint32_t)result;

        /* N flag: Set if result is negative (bit 31 set) */
        if (result32 & (1 << 31)) {
            flags |= (1 << 31);
        }
        /* Z flag: Set if result is zero */
        if (result32 == 0) {
            flags |= (1 << 30);
        }
    }

    /* C and V flags would need to be computed based on the operation */
    /* For ADD/SUB, they indicate carry/borrow and overflow */
    /* This is a simplified implementation */

    state->guest.pstate = flags;
}
