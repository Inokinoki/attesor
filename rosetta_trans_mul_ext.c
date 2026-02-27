/* ============================================================================
 * Rosetta Translator - Multiply/Divide Extension Implementation
 * ============================================================================
 *
 * This module implements ARM64 multiply and divide extension instructions:
 * - MADD: Multiply Add
 * - MSUB: Multiply Subtract
 * - SMADDL: Signed Multiply Add Long
 * - SMSUBL: Signed Multiply Subtract Long
 * - UMADDL: Unsigned Multiply Add Long
 * - UMSUBL: Unsigned Multiply Subtract Long
 * - MUL: Multiply
 * - SMULL: Signed Multiply Long
 * - SMULH: Signed Multiply High
 * - UMULL: Unsigned Multiply Long
 * - UMULH: Unsigned Multiply High
 * - SDIV: Signed Divide
 * - UDIV: Unsigned Divide
 * ============================================================================ */

#include "rosetta_trans_mul_ext.h"
#include "rosetta_arm64_decode.h"

/* ============================================================================
 * Multiply-Accumulate Instructions
 * ============================================================================ */

/**
 * translate_madd - Translate MADD (multiply add) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * MADD Rd, Rn, Rm, Ra  ->  Rd = Rn * Rm + Ra
 */
int translate_madd(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = op1 * op2 + acc;

    return 0;
}

/**
 * translate_msub - Translate MSUB (multiply subtract) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * MSUB Rd, Rn, Rm, Ra  ->  Rd = Rn * Rm - Ra
 */
int translate_msub(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = op1 * op2 - acc;

    return 0;
}

/* ============================================================================
 * Long Multiply-Accumulate Instructions (64-bit result from 32-bit operands)
 * ============================================================================ */

/**
 * translate_smaddl - Translate SMADDL (signed multiply add long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SMADDL Rd, Rn, Rm, Ra  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0]) + Ra
 */
int translate_smaddl(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    int64_t op1 = (int32_t)(uint32_t)state->guest.x[rn];
    int64_t op2 = (int32_t)(uint32_t)state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = (uint64_t)(op1 * op2) + acc;

    return 0;
}

/**
 * translate_smsubl - Translate SMSUBL (signed multiply subtract long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SMSUBL Rd, Rn, Rm, Ra  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0]) - Ra
 */
int translate_smsubl(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    int64_t op1 = (int32_t)(uint32_t)state->guest.x[rn];
    int64_t op2 = (int32_t)(uint32_t)state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = (uint64_t)(op1 * op2) - acc;

    return 0;
}

/**
 * translate_umaddl - Translate UMADDL (unsigned multiply add long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UMADDL Rd, Rn, Rm, Ra  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0]) + Ra
 */
int translate_umaddl(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    uint64_t op1 = (uint32_t)state->guest.x[rn];
    uint64_t op2 = (uint32_t)state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = op1 * op2 + acc;

    return 0;
}

/**
 * translate_umsubl - Translate UMSUBL (unsigned multiply subtract long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UMSUBL Rd, Rn, Rm, Ra  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0]) - Ra
 */
int translate_umsubl(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t ra = (insn[3] >> 10) & 0x1F;

    uint64_t op1 = (uint32_t)state->guest.x[rn];
    uint64_t op2 = (uint32_t)state->guest.x[rm];
    uint64_t acc = state->guest.x[ra];

    state->guest.x[rd] = op1 * op2 - acc;

    return 0;
}

/* ============================================================================
 * Basic Multiply Instructions
 * ============================================================================ */

/**
 * translate_mul - Translate MUL (multiply) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * MUL Rd, Rn, Rm  ->  Rd = Rn * Rm
 */
int translate_mul(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t sf = (insn[3] >> 31) & 1;

    if (sf) {
        state->guest.x[rd] = state->guest.x[rn] * state->guest.x[rm];
    } else {
        state->guest.x[rd] = (uint32_t)((uint32_t)state->guest.x[rn] *
                                         (uint32_t)state->guest.x[rm]);
    }

    return 0;
}

/**
 * translate_smull - Translate SMULL (signed multiply long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SMULL Rd, Rn, Rm  ->  Rd = sext(Rn[31:0]) * sext(Rm[31:0])
 */
int translate_smull(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    int64_t op1 = (int32_t)(uint32_t)state->guest.x[rn];
    int64_t op2 = (int32_t)(uint32_t)state->guest.x[rm];

    state->guest.x[rd] = (uint64_t)(op1 * op2);

    return 0;
}

/**
 * translate_umull - Translate UMULL (unsigned multiply long) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UMULL Rd, Rn, Rm  ->  Rd = zext(Rn[31:0]) * zext(Rm[31:0])
 */
int translate_umull(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint64_t op1 = (uint32_t)state->guest.x[rn];
    uint64_t op2 = (uint32_t)state->guest.x[rm];

    state->guest.x[rd] = op1 * op2;

    return 0;
}

/**
 * translate_smulh - Translate SMULH (signed multiply high) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SMULH Rd, Rn, Rm  ->  Rd = (sext(Rn) * sext(Rm)) >> 64
 */
