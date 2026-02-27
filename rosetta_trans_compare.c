/* ============================================================================
 * Rosetta Translator - Compare Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements ARM64 compare instruction translation:
 * - CMP: Compare (subtract and update flags)
 * - CMN: Compare Negative (add and update flags)
 * - TST: Test (AND and update flags)
 * ============================================================================ */

#include "rosetta_trans_compare.h"
#include "rosetta_arm64_decode.h"

/* ============================================================================
 * Flag Update Helpers
 * ============================================================================ */

/**
 * update_nzcv_flags_sub - Update NZCV flags after subtraction
 *
 * N (Negative): Set if result is negative (bit 63 = 1)
 * Z (Zero): Set if result is zero
 * C (Carry): Set if no borrow occurred (op1 >= op2 unsigned)
 * V (Overflow): Set if signed overflow occurred
 */
void update_nzcv_flags_sub(ThreadState *state, uint64_t result,
                           uint64_t op1, uint64_t op2)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative (bit 63 set) */
    if (result & (1ULL << 63)) {
        nzcv |= NZCV_N;
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= NZCV_Z;
    }

    /* C flag - Carry: set if no borrow (op1 >= op2 unsigned) */
    if (op1 >= op2) {
        nzcv |= NZCV_C;
    }

    /* V flag - Overflow: set if signed overflow occurred
     * Overflow occurs when:
     * - op1 is positive, op2 is negative, result is negative
     * - op1 is negative, op2 is positive, result is positive
     */
    int64_t a = (int64_t)op1;
    int64_t b = (int64_t)op2;
    int64_t r = (int64_t)result;
    if ((a > 0 && b < 0 && r < 0) || (a < 0 && b > 0 && r > 0)) {
        nzcv |= NZCV_V;
    }

    state->guest.pstate = nzcv;
}

/**
 * update_nzcv_flags_add - Update NZCV flags after addition
 *
 * N (Negative): Set if result is negative (bit 63 = 1)
 * Z (Zero): Set if result is zero
 * C (Carry): Set if unsigned overflow occurred (result < op1)
 * V (Overflow): Set if signed overflow occurred
 */
void update_nzcv_flags_add(ThreadState *state, uint64_t result,
                           uint64_t op1, uint64_t op2)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative */
    if (result & (1ULL << 63)) {
        nzcv |= NZCV_N;
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= NZCV_Z;
    }

    /* C flag - Carry: set if unsigned overflow (result < op1) */
    if (result < op1) {
        nzcv |= NZCV_C;
    }

    /* V flag - Overflow: set if signed overflow occurred
     * Overflow occurs when:
     * - Both operands positive, result negative
     * - Both operands negative, result positive
     */
    int64_t a = (int64_t)op1;
    int64_t b = (int64_t)op2;
    int64_t r = (int64_t)result;
    if ((a > 0 && b > 0 && r < 0) || (a < 0 && b < 0 && r > 0)) {
        nzcv |= NZCV_V;
    }

    state->guest.pstate = nzcv;
}

/**
 * update_nzcv_flags_and - Update NZ flags after AND operation
 *
 * N (Negative): Set if result is negative (bit 63 = 1)
 * Z (Zero): Set if result is zero
 * C, V: Unchanged for logical operations
 */
void update_nzcv_flags_and(ThreadState *state, uint64_t result)
{
    uint64_t nzcv = 0;

    /* N flag - Set if result is negative */
    if (result & (1ULL << 63)) {
        nzcv |= NZCV_N;
    }

    /* Z flag - Set if result is zero */
    if (result == 0) {
        nzcv |= NZCV_Z;
    }

    /* C and V flags are unchanged for logical operations */
    state->guest.pstate = nzcv;
}

/* ============================================================================
 * Compare Translation Functions
 * ============================================================================ */

/**
 * translate_cmp - Translate ARM64 CMP instruction
 *
 * CMP (register) compares two registers by subtraction.
 * Encoding: 1110101100000000nnnnn000001mmmmm
 * Bits:     [31:24] [23:16]   [15:8]    [7:0]
 *
 * The result is discarded, only flags are updated.
 */
int translate_cmp(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Decode instruction - treat as little-endian 32-bit value */
    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 - op2;

    update_nzcv_flags_sub(state, result, op1, op2);

    return 0;
}

/**
 * translate_cmp_imm - Translate ARM64 CMP immediate instruction
 *
 * CMP (immediate) compares register with 12-bit immediate.
 * Encoding: 1111000100iiiiiiinnnnn00000111111
 *
 * The result is discarded, only flags are updated.
 */
int translate_cmp_imm(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint16_t imm12 = arm64_get_imm12(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = (uint64_t)imm12;
    uint64_t result = op1 - op2;

    update_nzcv_flags_sub(state, result, op1, op2);

    return 0;
}

/**
 * translate_cmn - Translate ARM64 CMN instruction
 *
 * CMN (compare negative) adds two registers and updates flags.
 * Encoding: 1010101100000000nnnnn000001mmmmm
 *
 * The result is discarded, only flags are updated.
 */
int translate_cmn(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 + op2;

    update_nzcv_flags_add(state, result, op1, op2);

    return 0;
}

/**
 * translate_cmn_imm - Translate ARM64 CMN immediate instruction
 *
 * CMN (immediate) adds register with 12-bit immediate.
 */
int translate_cmn_imm(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint16_t imm12 = arm64_get_imm12(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = (uint64_t)imm12;
    uint64_t result = op1 + op2;

    update_nzcv_flags_add(state, result, op1, op2);

    return 0;
}

/**
 * translate_tst - Translate ARM64 TST instruction
 *
 * TST (test) performs bitwise AND and updates NZ flags.
 * Encoding: 1110101000000000nnnnn000001mmmmm
 *
 * The result is discarded, only flags are updated.
 */
int translate_tst(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint8_t rm = arm64_get_rm(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t op2 = state->guest.x[rm];
    uint64_t result = op1 & op2;

    update_nzcv_flags_and(state, result);

    return 0;
}

/**
 * translate_tst_imm - Translate ARM64 TST immediate instruction
 *
 * TST (immediate) performs bitwise AND with 12-bit immediate.
 */
int translate_tst_imm(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint32_t encoding = *((const uint32_t *)insn);

    uint8_t rn = arm64_get_rn(encoding);
    uint16_t imm12 = arm64_get_imm12(encoding);

    uint64_t op1 = state->guest.x[rn];
    uint64_t result = op1 & imm12;

    update_nzcv_flags_and(state, result);

    return 0;
}
