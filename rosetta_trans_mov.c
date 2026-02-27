/* ============================================================================
 * Rosetta Translator - MOV Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements ARM64 MOV family instruction translation:
 * - MOVZ: Move Wide with Zero
 * - MOVK: Move Wide with Keep
 * - MOVN: Move Wide with Not
 * - MVN: Move Not (bitwise NOT)
 * - NEG: Negate
 * - REV/REV16/REV32: Byte reversal
 * ============================================================================ */

#include "rosetta_trans_mov.h"
#include "rosetta_arm64_decode.h"

/* ============================================================================
 * MOV Wide Instruction Translation
 * ============================================================================ */

/**
 * translate_movz - Translate ARM64 MOVZ instruction
 *
 * MOVZ moves a 16-bit immediate to a register, zeroing the rest.
 * Encoding: 11010010100iiiiinnnnn00000100000
 *
 * The hw field (bits 21-22) specifies the shift:
 * - 0: No shift (bits 0-15)
 * - 1: Shift by 16 (bits 16-31)
 * - 2: Shift by 32 (bits 32-47)
 * - 3: Shift by 48 (bits 48-63)
 */
int translate_movz(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint16_t imm16 = arm64_get_imm16(encoding);
    uint8_t hw = arm64_get_hw(encoding);

    /* Build the 64-bit value with appropriate shift */
    uint64_t value = ((uint64_t)imm16) << (hw * 16);

    state->guest.x[rd] = value;

    return 0;
}

/**
 * translate_movk - Translate ARM64 MOVK instruction
 *
 * MOVK moves a 16-bit immediate into a register, keeping other bits.
 * Encoding: 11110010100iiiiinnnnn00000100000
 *
 * Used to build 64-bit constants in combination with MOVZ.
 */
int translate_movk(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint16_t imm16 = arm64_get_imm16(encoding);
    uint8_t hw = arm64_get_hw(encoding);

    /* Clear the target 16-bit field and insert new value */
    uint64_t mask = ~(((uint64_t)0xFFFF) << (hw * 16));
    uint64_t value = (state->guest.x[rd] & mask) | (((uint64_t)imm16) << (hw * 16));

    state->guest.x[rd] = value;

    return 0;
}

/**
 * translate_movn - Translate ARM64 MOVN instruction
 *
 * MOVN moves the bitwise NOT of a 16-bit immediate to a register.
 * Encoding: 10010010100iiiiinnnnn00000100000
 *
 * Used to load negative constants efficiently.
 */
int translate_movn(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint16_t imm16 = arm64_get_imm16(encoding);
    uint8_t hw = arm64_get_hw(encoding);

    /* Build the 64-bit value with appropriate shift, then NOT */
    uint64_t value = ((uint64_t)imm16) << (hw * 16);
    value = ~value;

    state->guest.x[rd] = value;

    return 0;
}

/* ============================================================================
 * MOV/Register Alias Translation
 * ============================================================================ */

/**
 * translate_mov - Translate ARM64 MOV (register alias) instruction
 *
 * MOV (register) is an alias for ORR with the zero register (XZR).
 * Encoding: 101010101010000000000000001mmmmm
 *
 * This is effectively: MOV Rd, Rm -> ORR Rd, XZR, Rm
 */
int translate_mov(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    /* MOV Rd, Rm is ORR Rd, XZR, Rm */
    state->guest.x[rd] = state->guest.x[rm];

    return 0;
}

/**
 * translate_mvn - Translate ARM64 MVN (Move Not) instruction
 *
 * MVN moves the bitwise NOT of a register.
 * Encoding: 001010100010000000000000001mmmmm
 *
 * MVN Rd, Rm -> Rd = ~Rm
 */
int translate_mvn(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    state->guest.x[rd] = ~state->guest.x[rm];

    return 0;
}

/* ============================================================================
 * Negate Instruction Translation
 * ============================================================================ */

/**
 * translate_neg - Translate ARM64 NEG (Negate) instruction
 *
 * NEG negates a register (subtracts from zero).
 * Encoding: 110100010010000000000000001mmmmm
 *
 * NEG Rd, Rm -> Rd = 0 - Rm (which is SUBS with Rn = ZR)
 */
int translate_neg(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t op2 = state->guest.x[rm];
    uint64_t result = 0 - op2;

    state->guest.x[rd] = result;

    /* Update flags like SUBS */
    uint64_t nzcv = 0;
    if (result & (1ULL << 63)) {
        nzcv |= NZCV_N;
    }
    if (result == 0) {
        nzcv |= NZCV_Z;
    }
    if (0 >= op2) {
        nzcv |= NZCV_C;
    }
    /* V flag: overflow if op2 is INT64_MIN */
    if ((int64_t)op2 == (int64_t)0x8000000000000000ULL) {
        nzcv |= NZCV_V;
    }

    state->guest.pstate = nzcv;

    return 0;
}

/**
 * translate_ngc - Translate ARM64 NGC (Negate with Carry) instruction
 *
 * NGC negates a register and subtracts the carry flag.
 * Encoding: 110100010010000000000000101mmmmm
 *
 * NGC Rd, Rm -> Rd = 0 - Rm - NOT(C)
 */
