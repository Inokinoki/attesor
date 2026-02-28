/* ============================================================================
 * Rosetta Refactored - FP Compare Operations Implementation
 * ============================================================================
 *
 * This module implements ARM64 floating-point compare instruction
 * translation to x86_64 machine code.
 * ============================================================================ */

#include "rosetta_fp_compare.h"
#include <stdint.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static inline int fp_get_size(uint32_t encoding)
{
    return (encoding >> 22) & 1;
}

static inline uint8_t fp_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

static inline uint8_t fp_get_rm(uint32_t encoding)
{
    return (encoding >> 16) & 0x1F;
}

static inline uint8_t xmm_reg(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* ============================================================================
 * FP Compare Operations
 * ============================================================================ */

/**
 * translate_fp_cmp - Translate ARM64 FCMP instruction
 * FCMP Vn, Vm  - Compare two FP registers
 * Sets NZCV flags based on result
 */
int translate_fp_cmp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                     uint64_t *x_regs, uint32_t *pstate)
{
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);  /* 0 = single, 1 = double */
    int op = (encoding >> 14) & 0x03;  /* 0 = FCMP, 1 = FCMPE */

    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;
    (void)x_regs;
    (void)pstate;
    (void)op;

    /* UCOMISD/UCOMISS - Unordered Compare Scalar */
    code_buf_emit_byte(code_buf, 0x66);  /* SSE2 prefix for double */
    if (!size) {
        code_buf_emit_byte(code_buf, 0xF3);  /* SS prefix for single */
    }
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2E);  /* UCOMISD/UCOMISS */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rn << 3) + xmm_rm);

    /* The comparison sets x86 flags (ZF, PF, CF) which need to be
     * translated to ARM NZCV flags. This is handled at runtime. */

    return 0;
}

/**
 * translate_fp_cmpe - Translate ARM64 FCMPE instruction
 * FCMPE Vn, Vm  - Compare with exception on NaN
 */
int translate_fp_cmpe(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                      uint64_t *x_regs, uint32_t *pstate)
{
    /* FCMPE is like FCMP but raises exception on NaN */
    /* For translation, we use UCOMISD which handles NaN specially */
    return translate_fp_cmp(encoding, code_buf, vec_regs, x_regs, pstate);
}

/**
 * translate_fp_cmp_zero - Translate ARM64 FCMP with zero
 * FCMP Vn, #0.0  - Compare FP register with zero
 */
int translate_fp_cmp_zero(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                          uint32_t *pstate)
{
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;
    (void)pstate;

    /* Use XOR to zero a temporary register */
    uint8_t xmm_tmp = 0;  /* Use XMM0 as temp */

    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x57);  /* XORPS */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_tmp);

    /* UCOMISD/UCOMISS against zero */
    code_buf_emit_byte(code_buf, 0x66);
    if (!size) {
        code_buf_emit_byte(code_buf, 0xF3);
    }
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2E);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rn << 3) + xmm_tmp);

    return 0;
}

/* ============================================================================
 * FP Compare Dispatch
 * ============================================================================ */

/**
 * translate_fp_compare_dispatch - Dispatch FP compare instruction
 */
int translate_fp_compare_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                  Vector128 *vec_regs, uint64_t *x_regs, uint32_t *pstate)
{
    /* Check for FP compare instruction pattern */
    /* FCMP: 0x1E 0x20 0x20 0x00 encoding */
    uint32_t mask = 0xFF20FC00;
    uint32_t match = 0x1E202000;

    if ((encoding & mask) != match) {
        return -1;  /* Not an FP compare instruction */
    }

    /* Check for zero immediate variant */
    if ((encoding & 0x0000001F) == 0x00000000) {
        /* FCMP Vn, #0.0 */
        return translate_fp_cmp_zero(encoding, code_buf, vec_regs, pstate);
    }

    /* Check for FCMPE (with exceptions) */
    if ((encoding & 0x00000010) != 0) {
        return translate_fp_cmpe(encoding, code_buf, vec_regs, x_regs, pstate);
    }

    /* Standard FCMP */
    return translate_fp_cmp(encoding, code_buf, vec_regs, x_regs, pstate);
}
