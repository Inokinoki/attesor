/* ============================================================================
 * Rosetta Refactored - Control Flow Instruction Implementation
 * ============================================================================
 *
 * This module implements full ARM64 control flow instruction translation.
 *
 * Supported instructions:
 * - B, BL (Branch, Branch with Link)
 * - BR, BLR (Branch to Register, Branch with Link to Register)
 * - RET (Return)
 * - B.cond (Conditional Branch)
 * - CBZ, CBNZ (Compare and Branch)
 * - TBZ, TBNZ (Test and Branch)
 * ============================================================================ */

#include "rosetta_refactored_control.h"
#include "rosetta_arm64_decode.h"
#include <stdint.h>

/* ============================================================================
 * Immediate Branch Instructions
 * ============================================================================ */

/**
 * translate_b - Translate ARM64 B (unconditional branch) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * B label  ->  PC = PC + imm26 * 4
 */
void translate_b(uint32_t encoding, ThreadState *state)
{
    int32_t imm26 = (int32_t)(encoding & 0x3FFFFFF);

    /* Sign extend 26-bit immediate */
    imm26 = (imm26 << 6) >> 6;

    /* Scale by 4 (word-aligned) */
    int64_t offset = imm26 * 4;

    /* Update program counter */
    state->guest.pc = state->guest.pc + offset;
}

/**
 * translate_bl - Translate ARM64 BL (branch with link) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * BL label  ->  LR = PC + 4; PC = PC + imm26 * 4
 */
void translate_bl(uint32_t encoding, ThreadState *state)
{
    int32_t imm26 = (int32_t)(encoding & 0x3FFFFFF);

    /* Sign extend 26-bit immediate */
    imm26 = (imm26 << 6) >> 6;

    /* Scale by 4 */
    int64_t offset = imm26 * 4;

    /* Link: save return address in LR (X30) */
    state->guest.x[30] = state->guest.pc + 4;

    /* Update program counter */
    state->guest.pc = state->guest.pc + offset;
}

/* ============================================================================
 * Register Branch Instructions
 * ============================================================================ */

/**
 * translate_br - Translate ARM64 BR (branch to register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * BR Xm  ->  PC = Xm
 */
void translate_br(uint32_t encoding, ThreadState *state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;

    /* Branch to address in register */
    state->guest.pc = state->guest.x[rn];
}

/**
 * translate_blr - Translate ARM64 BLR (branch with link to register) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * BLR Xm  ->  LR = PC + 4; PC = Xm
 */
void translate_blr(uint32_t encoding, ThreadState *state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;

    /* Link: save return address in LR (X30) */
    state->guest.x[30] = state->guest.pc + 4;

    /* Branch to address in register */
    state->guest.pc = state->guest.x[rn];
}

/**
 * translate_ret - Translate ARM64 RET (return) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * RET {Xn}  ->  PC = Xn (default X30 = LR)
 */
void translate_ret(uint32_t encoding, ThreadState *state)
{
    uint8_t rn = (encoding >> 5) & 0x1F;

    /* Return to address in register (default LR/X30) */
    state->guest.pc = state->guest.x[rn];
}

/* ============================================================================
 * Conditional Branch Instructions
 * ============================================================================ */

/**
 * translate_bcond - Translate ARM64 B.cond (conditional branch) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * B.cond label  ->  if (condition) PC = PC + imm19 * 4
 *
 * Conditions:
 *  0000 = EQ (Z == 1)           Equal
 *  0001 = NE (Z == 0)           Not equal
 *  0010 = CS/HS (C == 1)        Carry set / Unsigned higher or same
 *  0011 = CC/LO (C == 0)        Carry clear / Unsigned lower
 *  0100 = MI (N == 1)           Minus / Negative
 *  0101 = PL (N == 0)           Plus / Positive or zero
 *  0110 = VS (V == 1)           Overflow
 *  0111 = VC (V == 0)           No overflow
 *  1000 = HI (C == 1 && Z == 0) Unsigned higher
 *  1001 = LS (C == 0 || Z == 1) Unsigned lower or same
 *  1010 = GE (N == V)           Signed greater than or equal
 *  1011 = LT (N != V)           Signed less than
 *  1100 = GT (Z == 0 && N == V) Signed greater than
 *  1101 = LE (Z == 1 || N != V) Signed less than or equal
 *  1110 = AL (always)           Always (unconditional)
 *  1111 = NV (never)            Never (nop)
 */
