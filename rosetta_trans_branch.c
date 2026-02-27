/* ============================================================================
 * Rosetta Translator - Branch Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements ARM64 branch instruction translation.
 * ============================================================================ */

#include "rosetta_trans_branch.h"

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

int translate_b(ThreadState *state, const uint8_t *insn)
{
    int32_t imm26 = (int32_t)((insn[0] >> 2) | ((insn[1] & 0x03) << 6) |
                              ((insn[2] & 0xFF) << 8) | ((insn[3] & 0x03) << 16));
    imm26 = (imm26 << 6) >> 6;  /* Sign extend */

    state->guest.pc += imm26 * 4;

    return 0;
}

int translate_bl(ThreadState *state, const uint8_t *insn)
{
    int32_t imm26 = (int32_t)((insn[0] >> 2) | ((insn[1] & 0x03) << 6) |
                              ((insn[2] & 0xFF) << 8) | ((insn[3] & 0x03) << 16));
    imm26 = (imm26 << 6) >> 6;  /* Sign extend */

    /* Save return address */
    state->guest.x[30] = state->guest.pc + 4;

    /* Branch to target */
    state->guest.pc += imm26 * 4;

    return 0;
}

int translate_br(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    state->guest.pc = state->guest.x[rn];

    return 0;
}

int translate_bcond(ThreadState *state, const uint8_t *insn)
{
    uint8_t cond = (insn[0] >> 4) & 0x0F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;  /* Sign extend */

    /* Check condition flags (NZCV) */
    uint8_t N = (state->guest.pstate >> 31) & 1;  /* Negative */
    uint8_t Z = (state->guest.pstate >> 30) & 1;  /* Zero */
    uint8_t C = (state->guest.pstate >> 29) & 1;  /* Carry */
    uint8_t V = (state->guest.pstate >> 28) & 1;  /* Overflow */

    bool taken = false;
    switch (cond) {
        case 0x0: taken = (Z == 1); break;  /* EQ */
        case 0x1: taken = (Z == 0); break;  /* NE */
        case 0x2: taken = (C == 1); break;  /* CS/HS */
        case 0x3: taken = (C == 0); break;  /* CC/LO */
        case 0x4: taken = (N == 1); break;  /* MI */
        case 0x5: taken = (N == 0); break;  /* PL */
        case 0x6: taken = (V == 1); break;  /* VS */
        case 0x7: taken = (V == 0); break;  /* VC */
        case 0x8: taken = (N != V); break;  /* LT */
        case 0x9: taken = (N == V); break;  /* GE */
        case 0xA: taken = (Z || (N != V)); break;  /* LE */
        case 0xB: taken = (!Z && (N == V)); break;  /* GT */
        case 0xC: taken = (N == 1 || Z == 1); break;  /* LE (alt) */
        case 0xD: taken = (N == 0 && Z == 0); break;  /* GT (alt) */
        case 0xE: break;  /* AL (always) */
        case 0xF: break;  /* NV - reserved */
    }

    if (taken) {
        state->guest.pc += imm19 * 4;
    }

    return 0;
}

int translate_cbz(ThreadState *state, const uint8_t *insn)
{
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;

    if (state->guest.x[rt] == 0) {
        state->guest.pc += imm19 * 4;
    }

    return 0;
}

int translate_cbnz(ThreadState *state, const uint8_t *insn)
{
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    int32_t imm19 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x7F) << 8));
    imm19 = (imm19 << 13) >> 13;

    if (state->guest.x[rt] != 0) {
        state->guest.pc += imm19 * 4;
    }

    return 0;
}

int translate_tbz(ThreadState *state, const uint8_t *insn)
{
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t bit = ((insn[0] >> 5) & 0x07) | ((insn[1] & 0x01) << 3);
    int32_t imm14 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x3F) << 8));
    imm14 = (imm14 << 18) >> 18;  /* Sign extend */

    if ((state->guest.x[rt] & (1ULL << bit)) == 0) {
        state->guest.pc += imm14 * 4;
    }

    return 0;
}

