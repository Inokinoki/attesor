/* ============================================================================
 * Rosetta Refactored - FP Arithmetic Operations Implementation
 * ============================================================================
 *
 * This module implements ARM64 floating-point arithmetic instruction
 * translation to x86_64 machine code.
 * ============================================================================ */

#include "rosetta_fp_arith.h"
#include <stdint.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * fp_get_size - Get FP operation size from encoding
 * @encoding: ARM64 instruction encoding
 * Returns: 0 for single precision (32-bit), 1 for double precision (64-bit)
 */
static inline int fp_get_size(uint32_t encoding)
{
    return (encoding >> 22) & 1;
}

/**
 * fp_get_rd - Get destination FP register
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t fp_get_rd(uint32_t encoding)
{
    return encoding & 0x1F;
}

/**
 * fp_get_rn - Get first source FP register
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t fp_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * fp_get_rm - Get second source FP register
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t fp_get_rm(uint32_t encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * fp_get_ra - Get accumulator FP register (for fused operations)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t fp_get_ra(uint32_t encoding)
{
    return (encoding >> 10) & 0x1F;
}

/**
 * xmm_reg - Map ARM64 FP register to x86_64 XMM register
 * @arm_reg: ARM64 register number (0-31)
 * Returns: XMM register number (0-15)
 */
static inline uint8_t xmm_reg(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* ============================================================================
 * FP Arithmetic Operations - Basic
 * ============================================================================ */

/**
 * translate_fp_add - Translate ARM64 FADD instruction
 * FADD Vd, Vn, Vm
 */
int translate_fp_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);  /* 0 = single, 1 = double */

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;  /* Registers are accessed via emitted code */

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);  /* SSE2 prefix */
    code_buf_emit_byte(code_buf, 0x0F);
    if (size) {
        code_buf_emit_byte(code_buf, 0x28);  /* MOVAPD xmm1, xmm2 */
    } else {
        code_buf_emit_byte(code_buf, 0x28);  /* MOVAPS xmm1, xmm2 */
    }
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* ADDSD/ADDSS - Add scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));  /* F2=SD, F3=SS */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x58);  /* ADD */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_sub - Translate ARM64 FSUB instruction
 * FSUB Vd, Vn, Vm
 */
int translate_fp_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* SUBSD/SUBSS - Subtract scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x58 - 0x10);  /* SUB = 0x58 - 0x10 = 0x48, but we use 0x5C */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_mul - Translate ARM64 FMUL instruction
 * FMUL Vd, Vn, Vm
 */
int translate_fp_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* MULSD/MULSS - Multiply scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x59);  /* MUL */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_div - Translate ARM64 FDIV instruction
 * FDIV Vd, Vn, Vm
 */
int translate_fp_div(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* DIVSD/DIVSS - Divide scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5E);  /* DIV */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_sqrt - Translate ARM64 FSQRT instruction
 * FSQRT Vd, Vn
 */
int translate_fp_sqrt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* SQRTSD/SQRTSS - Square root scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x51);  /* SQRT */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    return 0;
}

/* ============================================================================
 * FP Arithmetic Operations - Min/Max
 * ============================================================================ */

/**
 * translate_fp_max - Translate ARM64 FMAX instruction
 * FMAX Vd, Vn, Vm
 */
int translate_fp_max(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* MAXSD/MAXSS - Maximum scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5F);  /* MAX */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_min - Translate ARM64 FMIN instruction
 * FMIN Vd, Vn, Vm
 */
int translate_fp_min(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* MINSD/MINSS - Minimum scalar */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5D);  /* MIN */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    return 0;
}

/**
 * translate_fp_maxnm - Translate ARM64 FMAXNM instruction
 * FMAXNM Vd, Vn, Vm (maximum number, returns numeric max even with NaN)
 */
int translate_fp_maxnm(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* FMAXNM - For now, use FMAX as approximation */
    /* Full implementation would handle NaN specially */
    return translate_fp_max(encoding, code_buf, vec_regs);
}

/**
 * translate_fp_minnm - Translate ARM64 FMINNM instruction
 * FMINNM Vd, Vn, Vm (minimum number, returns numeric min even with NaN)
 */
int translate_fp_minnm(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* FMINNM - For now, use FMIN as approximation */
    /* Full implementation would handle NaN specially */
    return translate_fp_min(encoding, code_buf, vec_regs);
}

/* ============================================================================
 * FP Arithmetic Operations - Unary
 * ============================================================================ */

/**
 * translate_fp_abs - Translate ARM64 FABS instruction
 * FABS Vd, Vn
 */
int translate_fp_abs(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* ANDPD/ANDPS - Clear sign bit (absolute value) */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x54);  /* AND */

    /* Emit reference to constant with sign bit cleared */
    /* For scalar: use immediate AND with sign mask */
    if (size) {
        /* Double precision: AND with 0x7FFFFFFFFFFFFFFF */
        code_buf_emit_byte(code_buf, 0x05);  /* ANDPD with memory */
        code_buf_emit_byte(code_buf, 0x25);
        code_buf_emit_word32(code_buf, 0);  /* Placeholder for address */
    } else {
        /* Single precision: AND with 0x7FFFFFFF */
        code_buf_emit_byte(code_buf, 0x05);  /* ANDPS with memory */
        code_buf_emit_byte(code_buf, 0x25);
        code_buf_emit_word32(code_buf, 0);  /* Placeholder for address */
    }

    return 0;
}

