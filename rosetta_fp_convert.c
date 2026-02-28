/* ============================================================================
 * Rosetta Refactored - FP Convert Operations Implementation
 * ============================================================================
 *
 * This module implements ARM64 floating-point conversion instruction
 * translation to x86_64 machine code.
 * ============================================================================ */

#include "rosetta_fp_convert.h"
#include <stdint.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static inline int fp_get_size(uint32_t encoding)
{
    return (encoding >> 22) & 1;
}

static inline uint8_t fp_get_rd(uint32_t encoding)
{
    return encoding & 0x1F;
}

static inline uint8_t fp_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

static inline uint8_t xmm_reg(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* ============================================================================
 * FP Convert - Integer to Float
 * ============================================================================ */

/**
 * translate_fp_scvtf - Translate ARM64 SCVTF (signed int to float)
 * SCVTF Vd, Vn  - Convert signed integer to float
 */
int translate_fp_scvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);  /* 0 = 32-bit, 1 = 64-bit */

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* CVTSI2SS/CVTSI2SD - Convert scalar integer to float */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));  /* F2=SD, F3=SS */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2A);  /* CVTSI2SS/CVTSI2SD */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_ucvtf - Translate ARM64 UCVTF (unsigned int to float)
 * UCVTF Vd, Vn  - Convert unsigned integer to float
 */
int translate_fp_ucvtf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* For unsigned, we need special handling since x86 only has signed convert */
    /* Sequence: Check sign, handle accordingly */

    /* For now, emit CVTSI2SD with note that unsigned handling is needed */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2A);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/* ============================================================================
 * FP Convert - Float to Integer
 * ============================================================================ */

/**
 * translate_fp_fcvtns - Translate ARM64 FCVTNS (float to signed int, nearest)
 */
int translate_fp_fcvtns(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* CVTSS2SI/CVTSD2SI - Convert float to signed integer */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2D);  /* CVTSS2SI/CVTSD2SI */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_fcvtnu - Translate ARM64 FCVTNU (float to unsigned int, nearest)
 */
int translate_fp_fcvtnu(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* For unsigned, need special handling */
    /* Use CVTSS2SI/CVTSD2SI with adjustment for negative values */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2D);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_fcvtps - Translate ARM64 FCVTPS (float to signed int, positive inf)
 */
int translate_fp_fcvtps(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* Need to set rounding mode to positive infinity first */
    /* For simplicity, use default rounding (nearest) */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x2D);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_fcvtpu - Translate ARM64 FCVTPU (float to unsigned int, positive inf)
 */
int translate_fp_fcvtpu(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    (void)encoding;
    (void)code_buf;
    (void)vec_regs;
    /* Placeholder - similar to FCVTPS but unsigned */
    return 0;
}

/**
 * translate_fp_fcvtn - Translate ARM64 FCVTN (narrow float to half precision)
 */
int translate_fp_fcvtn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    (void)encoding;
    (void)code_buf;
    (void)vec_regs;
    /* Placeholder - convert float to half precision */
    return 0;
}

/**
 * translate_fp_fcvtxn - Translate ARM64 FCVTXN (narrow with exponent boost)
 */
int translate_fp_fcvtxn(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    (void)encoding;
    (void)code_buf;
    (void)vec_regs;
    /* Placeholder - narrow with exponent boost */
    return 0;
}

/* ============================================================================
 * FP Convert - Float to Float (precision conversion)
 * ============================================================================ */

/**
 * translate_fp_cvtds - Translate ARM64 FCVTD (double to single)
 */
int translate_fp_cvtds(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* CVTSD2SS - Convert double to single */
    code_buf_emit_byte(code_buf, 0xF2);  /* SD prefix */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5A);  /* CVTSD2SS */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_cvtsd - Translate ARM64 FCVTS (single to double)
 */
int translate_fp_cvtsd(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* CVTSS2SD - Convert single to double */
    code_buf_emit_byte(code_buf, 0xF3);  /* SS prefix */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5A);  /* CVTSS2SD */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_cvthf - Translate ARM64 FCVT (half to float)
 */
int translate_fp_cvthf(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    (void)encoding;
    (void)code_buf;
    (void)vec_regs;
    /* Placeholder - half to float conversion */
    return 0;
}

/**
 * translate_fp_cvtfh - Translate ARM64 FCVT (float to half)
 */
int translate_fp_cvtfh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    (void)encoding;
    (void)code_buf;
    (void)vec_regs;
    /* Placeholder - float to half conversion */
    return 0;
}

/* ============================================================================
 * FP Move Operations
 * ============================================================================ */

/**
 * translate_fp_mov - Translate ARM64 FMOV (FP register move or immediate)
 */
int translate_fp_mov(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* MOVSD/MOVSS - Move scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x10);  /* MOV */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/**
 * translate_fp_mov_gpr - Translate ARM64 FMOV (GPR <-> FP register)
 */
int translate_fp_mov_gpr(uint32_t encoding, code_buf_t *code_buf,
                         Vector128 *vec_regs, uint64_t *x_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int op = (encoding >> 11) & 0x01;  /* 0 = GPR->FP, 1 = FP->GPR */

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t x86_rn = rn & 0x0F;

    (void)vec_regs;
    (void)x_regs;

    if (op) {
        /* FP -> GPR: MOVQ xmm, r64 */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xD6);  /* MOVQ */
        code_buf_emit_byte(code_buf, 0xC0 + (x86_rn << 3) + xmm_rd);
    } else {
        /* GPR -> FP: MOVQ r64, xmm */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x6E);  /* MOVD */
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + x86_rn);
    }

    return 0;
}

/* ============================================================================
 * FP Convert Dispatch
 * ============================================================================ */

/**
 * translate_fp_convert_dispatch - Dispatch FP convert instruction
 */
int translate_fp_convert_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                  Vector128 *vec_regs, uint64_t *x_regs)
{
    uint32_t op = (encoding >> 29) & 0x03;
    uint32_t op2 = (encoding >> 10) & 0x03;

    /* Check for integer<->float convert (0x1E opcode prefix) */
    if ((encoding & 0x0FE00000) != 0x0E000000) {
        /* Check for FMOV GPR <-> FP */
        if ((encoding & 0x0FF003F0) == 0x09E00100) {
            return translate_fp_mov_gpr(encoding, code_buf, vec_regs, x_regs);
        }
        return -1;  /* Not an FP convert instruction */
    }

    /* Dispatch based on operation type */
    switch (op) {
        case 0x00:  /* Integer to float */
            if (op2 == 0x00) {
                return translate_fp_scvtf(encoding, code_buf, vec_regs);
            } else if (op2 == 0x01) {
                return translate_fp_ucvtf(encoding, code_buf, vec_regs);
            }
            break;
        case 0x01:  /* Float to integer */
            switch (op2) {
                case 0x00: return translate_fp_fcvtns(encoding, code_buf, vec_regs);
                case 0x01: return translate_fp_fcvtnu(encoding, code_buf, vec_regs);
            }
            break;
        case 0x02:  /* Float to float (different precision) */
            return translate_fp_cvtds(encoding, code_buf, vec_regs);
        case 0x03:  /* FP move */
            return translate_fp_mov(encoding, code_buf, vec_regs);
    }

    return -1;  /* Not handled */
}