void translate_bcond(uint32_t encoding, ThreadState *state)
{
    uint8_t cond = (encoding >> 0) & 0xF;
    int32_t imm19 = (int32_t)((encoding >> 5) & 0x7FFFF);

    /* Sign extend 19-bit immediate */
    imm19 = (imm19 << 13) >> 13;

    /* Scale by 4 */
    int64_t offset = imm19 * 4;

    /* Extract flags */
    uint8_t Z = (state->guest.pstate >> 30) & 1;
    uint8_t N = (state->guest.pstate >> 31) & 1;
    uint8_t C = (state->guest.pstate >> 29) & 1;
    uint8_t V = (state->guest.pstate >> 28) & 1;

    int taken = 0;

    switch (cond) {
        case 0x0:  /* EQ */
            taken = (Z == 1);
            break;
        case 0x1:  /* NE */
            taken = (Z == 0);
            break;
        case 0x2:  /* CS/HS */
            taken = (C == 1);
            break;
        case 0x3:  /* CC/LO */
            taken = (C == 0);
            break;
        case 0x4:  /* MI */
            taken = (N == 1);
            break;
        case 0x5:  /* PL */
            taken = (N == 0);
            break;
        case 0x6:  /* VS */
            taken = (V == 1);
            break;
        case 0x7:  /* VC */
            taken = (V == 0);
            break;
        case 0x8:  /* HI */
            taken = (C == 1 && Z == 0);
            break;
        case 0x9:  /* LS */
            taken = (C == 0 || Z == 1);
            break;
        case 0xA:  /* GE */
            taken = (N == V);
            break;
        case 0xB:  /* LT */
            taken = (N != V);
            break;
        case 0xC:  /* GT */
            taken = (Z == 0 && N == V);
            break;
        case 0xD:  /* LE */
            taken = (Z == 1 || N != V);
            break;
        case 0xE:  /* AL */
            taken = 1;
            break;
        case 0xF:  /* NV */
            taken = 0;
            break;
    }

    if (taken) {
        state->guest.pc = state->guest.pc + offset;
    } else {
        /* Fall through - PC already points to next instruction */
    }
}

/* ============================================================================
 * Compare and Branch Instructions
 * ============================================================================ */

/**
 * translate_cbz - Translate ARM64 CBZ (compare and branch if zero) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * CBZ Rt, label  ->  if (Rt == 0) PC = PC + imm19 * 4
 */
void translate_cbz(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    int32_t imm19 = (int32_t)((encoding >> 5) & 0x7FFFF);

    /* Sign extend 19-bit immediate */
    imm19 = (imm19 << 13) >> 13;

    /* Scale by 4 */
    int64_t offset = imm19 * 4;

    /* Check if register is zero */
    int is_zero;
    if (sf) {
        is_zero = (state->guest.x[rt] == 0);
    } else {
        is_zero = ((uint32_t)state->guest.x[rt] == 0);
    }

    if (is_zero) {
        state->guest.pc = state->guest.pc + offset;
    }
}

/**
 * translate_cbnz - Translate ARM64 CBNZ (compare and branch if not zero) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * CBNZ Rt, label  ->  if (Rt != 0) PC = PC + imm19 * 4
 */
void translate_cbnz(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = (encoding >> 5) & 0x1F;
    uint8_t sf = (encoding >> 31) & 1;
    int32_t imm19 = (int32_t)((encoding >> 5) & 0x7FFFF);

    /* Sign extend 19-bit immediate */
    imm19 = (imm19 << 13) >> 13;

    /* Scale by 4 */
    int64_t offset = imm19 * 4;

    /* Check if register is not zero */
    int is_not_zero;
    if (sf) {
        is_not_zero = (state->guest.x[rt] != 0);
    } else {
        is_not_zero = ((uint32_t)state->guest.x[rt] != 0);
    }

    if (is_not_zero) {
        state->guest.pc = state->guest.pc + offset;
    }
}