/**
 * translate_fp_neg - Translate ARM64 FNEG instruction
 * FNEG Vd, Vn
 */
int translate_fp_neg(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);

    (void)vec_regs;

    /* MOVAPD/MOVAPS - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* XORPD/XORPS - Flip sign bit (negate) */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x57);  /* XOR */

    /* Emit reference to constant with sign bit set */
    if (size) {
        /* Double precision: XOR with 0x8000000000000000 */
        code_buf_emit_byte(code_buf, 0x05);  /* XORPD with memory */
        code_buf_emit_byte(code_buf, 0x25);
        code_buf_emit_word32(code_buf, 0);  /* Placeholder for address */
    } else {
        /* Single precision: XOR with 0x80000000 */
        code_buf_emit_byte(code_buf, 0x05);  /* XORPS with memory */
        code_buf_emit_byte(code_buf, 0x25);
        code_buf_emit_word32(code_buf, 0);  /* Placeholder for address */
    }

    return 0;
}

/* ============================================================================
 * FP Arithmetic Operations - Fused Multiply-Add
 * ============================================================================ */

/**
 * translate_fp_fma - Translate ARM64 FMADD instruction
 * FMADD Vd, Vn, Vm, Va  =>  Vd = Va + (Vn * Vm)
 */
int translate_fp_fma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    uint8_t ra = fp_get_ra(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);
    uint8_t xmm_ra = xmm_reg(ra);

    (void)vec_regs;

    /* Check if FMA instructions are available (CPUID feature check) */
    /* For now, use sequence: MUL + ADD */

    /* MOV Vd, Vn (copy multiplicand) */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* MUL Vd, Vm */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x59);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    /* ADD Vd, Va */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x58);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_ra);

    return 0;
}

/**
 * translate_fp_fms - Translate ARM64 FMSUB instruction
 * FMSUB Vd, Vn, Vm, Va  =>  Vd = Va - (Vn * Vm)
 */
int translate_fp_fms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = fp_get_rd(encoding);
    uint8_t rn = fp_get_rn(encoding);
    uint8_t rm = fp_get_rm(encoding);
    uint8_t ra = fp_get_ra(encoding);
    int size = fp_get_size(encoding);

    uint8_t xmm_rd = xmm_reg(rd);
    uint8_t xmm_rn = xmm_reg(rn);
    uint8_t xmm_rm = xmm_reg(rm);
    uint8_t xmm_ra = xmm_reg(ra);

    (void)vec_regs;

    /* MOV Vd, Vn */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* MUL Vd, Vm */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x59);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    /* SUB Vd = Va - Vd (reverse subtract) */
    code_buf_emit_byte(code_buf, 0xF2 + (size ? 0x00 : 0x01));
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x5C);  /* SUB */
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_ra << 3) + xmm_rd);

    /* MOV Vd, Va (result is now in Va, move to Vd) */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_ra);

    return 0;
}

/**
 * translate_fp_fnma - Translate ARM64 FNMADD instruction
 * FNMADD Vd, Vn, Vm, Va  =>  Vd = -(Va + (Vn * Vm))
 */
int translate_fp_fnma(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* FNMADD = negate(FMADD) */
    /* For now, emit FMADD then negate */
    int result = translate_fp_fma(encoding, code_buf, vec_regs);
    if (result == 0) {
        uint8_t rd = fp_get_rd(encoding);
        uint8_t xmm_rd = xmm_reg(rd);

        /* XOR to negate (flip sign bit) */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x57);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rd);
    }
    return result;
}

/**
 * translate_fp_fnms - Translate ARM64 FNMSUB instruction
 * FNMSUB Vd, Vn, Vm, Va  =>  Vd = -(Va - (Vn * Vm))
 */
int translate_fp_fnms(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* FNMSUB = negate(FMSUB) */
    int result = translate_fp_fms(encoding, code_buf, vec_regs);
    if (result == 0) {
        uint8_t rd = fp_get_rd(encoding);
        uint8_t xmm_rd = xmm_reg(rd);

        /* XOR to negate */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x57);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rd);
    }
    return result;
}

/* ============================================================================
 * FP Arithmetic Dispatch
 * ============================================================================ */

/**
 * translate_fp_arith_dispatch - Dispatch FP arithmetic instruction
 */
int translate_fp_arith_dispatch(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* Check if this is a floating-point instruction (opcode 0x1E for scalar) */
    uint8_t opcode = (encoding >> 24) & 0xFF;
    uint8_t op2 = (encoding >> 21) & 0x07;

    if (opcode != 0x1E) {
        return -1;  /* Not an FP instruction */
    }

    /* Dispatch based on operation type */
    switch (op2) {
        case 0x00:  /* FADD */
            return translate_fp_add(encoding, code_buf, vec_regs);
        case 0x01:  /* FCMP, FMAX, FMIN, etc. */
            /* Further dispatch needed */
            break;
        case 0x02:  /* FSUB */
            return translate_fp_sub(encoding, code_buf, vec_regs);
        case 0x03:  /* FMUL */
            return translate_fp_mul(encoding, code_buf, vec_regs);
        case 0x04:  /* FMADD, FMSUB, FNMADD, FNMSUB */
            /* Further dispatch based on op4 */
            break;
        case 0x05:  /* FDIV */
            return translate_fp_div(encoding, code_buf, vec_regs);
        case 0x06:  /* FMAX, FMIN */
            /* Further dispatch */
            break;
        default:
            break;
    }

    /* Not handled by arithmetic dispatch */
    return -1;
}
