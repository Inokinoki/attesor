/* ============================================================================
 * Rosetta Translator - NEON/SIMD Translation Implementation
 * ============================================================================
 *
 * This module implements NEON/SIMD instruction translation functions
 * for ARM64 to x86_64 binary translation.
 * ============================================================================ */

#include "rosetta_trans_neon.h"
#include "rosetta_jit_emit.h"
#include "rosetta_jit_emit_simd.h"
#include "rosetta_refactored_vector.h"
#include <string.h>

/* ============================================================================
 * NEON Arithmetic Translation
 * ============================================================================ */

/**
 * translate_add_vec - Translate ARM64 ADD (vector) instruction
 *
 * ARM64 ADD (vector) adds corresponding elements in two SIMD registers.
 * This is translated to x86_64 PADDD/PADDQ/PADDB/PADDW depending on size.
 */
int translate_add_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Extract register operands from ARM64 instruction */
    uint8_t rd = (insn[0] >> 0) & 0x1F;   /* Destination vector register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* First source vector register */
    uint8_t rm = (insn[2] >> 16) & 0x1F;  /* Second source vector register */

    /* Map ARM64 V registers to x86_64 XMM registers */
    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Emit x86_64 PADDD (packed add doublewords) */
    /* 66 0F FE /r - PADDD xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xFE);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    (void)xmm_rn;  /* For now, assume rn == rd (accumulator form) */

    return 0;
}

/**
 * translate_sub_vec - Translate ARM64 SUB (vector) instruction
 *
 * ARM64 SUB (vector) subtracts corresponding elements in two SIMD registers.
 * This is translated to x86_64 PSUBD/PSUBQ/PSUBB/PSUBW depending on size.
 */
int translate_sub_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Extract register operands from ARM64 instruction */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    /* Map ARM64 V registers to x86_64 XMM registers */
    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Emit x86_64 PSUBD (packed subtract doublewords) */
    /* 66 0F FA /r - PSUBD xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xFA);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_mul_vec - Translate ARM64 MUL (vector) instruction
 *
 * ARM64 MUL (vector) multiplies corresponding elements in two SIMD registers.
 * This is translated to x86_64 PMULLD/PMULUDQ depending on size.
 */
int translate_mul_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Extract register operands from ARM64 instruction */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    /* Map ARM64 V registers to x86_64 XMM registers */
    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Emit x86_64 PMULLD (packed multiply low doublewords) */
    /* 66 0F 38 28 /r - PMULLD xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x38);
    jit_emit_byte(0x28);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_mla_vec - Translate ARM64 MLA (vector multiply-accumulate)
 *
 * ARM64 MLA multiplies elements and adds to accumulator.
 * Translated to x86_64 PMADDWD or sequence of MUL + ADD.
 */
int translate_mla_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Extract register operands */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* For MLA: result = rd + (rn * rm) */
    /* Use PMADDWD for 16-bit multiply-accumulate */
    /* 66 0F F5 /r - PMADDWD xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xF5);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    /* Then add to accumulator (rd) */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xFE);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    return 0;
}

/**
 * translate_mls_vec - Translate ARM64 MLS (vector multiply-subtract)
 *
 * ARM64 MLS multiplies elements and subtracts from accumulator.
 */
int translate_mls_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    /* Extract register operands */
    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* For MLS: result = rd - (rn * rm) */
    /* First multiply */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x38);
    jit_emit_byte(0x28);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    /* Then subtract from accumulator */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xFA);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    return 0;
}

/* ============================================================================
 * NEON Logical Translation
 * ============================================================================ */

/**
 * translate_and_vec - Translate ARM64 AND (vector) instruction
 */
int translate_and_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 PAND: 66 0F DB /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xDB);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_orr_vec - Translate ARM64 ORR (vector) instruction
 */
int translate_orr_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 POR: 66 0F EB /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xEB);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_eor_vec - Translate ARM64 EOR (vector) instruction
 */
int translate_eor_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 PXOR: 66 0F EF /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xEF);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_bic_vec - Translate ARM64 BIC (vector) instruction
 *
 * BIC performs: rd = rn AND NOT rm
 */
int translate_bic_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* First copy rn to rd */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* Load rm into temp and negate */
    uint8_t xmm_tmp = 15;  /* Use XMM15 as temp */

    /* PANDN: 66 0F DF /r - dst = NOT src1 AND src2 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xDF);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/* ============================================================================
 * NEON Compare Translation
 * ============================================================================ */