/* ============================================================================
 * Test and Branch Instructions
 * ============================================================================ */

/**
 * translate_tbz - Translate ARM64 TBZ (test bit and branch if zero) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * TBZ Rt, #bit, label  ->  if ((Rt >> bit) & 1 == 0) PC = PC + imm14 * 4
 */
void translate_tbz(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = (encoding >> 5) & 0x1F;
    uint8_t b5 = (encoding >> 31) & 1;  /* Bit 5 of bit position */
    uint8_t b40 = (encoding >> 19) & 0x1F;  /* Bits 4:0 of bit position */
    int32_t imm14 = (int32_t)((encoding >> 5) & 0x3FFF);

    /* Construct full bit position */
    uint8_t bit_pos = (b5 << 5) | b40;

    /* Sign extend 14-bit immediate */
    imm14 = (imm14 << 18) >> 18;

    /* Scale by 4 */
    int64_t offset = imm14 * 4;

    /* Test the bit */
    uint64_t value = state->guest.x[rt];
    int bit_is_zero = !((value >> bit_pos) & 1);

    if (bit_is_zero) {
        state->guest.pc = state->guest.pc + offset;
    }
}

/**
 * translate_tbnz - Translate ARM64 TBNZ (test bit and branch if not zero) instruction
 * @encoding: ARM64 instruction encoding
 * @state: Thread state
 *
 * TBNZ Rt, #bit, label  ->  if ((Rt >> bit) & 1 == 1) PC = PC + imm14 * 4
 */
void translate_tbnz(uint32_t encoding, ThreadState *state)
{
    uint8_t rt = (encoding >> 5) & 0x1F;
    uint8_t b5 = (encoding >> 31) & 1;
    uint8_t b40 = (encoding >> 19) & 0x1F;
    int32_t imm14 = (int32_t)((encoding >> 5) & 0x3FFF);

    /* Construct full bit position */
    uint8_t bit_pos = (b5 << 5) | b40;

    /* Sign extend 14-bit immediate */
    imm14 = (imm14 << 18) >> 18;

    /* Scale by 4 */
    int64_t offset = imm14 * 4;

    /* Test the bit */
    uint64_t value = state->guest.x[rt];
    int bit_is_set = ((value >> bit_pos) & 1);

    if (bit_is_set) {
        state->guest.pc = state->guest.pc + offset;
    }
}

/* ============================================================================
 * Condition Flag Helpers
 * ============================================================================ */

/**
 * check_condition - Check if a condition code is satisfied
 * @cond: Condition code (0-15)
 * @state: Thread state
 * Returns: 1 if condition is true, 0 otherwise
 */
int check_condition(uint8_t cond, ThreadState *state)
{
    uint8_t Z = (state->guest.pstate >> 30) & 1;
    uint8_t N = (state->guest.pstate >> 31) & 1;
    uint8_t C = (state->guest.pstate >> 29) & 1;
    uint8_t V = (state->guest.pstate >> 28) & 1;

    switch (cond) {
        case 0x0: return (Z == 1);                    /* EQ */
        case 0x1: return (Z == 0);                    /* NE */
        case 0x2: return (C == 1);                    /* CS/HS */
        case 0x3: return (C == 0);                    /* CC/LO */
        case 0x4: return (N == 1);                    /* MI */
        case 0x5: return (N == 0);                    /* PL */
        case 0x6: return (V == 1);                    /* VS */
        case 0x7: return (V == 0);                    /* VC */
        case 0x8: return (C == 1 && Z == 0);          /* HI */
        case 0x9: return (C == 0 || Z == 1);          /* LS */
        case 0xA: return (N == V);                    /* GE */
        case 0xB: return (N != V);                    /* LT */
        case 0xC: return (Z == 0 && N == V);          /* GT */
        case 0xD: return (Z == 1 || N != V);          /* LE */
        case 0xE: return 1;                           /* AL */
        case 0xF: return 0;                           /* NV */
        default: return 0;
    }
}