int translate_smulh(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    /* Use 128-bit multiplication to get high 64 bits */
    int64_t op1 = (int64_t)state->guest.x[rn];
    int64_t op2 = (int64_t)state->guest.x[rm];

    /* For signed multiplication, we need the high 64 bits of the 128-bit result */
    /* Split into high and low 32-bit parts */
    int64_t op1_hi = op1 >> 32;
    int64_t op1_lo = (int32_t)op1;
    int64_t op2_hi = op2 >> 32;
    int64_t op2_lo = (int32_t)op2;

    /* Compute partial products */
    int64_t hi_hi = op1_hi * op2_hi;
    int64_t hi_lo = op1_hi * op2_lo;
    int64_t lo_hi = op1_lo * op2_hi;
    /* lo_lo doesn't affect high 64 bits directly */

    /* Combine with proper shifts */
    state->guest.x[rd] = hi_hi + ((hi_lo >> 32) & 0xFFFFFFFF) + ((lo_hi >> 32) & 0xFFFFFFFF);

    return 0;
}

/**
 * translate_umulh - Translate UMULH (unsigned multiply high) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UMULH Rd, Rn, Rm  ->  Rd = (zext(Rn) * zext(Rm)) >> 64
 */
int translate_umulh(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    /* Use 128-bit multiplication to get high 64 bits */
    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];

    /* Split into high and low 32-bit parts */
    uint64_t op1_hi = op1 >> 32;
    uint64_t op1_lo = op1 & 0xFFFFFFFF;
    uint64_t op2_hi = op2 >> 32;
    uint64_t op2_lo = op2 & 0xFFFFFFFF;

    /* Compute partial products that affect high 64 bits */
    uint64_t hi_hi = op1_hi * op2_hi;
    uint64_t hi_lo = op1_hi * op2_lo;
    uint64_t lo_hi = op1_lo * op2_hi;

    /* Combine with proper shifts */
    state->guest.x[rd] = hi_hi + ((hi_lo >> 32) & 0xFFFFFFFF) + ((lo_hi >> 32) & 0xFFFFFFFF);

    return 0;
}

/* ============================================================================
 * Division Instructions
 * ============================================================================ */

/**
 * translate_sdiv - Translate SDIV (signed divide) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SDIV Rd, Rn, Rm  ->  Rd = sext(Rn) / sext(Rm)
 */
int translate_sdiv(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t sf = (insn[3] >> 31) & 1;

    int64_t dividend = (int64_t)state->guest.x[rn];
    int64_t divisor = (int64_t)state->guest.x[rm];
    int64_t result;

    if (divisor == 0) {
        /* Division by zero returns 0 in ARM64 */
        result = 0;
    } else if (dividend == INT64_MIN && divisor == -1) {
        /* Overflow case: INT64_MIN / -1 would overflow */
        result = INT64_MIN;
    } else {
        if (sf) {
            result = dividend / divisor;
        } else {
            result = (int32_t)((int32_t)dividend / (int32_t)divisor);
        }
    }

    state->guest.x[rd] = (uint64_t)result;

    return 0;
}

/**
 * translate_udiv - Translate UDIV (unsigned divide) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UDIV Rd, Rn, Rm  ->  Rd = zext(Rn) / zext(Rm)
 */
int translate_udiv(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t sf = (insn[3] >> 31) & 1;

    uint64_t dividend = state->guest.x[rn];
    uint64_t divisor = state->guest.x[rm];
    uint64_t result;

    if (divisor == 0) {
        /* Division by zero returns 0 in ARM64 */
        result = 0;
    } else {
        if (sf) {
            result = dividend / divisor;
        } else {
            result = (uint32_t)((uint32_t)dividend / (uint32_t)divisor);
        }
    }

    state->guest.x[rd] = result;

    return 0;
}

/* ============================================================================
 * Remainder Instructions
 * ============================================================================ */

/**
 * translate_smod - Translate SMOD (signed remainder) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * SMOD Rd, Rn, Rm  ->  Rd = sext(Rn) % sext(Rm)
 */
int translate_smod(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t sf = (insn[3] >> 31) & 1;

    int64_t dividend = (int64_t)state->guest.x[rn];
    int64_t divisor = (int64_t)state->guest.x[rm];
    int64_t result;

    if (divisor == 0) {
        /* Division by zero returns 0 in ARM64 */
        result = 0;
    } else {
        if (sf) {
            result = dividend % divisor;
        } else {
            result = (int32_t)((int32_t)dividend % (int32_t)divisor);
        }
    }

    state->guest.x[rd] = (uint64_t)result;

    return 0;
}

/**
 * translate_umod - Translate UMOD (unsigned remainder) instruction
 * @state: Thread state
 * @insn: ARM64 instruction bytes
 * Returns: 0 on success, -1 on failure
 *
 * UMOD Rd, Rn, Rm  ->  Rd = zext(Rn) % zext(Rm)
 */
int translate_umod(ThreadState *state, const uint8_t *insn)
{
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;
    uint8_t sf = (insn[3] >> 31) & 1;

    uint64_t dividend = state->guest.x[rn];
    uint64_t divisor = state->guest.x[rm];
    uint64_t result;

    if (divisor == 0) {
        /* Division by zero returns 0 in ARM64 */
        result = 0;
    } else {
        if (sf) {
            result = dividend % divisor;
        } else {
            result = (uint32_t)((uint32_t)dividend % (uint32_t)divisor);
        }
    }

    state->guest.x[rd] = result;

    return 0;
}
