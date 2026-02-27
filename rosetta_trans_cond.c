/* ============================================================================
 * Rosetta Translator - Conditional Instruction Implementation
 * ============================================================================
 *
 * This module implements ARM64 conditional select and conditional operation
 * instructions:
 * - CSEL: Conditional Select
 * - CSINC: Conditional Select Increment
 * - CSINV: Conditional Select Invert
 * - CSNEG: Conditional Select Negate
 * - CSET: Condition Set
 * - CSETM: Condition Set Mask
 * - CINC: Conditional Increment
 * - CINVERT: Conditional Invert
 * - CNEG: Conditional Negate
 * ============================================================================ */

#include "rosetta_trans_cond.h"
#include "rosetta_arm64_decode.h"

/* ============================================================================
 * Condition Code Helper
 * ============================================================================ */

/**
 * check_condition_code - Check if condition code is satisfied
 * @cond: Condition code (0-15)
 * @pstate: Processor state (NZCV flags)
 * Returns: true if condition is true, false otherwise
 */
bool check_condition_code(uint8_t cond, uint64_t pstate)
{
    uint8_t N = (pstate >> 31) & 1;
    uint8_t Z = (pstate >> 30) & 1;
    uint8_t C = (pstate >> 29) & 1;
    uint8_t V = (pstate >> 28) & 1;

    switch (cond) {
        case 0x0: return (Z == 1);                    /* EQ - Equal */
        case 0x1: return (Z == 0);                    /* NE - Not equal */
        case 0x2: return (C == 1);                    /* CS/HS - Carry set / Unsigned higher or same */
        case 0x3: return (C == 0);                    /* CC/LO - Carry clear / Unsigned lower */
        case 0x4: return (N == 1);                    /* MI - Minus / Negative */
        case 0x5: return (N == 0);                    /* PL - Plus / Positive or zero */
        case 0x6: return (V == 1);                    /* VS - Overflow */
        case 0x7: return (V == 0);                    /* VC - No overflow */
        case 0x8: return (C == 1 && Z == 0);          /* HI - Unsigned higher */
        case 0x9: return (C == 0 || Z == 1);          /* LS - Unsigned lower or same */
        case 0xA: return (N == V);                    /* GE - Signed greater than or equal */
        case 0xB: return (N != V);                    /* LT - Signed less than */
        case 0xC: return (Z == 0 && N == V);          /* GT - Signed greater than */
        case 0xD: return (Z == 1 || N != V);          /* LE - Signed less than or equal */
        case 0xE: return true;                        /* AL - Always */
        case 0xF: return false;                       /* NV - Never */
        default: return false;
    }
}

/**
 * get_inverted_condition - Get inverted condition code
 * @cond: Original condition code
 * Returns: Inverted condition code
 */
uint8_t get_inverted_condition(uint8_t cond)
{
    /* Invert the condition by XORing with 1 */
    return cond ^ 1;
}

/* ============================================================================
 * Conditional Select (CSEL) and Variants
 * ============================================================================ */

/**
 * translate_csel - Translate CSEL (conditional select) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSEL Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm;
 */
int translate_csel(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn];
    } else {
        state->guest.x[rd] = state->guest.x[rm];
    }

    return 0;
}

/**
 * translate_csinc - Translate CSINC (conditional select increment) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSINC Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm + 1;
 */
int translate_csinc(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn];
    } else {
        state->guest.x[rd] = state->guest.x[rm] + 1;
    }

    return 0;
}

/**
 * translate_csinv - Translate CSINV (conditional select invert) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSINV Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = ~Rm;
 */
int translate_csinv(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn];
    } else {
        state->guest.x[rd] = ~state->guest.x[rm];
    }

    return 0;
}

/**
 * translate_csneg - Translate CSNEG (conditional select negate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSNEG Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = -Rm;
 */
int translate_csneg(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn];
    } else {
        state->guest.x[rd] = -state->guest.x[rm];
    }

    return 0;
}

/* ============================================================================
 * Condition Set Instructions
 * ============================================================================ */

/**
 * translate_cset - Translate CSET (condition set) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSET Rd, cond  ->  Rd = (cond) ? 1 : 0;
 * Alias: CSINC Rd, XZR, XZR, cond
 */
int translate_cset(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    state->guest.x[rd] = cond_true ? 1 : 0;

    return 0;
}

/**
 * translate_csetm - Translate CSETM (condition set mask) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CSETM Rd, cond  ->  Rd = (cond) ? 0xFFFFFFFFFFFFFFFF : 0;
 * Alias: CSINV Rd, XZR, XZR, ~cond
 */
int translate_csetm(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    state->guest.x[rd] = cond_true ? ~0ULL : 0;

    return 0;
}

/* ============================================================================
 * Conditional Operation Instructions (aliases)
 * ============================================================================ */

/**
 * translate_cinc - Translate CINC (conditional increment) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CINC Rd, Rn, cond  ->  if (cond) Rd = Rn + 1; else Rd = Rn;
 * Alias: CSINC Rd, Rn, Rn, ~cond
 */
int translate_cinc(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = state->guest.x[rn] + 1;
    } else {
        state->guest.x[rd] = state->guest.x[rn];
    }

    return 0;
}

/**
 * translate_cinvert - Translate CINVERT (conditional invert) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CINVERT Rd, Rn, cond  ->  if (cond) Rd = ~Rn; else Rd = Rn;
 * Alias: CSINV Rd, Rn, Rn, ~cond
 */
int translate_cinvert(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = ~state->guest.x[rn];
    } else {
        state->guest.x[rd] = state->guest.x[rn];
    }

    return 0;
}

/**
 * translate_cneg - Translate CNEG (conditional negate) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * CNEG Rd, Rn, cond  ->  if (cond) Rd = -Rn; else Rd = Rn;
 * Alias: CSNEG Rd, Rn, Rn, ~cond
 */
int translate_cneg(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t cond = (insn[0] >> 12) & 0x0F;

    bool cond_true = check_condition_code(cond, state->guest.pstate);

    if (cond_true) {
        state->guest.x[rd] = -state->guest.x[rn];
    } else {
        state->guest.x[rd] = state->guest.x[rn];
    }

    return 0;
}

/* ============================================================================
 * Conditional Select with Zero Variants
 * ============================================================================ */

/**
 * translate_sel_z - Translate SEL (vector select) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SEL Vd.Vn, Vm  ->  Select elements based on condition
 */
int translate_sel_z(ThreadState *state, const uint8_t *insn)
{
    /* Placeholder for SIMD vector select */
    (void)state;
    (void)insn;
    return 0;
}
