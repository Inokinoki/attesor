/* ============================================================================
 * Rosetta Translator - FP Helper Functions
 * ============================================================================
 *
 * This module provides FP scalar helper functions for JIT compilation
 * including FABS, FNEG, and FCSEL implementations.
 * ============================================================================ */

#include "rosetta_jit_emit.h"
#include "rosetta_jit_emit_simd.h"
#include <stdint.h>

/* ============================================================================
 * FP Scalar Helper Functions
 * ============================================================================ */

/**
 * Emit FABS (Floating-point Absolute Value) for scalar
 * Flips sign bit using ANDPS/ANDPD with sign mask
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
void emit_fabs_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FABS, clear the sign bit:
     * Single-precision: AND with 0x7FFFFFFF (clear bit 31)
     * Double-precision: AND with 0x7FFFFFFFFFFFFFFF (clear bit 63)
     *
     * Approach: Use GPR to construct mask, then move to XMM, then ANDPS/ANDPD
     */

    uint8_t temp_xmm = 15;  /* Use XMM15 as temp (clobber safe in our model) */
    uint8_t temp_gpr = 0;   /* Use RAX/EAX as temp */

    /* For single precision: */
    if (!is_double) {
        /* MOV eax, 0x7FFFFFFF - immediate move to clear sign bit */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));  /* MOV r32, imm32 */
        if (temp_gpr >= 8) buf->buffer[buf->offset - 1] |= 0x04;
        emit_word32(buf, 0x7FFFFFFF);

        /* MOVD xmm15, eax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x00  ; broadcast to all lanes */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x00);  /* Immediate: shuffle control */

        /* ANDPS dst, xmm15 */
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    } else {
        /* Double precision: need 0x7FFFFFFFFFFFFFFF
         * MOV rax, 0x7FFFFFFFFFFFFFFF (via MOV + shift)
         */
        emit_byte(buf, 0x48 | ((temp_gpr & 8) >> 3));  /* REX.W */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));  /* MOV r64, imm64 */
        emit_word32(buf, 0xFFFFFFFF);  /* Low 32 bits */
        emit_word32(buf, 0x7FFFFFFF);  /* High 32 bits */

        /* MOVQ xmm15, rax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x48);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x44 - duplicate low 64 to high 64 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x44);  /* [1:0, 1:0] = duplicate lane 1 to both */

        /* ANDPD dst, xmm15 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    }
}

/**
 * Emit FNEG (Floating-point Negate) for scalar
 * Flips sign bit using XORPS/XORPD with sign mask
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register (0-15)
 * @param is_double 1 for double-precision, 0 for single-precision
 */
void emit_fneg_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FNEG, flip the sign bit:
     * Single-precision: XOR with 0x80000000 (set bit 31)
     * Double-precision: XOR with 0x8000000000000000 (set bit 63)
     *
     * Same approach as FABS but using XOR instead of AND
     */

    uint8_t temp_xmm = 15;
    uint8_t temp_gpr = 0;

    if (!is_double) {
        /* MOV eax, 0x80000000 */
        emit_byte(buf, 0xB8 + (temp_gpr & 7));
        if (temp_gpr >= 8) buf->buffer[buf->offset - 1] |= 0x04;
        emit_word32(buf, 0x80000000);

        /* MOVD xmm15, eax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x00 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x00);

        /* XORPS dst, xmm15 */
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    } else {
        /* MOV rax, 0x8000000000000000 */
        emit_byte(buf, 0x48 | ((temp_gpr & 8) >> 3));
        emit_byte(buf, 0xB8 + (temp_gpr & 7));
        emit_word32(buf, 0x00000000);
        emit_word32(buf, 0x80000000);

        /* MOVQ xmm15, rax */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x48);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x6E);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_gpr & 7) << 3));

        /* PSHUFD xmm15, xmm15, 0x44 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x70);
        emit_byte(buf, 0xC0 + (temp_xmm & 7) + ((temp_xmm & 7) << 3));
        emit_byte(buf, 0x44);

        /* XORPD dst, xmm15 */
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((temp_xmm & 7) << 3));
    }
}

/**
 * Emit FCSEL (Floating-point Conditional Select) for scalar
 * Implements: dst = (cond true) ? src : src2
 * @param buf Code buffer
 * @param dst Destination XMM register (0-15)
 * @param src Source XMM register for TRUE condition (0-15)
 * @param src2 Source XMM register for FALSE condition (0-15)
 * @param cond ARM64 condition code (0-15)
 */