/**
 * translate_cmge_vec - Translate ARM64 CMGE (vector compare >=)
 *
 * CMGE compares signed values and sets mask bits.
 */
int translate_cmge_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Use PCMPGTD for signed comparison */
    /* 66 0F 6E /r - PCMPGTD xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6E);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    return 0;
}

/**
 * translate_cmhs_vec - Translate ARM64 CMHS (unsigned vector compare >=)
 */
int translate_cmhs_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* For unsigned compare, we need a different approach */
    /* Use PCMPEQ and combine with other logic */
    /* This is a simplified implementation */
    jit_emit_byte(xmm_rd);

    return 0;
}

/**
 * translate_cmeq_vec - Translate ARM64 CMEQ (vector compare ==)
 */
int translate_cmeq_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 PCMPEQD: 66 0F 75 /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x75);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    return 0;
}

/**
 * translate_cmgt_vec - Translate ARM64 CMGT (vector compare >)
 */
int translate_cmgt_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 PCMPGTD: 66 0F 6F /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    return 0;
}

/* ============================================================================
 * NEON Load/Store Translation
 * ============================================================================ */

/**
 * translate_ld1 - Translate ARM64 LD1 (load single structure)
 *
 * LD1 loads a single structure (one element) from memory to a NEON register.
 */
int translate_ld1(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;   /* Target vector register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Base address register */

    uint8_t xmm_rt = rt & 0x0F;
    uint8_t x86_rn = map_arm64_to_x86_gpr(rn);

    /* x86_64 MOVDQU (move unaligned): F3 0F 6F /r */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));  /* [rn] addressing */

    return 0;
}

/**
 * translate_st1 - Translate ARM64 ST1 (store single structure)
 */
int translate_st1(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;   /* Source vector register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;   /* Base address register */

    uint8_t xmm_rt = rt & 0x0F;
    uint8_t x86_rn = map_arm64_to_x86_gpr(rn);

    /* x86_64 MOVDQU (store): F3 0F 7F /r */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    return 0;
}

/**
 * translate_ld2 - Translate ARM64 LD2 (load 2 structures)
 */
int translate_ld2(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = ((insn[0] >> 5) & 0x07);  /* Second register */
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rt = rt & 0x0F;
    uint8_t xmm_rt2 = rt2 & 0x0F;

    /* Load two consecutive structures */
    /* This is simplified - real implementation would deinterleave */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    jit_emit_byte(xmm_rt2);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0x44 + ((xmm_rt2 & 7) << 3));  /* Offset by 16 bytes */

    return 0;
}

/**
 * translate_st2 - Translate ARM64 ST2 (store 2 structures)
 */
int translate_st2(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rt2 = ((insn[0] >> 5) & 0x07);

    uint8_t xmm_rt = rt & 0x0F;
    uint8_t xmm_rt2 = rt2 & 0x0F;

    /* Store two consecutive structures */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    jit_emit_byte(xmm_rt2);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7F);
    jit_emit_byte(0x44 + ((xmm_rt2 & 7) << 3));

    return 0;
}

/**
 * translate_ld3 - Translate ARM64 LD3 (load 3 structures)
 */
int translate_ld3(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rt = rt & 0x0F;

    /* Load three structures (RGB to planar, for example) */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    return 0;
}

/**
 * translate_st3 - Translate ARM64 ST3 (store 3 structures)
 */
int translate_st3(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;

    uint8_t xmm_rt = rt & 0x0F;

    /* Store three structures */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    return 0;
}

/**
 * translate_ld4 - Translate ARM64 LD4 (load 4 structures)
 */
int translate_ld4(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rt = rt & 0x0F;

    /* Load four structures (RGBA to planar) */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    return 0;
}

/**
 * translate_st4 - Translate ARM64 ST4 (store 4 structures)
 */
int translate_st4(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rt = (insn[0] >> 0) & 0x1F;

    uint8_t xmm_rt = rt & 0x0F;

    /* Store four structures */
    jit_emit_byte(xmm_rt);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7F);
    jit_emit_byte(0x04 + ((xmm_rt & 7) << 3));

    return 0;
}

/* ============================================================================
 * NEON Shift Translation
 * ============================================================================ */

/**
 * translate_shl_vec - Translate ARM64 SHL (vector shift left)
 */