int translate_ngc(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    /* Get carry flag (inverted - C=0 means borrow) */
    uint64_t carry = (state->guest.pstate & NZCV_C) ? 0 : 1;

    uint64_t op2 = state->guest.x[rm];
    uint64_t result = 0 - op2 - carry;

    state->guest.x[rd] = result;

    /* Update flags */
    uint64_t nzcv = 0;
    if (result & (1ULL << 63)) {
        nzcv |= NZCV_N;
    }
    if (result == 0) {
        nzcv |= NZCV_Z;
    }
    if (0 >= op2 + carry) {
        nzcv |= NZCV_C;
    }

    state->guest.pstate = nzcv;

    return 0;
}

/* ============================================================================
 * Byte Reversal Instruction Translation
 * ============================================================================ */

/**
 * translate_rev - Translate ARM64 REV (Reverse Bytes) instruction
 *
 * REV reverses the byte order in a register.
 * Encoding: 110101010011000000000000001mmmmm
 *
 * REV Xd, Xm -> Xd = byteswap(Xm)
 */
int translate_rev(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t src = state->guest.x[rm];

    /* Byte swap the 64-bit value */
    uint64_t result = ((src & 0x00000000000000FFULL) << 56) |
                      ((src & 0x000000000000FF00ULL) << 40) |
                      ((src & 0x0000000000FF0000ULL) << 24) |
                      ((src & 0x00000000FF000000ULL) << 8)  |
                      ((src & 0x000000FF00000000ULL) >> 8)  |
                      ((src & 0x0000FF0000000000ULL) >> 24) |
                      ((src & 0x00FF000000000000ULL) >> 40) |
                      ((src & 0xFF00000000000000ULL) >> 56);

    state->guest.x[rd] = result;

    return 0;
}

/**
 * translate_rev16 - Translate ARM64 REV16 instruction
 *
 * REV16 reverses bytes within each 16-bit halfword.
 * Encoding: 000110101000000000000000001mmmmm
 */
int translate_rev16(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t src = state->guest.x[rm];

    /* Reverse bytes within each 16-bit halfword */
    uint64_t result = ((src & 0x00FF00FF00FF00FFULL) << 8) |
                      ((src & 0xFF00FF00FF00FF00ULL) >> 8);

    state->guest.x[rd] = result;

    return 0;
}

/**
 * translate_rev32 - Translate ARM64 REV32 instruction
 *
 * REV32 reverses bytes within each 32-bit word.
 * Encoding: 010110101000000000000000001mmmmm
 */
int translate_rev32(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rd = arm64_get_rd(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t src = state->guest.x[rm];

    /* Reverse bytes within each 32-bit word */
    uint32_t lo = (uint32_t)(src & 0xFFFFFFFFULL);
    uint32_t hi = (uint32_t)(src >> 32);

    lo = ((lo & 0x000000FF) << 24) |
         ((lo & 0x0000FF00) << 8) |
         ((lo & 0x00FF0000) >> 8) |
         ((lo & 0xFF000000) >> 24);

    hi = ((hi & 0x000000FF) << 24) |
         ((hi & 0x0000FF00) << 8) |
         ((hi & 0x00FF0000) >> 8) |
         ((hi & 0xFF000000) >> 24);

    state->guest.x[rd] = ((uint64_t)hi << 32) | lo;

    return 0;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * build_imm64 - Build a 64-bit immediate from MOVZ/MOVK sequence
 *
 * ARM64 often uses sequences like:
 *   MOVZ X0, #0x1234      ; X0 = 0x0000000000001234
 *   MOVK X0, #0x5678, LSL #16  ; X0 = 0x0000000056781234
 *   MOVK X0, #0xABCD, LSL #32  ; X0 = 0x00000000ABCD56781234
 *   MOVK X0, #0xEF01, LSL #48  ; X0 = 0xEF01ABCD56781234
 */
uint64_t build_imm64(ThreadState *state, const uint8_t *insn, int max_insns)
{
    uint64_t value = 0;
    bool started = false;
    int i;

    for (i = 0; i < max_insns; i++) {
        uint32_t encoding = *((const uint32_t *)(insn + i * 4));
        uint8_t rd;
        uint16_t imm16;
        uint8_t hw;

        if (arm64_is_movz(encoding)) {
            rd = arm64_get_rd(encoding);
            imm16 = arm64_get_imm16(encoding);
            hw = arm64_get_hw(encoding);

            value = ((uint64_t)imm16) << (hw * 16);
            started = true;
            (void)rd;  /* Assume same register */
        } else if (arm64_is_movk(encoding) && started) {
            rd = arm64_get_rd(encoding);
            imm16 = arm64_get_imm16(encoding);
            hw = arm64_get_hw(encoding);

            uint64_t mask = ~(((uint64_t)0xFFFF) << (hw * 16));
            value = (value & mask) | (((uint64_t)imm16) << (hw * 16));
            (void)rd;
        } else {
            break;  /* End of MOV sequence */
        }
    }

    (void)state;  /* Unused in this implementation */
    return value;
}