void emit_fcsel_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t src2, uint8_t cond)
{
    /* FCSEL: dst = (flags match cond) ? src : src2
     *
     * x86 doesn't have direct XMM conditional moves, but we can use:
     * 1. Conditional branches (slow)
     * 2. CMOV for GPRs with register spills
     * 3. BLENDPS/BLENDPD (SSE4.1) - requires SSE4.1
     *
     * Using conditional branch approach:
     *   movsd dst, src       ; assume true
     *   j<cond> skip         ; if condition true, skip
     *   movsd dst, src2      ; else load false value
     * skip:
     */

    /* Copy src (true value) to dst */
    emit_movsd_xmm_xmm(buf, dst, src);

    /* Map ARM64 condition to x86 condition for "jump if false"
     * We want to jump (skip the false load) if the condition is TRUE
     * So we use the same condition mapping as regular branches
     */

    /* Emit conditional jump to skip the false load
     * The condition is already in EFLAGS from preceding FCMP
     */
    uint8_t *jump_patch = &buf->buffer[buf->offset];
    emit_byte(buf, 0x0F);  /* Prefix for conditional jumps */

    /* Map ARM64 condition to x86 Jcc */
    uint8_t x86_cond = cond;
    switch (cond) {
        case 0:  /* EQ */ x86_cond = 0x4; break;  /* JE */
        case 1:  /* NE */ x86_cond = 0x5; break;  /* JNE */
        case 2:  /* CS/HS */ x86_cond = 0x3; break;  /* JAE */
        case 3:  /* CC */ x86_cond = 0x2; break;  /* JB */
        case 4:  /* MI */ x86_cond = 0x8; break;  /* JS */
        case 5:  /* PL */ x86_cond = 0x9; break;  /* JNS */
        case 6:  /* VS */ x86_cond = 0x6; break;  /* JO */
        case 7:  /* VC */ x86_cond = 0x7; break;  /* JNO */
        case 8:  /* HI */ x86_cond = 0x7; break;  /* JA */
        case 9:  /* LS */ x86_cond = 0x6; break;  /* JBE */
        case 10: /* GE */ x86_cond = 0x9; break;  /* JGE */
        case 11: /* LT */ x86_cond = 0xC; break;  /* JL */
        case 12: /* GT */ x86_cond = 0xF; break;  /* JG */
        case 13: /* LE */ x86_cond = 0xE; break;  /* JLE */
        case 14: /* AL */ x86_cond = 0xFF; break; /* Always - skip jump */
        case 15: /* NV */ x86_cond = 0xFF; break; /* Never - always jump */
    }

    if (x86_cond == 0xFF) {
        /* AL: always true - no jump needed, dst already has src */
        /* NV: never true - always load src2 */
        if (cond == 15) {
            emit_movsd_xmm_xmm(buf, dst, src2);
        }
        (void)jump_patch;
        return;
    }

    emit_byte(buf, 0x80 | x86_cond);  /* Jcc rel32 */
    emit_word32(buf, 0);  /* Placeholder offset - will be patched */

    /* Emit MOVSD dst, src2 (load false value) */
    emit_movsd_xmm_xmm(buf, dst, src2);

    /* Calculate and patch jump offset (skip over MOVSD = 4 bytes) */
    int32_t skip_offset = (int32_t)(buf->offset - (jump_patch + 6 - buf->buffer));

    /* Patch the jump offset */
    jump_patch[2] = skip_offset & 0xFF;
    jump_patch[3] = (skip_offset >> 8) & 0xFF;
    jump_patch[4] = (skip_offset >> 16) & 0xFF;
    jump_patch[5] = (skip_offset >> 24) & 0xFF;
}

/* ============================================================================
 * FP Estimate Functions
 * ============================================================================ */

/**
 * fp_recip_estimate - Floating-point Reciprocal Estimate (FRECPE)
 * @value: Input float value
 * Returns: Estimated 1.0/value
 *
 * Implements ARM64 FRECPE instruction emulation using a simplified
 * exponent-based approximation.
 */