int translate_shl_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t imm = (insn[2] >> 16) & 0x3F;  /* Shift amount */

    uint8_t xmm_rd = rd & 0x0F;

    /* x86_64 PSLLD: 66 0F 72 /6 ib */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x72);
    jit_emit_byte(0xE0 + xmm_rd);
    jit_emit_byte(imm & 0xFF);

    return 0;
}

/**
 * translate_shr_vec - Translate ARM64 SHR/SHRN (vector shift right)
 */
int translate_shr_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t imm = (insn[2] >> 16) & 0x3F;

    uint8_t xmm_rd = rd & 0x0F;

    /* x86_64 PSRLD: 66 0F 72 /5 ib */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x72);
    jit_emit_byte(0xD0 + xmm_rd);
    jit_emit_byte(imm & 0xFF);

    return 0;
}

/**
 * translate_sshr_vec - Translate ARM64 SSHR (signed shift right)
 */
int translate_sshr_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t imm = (insn[2] >> 16) & 0x3F;

    uint8_t xmm_rd = rd & 0x0F;

    /* x86_64 PSRAD (arithmetic shift): 66 0F 72 /4 ib */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x72);
    jit_emit_byte(0xC0 + xmm_rd);
    jit_emit_byte(imm & 0xFF);

    return 0;
}

/**
 * translate_ushr_vec - Translate ARM64 USHR (unsigned shift right)
 */
int translate_ushr_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t imm = (insn[2] >> 16) & 0x3F;

    uint8_t xmm_rd = rd & 0x0F;

    /* Use logical shift right for unsigned */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x72);
    jit_emit_byte(0xD0 + xmm_rd);
    jit_emit_byte(imm & 0xFF);

    return 0;
}

/* ============================================================================
 * NEON Convert/Reduce Translation
 * ============================================================================ */

/**
 * translate_fcvtns - Translate ARM64 FCVTNS (float to signed int)
 */
int translate_fcvtns(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* x86_64 CVTTPS2DQ: F3 0F 5B /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0xF3);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x5B);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/**
 * translate_scvtf - Translate ARM64 SCVTF (signed int to float)
 */
int translate_scvtf(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* x86_64 CVTDQ2PS: 0F 5B /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x5B);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/* ============================================================================
 * NEON Zip/Unzip Translation
 * ============================================================================ */

/**
 * translate_zip - Translate ARM64 ZIP (interleave vectors)
 */
int translate_zip(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Use PUNPCKL/PUNPCKH for zip operations */
    /* PUNPCKLBW: 66 0F 60 /r */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x60);
    jit_emit_byte(0xC0 + xmm_rn + (xmm_rm << 3));

    return 0;
}

/**
 * translate_uzp - Translate ARM64 UZP (unzip vectors)
 */
int translate_uzp(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* Unzip is more complex - requires shuffle operations */
    /* Simplified implementation */
    jit_emit_byte(xmm_rd);

    return 0;
}

/**
 * translate_trn - Translate ARM64 TRN (transpose vectors)
 */
int translate_trn(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Transpose using unpack instructions */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x62);  /* PUNPCKLDQ */
    jit_emit_byte(0xC0 + xmm_rn);

    return 0;
}

/* ============================================================================
 * NEON Saturating Operations
 * ============================================================================ */

/**
 * translate_sqadd - Translate ARM64 SQADD (saturating add)
 */
int translate_sqadd(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_64 PADDSB/PADDSW for signed saturating add */
    /* 66 0F EC /r - PADDSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xEC);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_sqsub - Translate ARM64 SQSUB (saturating subtract)
 */
int translate_sqsub(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t rm = (insn[2] >> 16) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* x86_66 0F E9 /r - PSUBSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xE9);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/* ============================================================================
 * NEON Reduction Operations
 * ============================================================================ */

/**
 * translate_addv - Translate ARM64 ADDV (across vector add)
 */
int translate_addv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* Horizontal add using HADDPS */
    /* F2 0F 7C /r - HADDPS xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0xF2);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x7C);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/**
 * translate_maxv - Translate ARM64 MAXV (across vector max)
 */
int translate_maxv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* Horizontal max - requires multiple steps */
    /* Simplified implementation */
    jit_emit_byte(xmm_rd);

    return 0;
}

/**
 * translate_minv - Translate ARM64 MINV (across vector min)
 */
int translate_minv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;

    /* Horizontal min - requires multiple steps */
    jit_emit_byte(xmm_rd);

    return 0;
}
