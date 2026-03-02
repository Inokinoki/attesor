/* ============================================================================
 * Rosetta Translator - ALU Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements ARM64 arithmetic and logic instruction translation.
 * Note: Uses host.x[] for ARM64 registers (host architecture)
 * ============================================================================ */

#include "rosetta_trans_alu.h"

/* ============================================================================
 * Flag Update Helpers
 * ============================================================================ */

void update_nzcv_flags(ThreadState *state, uint64_t result, uint64_t op1,
                       uint64_t op2, bool is_add, bool is_logical)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative (bit 63 set) */
    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= (1ULL << 30);
    }

    if (!is_logical) {
        /* C flag - Carry/Borrow flag */
        if (is_add) {
            /* Addition: carry if result < op1 (unsigned overflow) */
            if (result < op1) {
                nzcv |= (1ULL << 29);
            }
        } else {
            /* Subtraction: borrow if op1 < op2 (unsigned underflow) */
            if (op1 < op2) {
                nzcv |= (1ULL << 29);
            }
        }

        /* V flag - Overflow flag (signed overflow) */
        if (is_add) {
            /* Overflow if operands have same sign but result has different sign */
            int64_t a = (int64_t)op1;
            int64_t b = (int64_t)op2;
            int64_t r = (int64_t)result;
            if ((a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0)) {
                nzcv |= (1ULL << 28);
            }
        } else {
            /* Subtraction: overflow if signs differ and result sign differs from op1 */
            int64_t a = (int64_t)op1;
            int64_t b = (int64_t)op2;
            int64_t r = (int64_t)result;
            if ((a > 0 && b < 0 && r < 0) || (a < 0 && b > 0 && r > 0)) {
                nzcv |= (1ULL << 28);
            }
        }
    }

    state->host.pstate = nzcv;
}

void update_nzcv_flags_and(ThreadState *state, uint64_t result)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative */
    if (result & (1ULL << 63)) {
        nzcv |= (1ULL << 31);
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= (1ULL << 30);
    }

    /* C and V flags are unchanged for logical operations */
    state->host.pstate = nzcv;
}

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

int translate_add(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = state->host.x[rm];

    state->host.x[rd] = op1 + op2;
    update_nzcv_flags(state, state->host.x[rd], op1, op2, true, false);

    return 0;
}

int translate_sub(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = state->host.x[rm];

    state->host.x[rd] = op1 - op2;
    update_nzcv_flags(state, state->host.x[rd], op1, op2, false, false);

    return 0;
}

int translate_and(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = state->host.x[rm];

    state->host.x[rd] = op1 & op2;
    update_nzcv_flags_and(state, state->host.x[rd]);

    return 0;
}

int translate_orr(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = state->host.x[rm];

    state->host.x[rd] = op1 | op2;
    update_nzcv_flags_and(state, state->host.x[rd]);

    return 0;
}

int translate_eor(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = state->host.x[rm];

    state->host.x[rd] = op1 ^ op2;
    update_nzcv_flags_and(state, state->host.x[rd]);

    return 0;
}

int translate_mul(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    state->host.x[rd] = state->host.x[rn] * state->host.x[rm];

    return 0;
}

int translate_div(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    bool signed_div = (insn[3] >> 1) & 1;

    uint64_t divisor = state->host.x[rm];
    if (divisor == 0) {
        state->host.x[rd] = 0;  /* ARM64 returns 0 on div by zero */
        return 0;
    }

    if (signed_div) {
        state->host.x[rd] = (int64_t)state->host.x[rn] / (int64_t)divisor;
    } else {
        state->host.x[rd] = state->host.x[rn] / divisor;
    }

    return 0;
}

int translate_mvn(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    state->host.x[rd] = ~state->host.x[rm];

    return 0;
}

int translate_add_imm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = (uint16_t)((insn[2] >> 10) & 0xFFF);

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = imm12;

    state->host.x[rd] = op1 + op2;
    update_nzcv_flags(state, state->host.x[rd], op1, op2, true, false);

    return 0;
}

int translate_sub_imm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = (uint16_t)((insn[2] >> 10) & 0xFFF);

    uint64_t op1 = state->host.x[rn];
    uint64_t op2 = imm12;

    state->host.x[rd] = op1 - op2;
    update_nzcv_flags(state, state->host.x[rd], op1, op2, false, false);

    return 0;
}

int translate_and_imm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = (uint16_t)((insn[2] >> 10) & 0xFFF);

    state->host.x[rd] = state->host.x[rn] & imm12;
    update_nzcv_flags_and(state, state->host.x[rd]);

    return 0;
}

int translate_orr_imm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint16_t imm12 = (uint16_t)((insn[2] >> 10) & 0xFFF);

    state->host.x[rd] = state->host.x[rn] | imm12;
    update_nzcv_flags_and(state, state->host.x[rd]);

    return 0;
}