int translate_tbnz(ThreadState *state, const uint8_t *insn)
{
    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t bit = ((insn[0] >> 5) & 0x07) | ((insn[1] & 0x01) << 3);
    int32_t imm14 = (int32_t)((insn[1] >> 3) | ((insn[2] & 0x07) << 5) |
                              ((insn[3] & 0x3F) << 8));
    imm14 = (imm14 << 18) >> 18;  /* Sign extend */

    if ((state->guest.x[rt] & (1ULL << bit)) != 0) {
        state->guest.pc += imm14 * 4;
    }

    return 0;
}

int translate_ret(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    state->guest.pc = state->guest.x[rn];

    return 0;
}

int translate_blr(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    /* Save return address */
    state->guest.x[30] = state->guest.pc + 4;

    /* Branch to register */
    state->guest.pc = state->guest.x[rn];

    return 0;
}

int translate_csel(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    /* Check condition flags */
    uint8_t N = (state->guest.pstate >> 31) & 1;
    uint8_t Z = (state->guest.pstate >> 30) & 1;
    uint8_t C = (state->guest.pstate >> 29) & 1;
    uint8_t V = (state->guest.pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;  /* EQ */
        case 0x1: cond_true = (Z == 0); break;  /* NE */
        case 0x2: cond_true = (C == 1); break;  /* CS/HS */
        case 0x3: cond_true = (C == 0); break;  /* CC/LO */
        case 0x4: cond_true = (N == 1); break;  /* MI */
        case 0x5: cond_true = (N == 0); break;  /* PL */
        case 0x6: cond_true = (V == 1); break;  /* VS */
        case 0x7: cond_true = (V == 0); break;  /* VC */
        case 0x8: cond_true = (N != V); break;  /* LT */
        case 0x9: cond_true = (N == V); break;  /* GE */
        case 0xA: cond_true = (Z || (N != V)); break;  /* LE */
        case 0xB: cond_true = (!Z && (N == V)); break;  /* GT */
        case 0xC: cond_true = (N == 1 || Z == 1); break;  /* LE (alt) */
        case 0xD: cond_true = (N == 0 && Z == 0); break;  /* GT (alt) */
        default: cond_true = true;  /* AL */
    }

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn];
    } else {
        state->guest.x[rd] = state->guest.x[rm];
    }

    return 0;
}

int translate_cmp(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 - op2;

    /* Update NZCV flags */
    uint64_t nzcv = 0;

    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);  /* N flag */
    }
    if (result == 0) {
        nzcv |= (1ULL << 30);  /* Z flag */
    }
    if (op1 < op2) {
        nzcv |= (1ULL << 29);  /* C flag (borrow) */
    }
    /* V flag - signed overflow */
    int64_t a = (int64_t)op1;
    int64_t b = (int64_t)op2;
    int64_t r = (int64_t)result;
    if ((a > 0 && b < 0 && r < 0) || (a < 0 && b > 0 && r > 0)) {
        nzcv |= (1ULL << 28);
    }

    state->guest.pstate = nzcv;

    return 0;
}

int translate_cmn(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 + op2;

    /* Update NZCV flags */
    uint64_t nzcv = 0;

    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);  /* N flag */
    }
    if (result == 0) {
        nzcv |= (1ULL << 30);  /* Z flag */
    }
    if (result < op1) {
        nzcv |= (1ULL << 29);  /* C flag (carry) */
    }
    /* V flag - signed overflow */
    int64_t a = (int64_t)op1;
    int64_t b = (int64_t)op2;
    int64_t r = (int64_t)result;
    if ((a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0)) {
        nzcv |= (1ULL << 28);
    }

    state->guest.pstate = nzcv;

    return 0;
}

int translate_tst(ThreadState *state, const uint8_t *insn)
{
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t result = state->guest.x[rn] & state->guest.x[rm];

    /* Update N, Z flags */
    uint64_t nzcv = 0;

    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);  /* N flag */
    }
    if (result == 0) {
        nzcv |= (1ULL << 30);  /* Z flag */
    }

    state->guest.pstate = nzcv;

    return 0;
}