float fp_recip_estimate(float value)
{
    uint32_t x = *(uint32_t *)&value;
    uint32_t sign, exp, mant;
    int32_t exp_val;
    uint32_t result;

    /* Extract sign, exponent, and mantissa */
    sign = (x >> 31) & 0x00000001;
    exp = (x >> 23) & 0x000000FF;
    mant = x & 0x007FFFFF;

    /* Handle special cases */
    if (exp == 0) {
        /* Input is zero or subnormal - return infinity */
        result = (sign << 31) | 0x7F800000;
        return *(float *)&result;
    }
    if (exp == 0xFF) {
        if (mant == 0) {
            /* Input is infinity - return zero */
            result = (sign << 31);
            return *(float *)&result;
        }
        /* Input is NaN - return NaN */
        return value;
    }

    /* Compute reciprocal exponent: new_exp = 254 - old_exp */
    exp_val = 254 - (int32_t)exp;

    /* Approximate mantissa reciprocal using linear approximation */
    /* For 1.0 <= m < 2.0, approximate 1/m using: 1.75 - 0.75*m */
    /* This is a simplification - FRECPE has specific accuracy bounds */
    if (mant < 0x00400000) {
        /* mant < 0.5: result mantissa is in [1.0, 2.0) */
        mant = 0x00800000 - mant;
    } else {
        /* mant >= 0.5: result mantissa is in [0.5, 1.0) */
        mant = 0x00C00000 - (mant << 1);
        exp_val--;
    }

    /* Handle exponent overflow/underflow */
    if (exp_val <= 0) {
        /* Underflow - return zero */
        result = (sign << 31);
        return *(float *)&result;
    }
    if (exp_val >= 255) {
        /* Overflow - return infinity */
        result = (sign << 31) | 0x7F800000;
        return *(float *)&result;
    }

    /* Construct result */
    result = (sign << 31) | ((uint32_t)exp_val << 23) | (mant & 0x007FFFFF);

    return *(float *)&result;
}

/**
 * fp_rsqrt_estimate - Floating-point Reciprocal Square Root Estimate (FRSQRTE)
 * @value: Input float value
 * Returns: Estimated 1.0/sqrt(value)
 *
 * Implements ARM64 FRSQRTE instruction emulation.
 * Returns 1/sqrt(2) approximation for normalized inputs.
 */
float fp_rsqrt_estimate(float value)
{
    uint32_t x = *(uint32_t *)&value;
    uint32_t exp, mant;
    int32_t exp_val;
    uint32_t result;

    /* Extract exponent and mantissa (sign is ignored for rsqrt) */
    exp = (x >> 23) & 0x000000FF;
    mant = x & 0x007FFFFF;

    /* Handle special cases */
    if (exp == 0) {
        if (mant == 0) {
            /* Input is zero - return infinity */
            result = 0x7F800000;
            return *(float *)&result;
        }
        /* Subnormal - treat as zero */
        result = 0x7F800000;
        return *(float *)&result;
    }
    if (exp == 0xFF) {
        if (mant == 0) {
            /* Input is infinity - return zero */
            result = 0x00000000;
            return *(float *)&result;
        }
        /* Input is NaN - return NaN */
        return value;
    }

    /* Compute rsqrt exponent: new_exp = (381 - old_exp) / 2 */
    /* This gives: exp(1/sqrt(x)) = (254 - exp(x)) / 2 + 63.5 */
    exp_val = (381 - (int32_t)exp) / 2;

    /* Approximate mantissa for 1/sqrt(m) where 1.0 <= m < 2.0 */
    /* Using linear approximation: 1.5 - 0.5*m */
    /* The magic constant 0x5A827999 is approximately 1/sqrt(2) */
    if (mant < 0x00400000) {
        mant = 0x007FFFFF - (mant >> 1);
    } else {
        /* Use precomputed approximation for 1/sqrt(m) */
        mant = 0x005A8279 - ((mant * 0x002D0000) >> 24);
        exp_val--;
    }

    /* Handle exponent overflow/underflow */
    if (exp_val <= 0) {
        result = 0x00000000;
        return *(float *)&result;
    }
    if (exp_val >= 255) {
        result = 0x7F800000;
        return *(float *)&result;
    }

    /* Construct result (sign is always positive) */
    result = ((uint32_t)exp_val << 23) | (mant & 0x007FFFFF);

    return *(float *)&result;
}
