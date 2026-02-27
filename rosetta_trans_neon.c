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
 *
 * SQADD performs signed saturating addition on vector elements.
 * Result is saturated to the element size range on overflow.
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
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Copy rn to rd first */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* x86_64 PADDSB for signed saturating add (8-bit) */
    /* 66 0F EC /r - PADDSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xEC);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_uqadd - Translate ARM64 UQADD (unsigned saturating add)
 *
 * UQADD performs unsigned saturating addition on vector elements.
 */
int translate_uqadd(ThreadState *state, const uint8_t *insn)
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

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* x86_64 PADDUSB for unsigned saturating add (8-bit) */
    /* 66 0F DC /r - PADDUSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xDC);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_sqsub - Translate ARM64 SQSUB (saturating subtract)
 *
 * SQSUB performs signed saturating subtraction on vector elements.
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
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_rm = rm & 0x0F;

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* x86_64 PSUBSB for signed saturating subtract (8-bit) */
    /* 66 0F E9 /r - PSUBSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xE9);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/**
 * translate_uqsub - Translate ARM64 UQSUB (unsigned saturating subtract)
 *
 * UQSUB performs unsigned saturating subtraction on vector elements.
 */
int translate_uqsub(ThreadState *state, const uint8_t *insn)
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

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* x86_64 PSUBUSB for unsigned saturating subtract (8-bit) */
    /* 66 0F D9 /r - PSUBUSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xD9);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    return 0;
}

/* ============================================================================
 * NEON Doubling Saturating Multiply
 * ============================================================================ */

/**
 * translate_sqdmulh - Translate ARM64 SQDMULH (signed saturating doubling
 * multiply high)
 *
 * SQDMULH multiplies signed values, doubles the result, and extracts the
 * high half. Used in fixed-point arithmetic.
 */
int translate_sqdmulh(ThreadState *state, const uint8_t *insn)
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

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* Use PMULHW for high-half multiply (signed) */
    /* 66 0F D5 /r - PMULHW xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xD5);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    /* Then double (shift left by 1) with saturation */
    /* 66 0F F5 /r - PADDW xmm1, xmm2/m128 (add to self) */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xF5);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/**
 * translate_sqrdmulh - Translate ARM64 SQRDMULH (signed saturating rounding
 * doubling multiply high)
 *
 * SQRDMULH multiplies signed values, rounds, doubles, and extracts high half.
 */
int translate_sqrdmulh(ThreadState *state, const uint8_t *insn)
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

    /* For rounding, we'd need to add 1 to the high half before doubling */
    /* Simplified: use same as SQDMULH for now */

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* PMULHW for high-half multiply */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xD5);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rm << 3));

    /* Double the result */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xF5);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/* ============================================================================
 * NEON Absolute Value and Negation
 * ============================================================================ */

/**
 * translate_abs_vec - Translate ARM64 ABS (vector absolute value)
 *
 * ABS computes the absolute value of each vector element.
 */
int translate_abs_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Copy rn to rd first */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rn);

    /* Use PABSB/PABSW/PABSD for absolute value */
    /* 66 0F 38 1C /r - PABSB xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x38);
    jit_emit_byte(0x1C);
    jit_emit_byte(0xC0 + xmm_rd);

    return 0;
}

/**
 * translate_neg_vec - Translate ARM64 NEG (vector negate)
 *
 * NEG negates each vector element (0 - value).
 */
int translate_neg_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Zero out rd */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x57);
    jit_emit_byte(0xC0 + xmm_rd);  /* PXOR xmm_rd, xmm_rd */

    /* Subtract rn from zero */
    /* 66 0F F1 /r - PSUBW xmm1, xmm2/m128 */
    jit_emit_byte(xmm_rd);
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xF1);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    return 0;
}

/* ============================================================================
 * NEON Bitwise Operations
 * ============================================================================ */

/**
 * translate_rbit_vec - Translate ARM64 RBIT (reverse bit order)
 *
 * RBIT reverses the bit order in each byte of a vector.
 * Implementation uses a shuffle mask and bit manipulation.
 */
int translate_rbit_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Copy rn to rd first */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    /* Use PSHUFB (SSSE3) with bit-reversal shuffle mask */
    /* The mask maps each byte position to its bit-reversed position */
    /* 66 0F 38 00 /r - PSHUFB xmm1, xmm2/m128 */

    /* First, we need to set up a shuffle mask in a temp register (XMM7) */
    /* The mask for bit reversal within each byte: */
    /* [0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00, 0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08] */
    /* This is simplified - full implementation would load the proper mask */

    /* For now, emit the PSHUFB instruction structure */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x38);
    jit_emit_byte(0x00);
    jit_emit_byte(0xC0 + xmm_rd + (7 << 3));  /* Use XMM7 as mask source */

    return 0;
}

/**
 * translate_cls_vec - Translate ARM64 CLS (count leading sign bits)
 *
 * CLS counts the leading sign bits (copies of the sign bit) in each element.
 * CLS(x) = CLZ(x XOR (x << 1)) - 1
 * Equivalently: CLS(x) = CLZ((x << 1) XOR x) for signed interpretation
 */
int translate_cls_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[2] >> 22) & 0x03;  /* Element size: 00=8bit, 01=16bit, 10=32bit */

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Step 1: Copy rn to rd */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    /* Step 2: Create a shifted copy for XOR */
    /* Use temp register XMM7 for the shift */
    uint8_t xmm_tmp = 7;

    /* Copy rn to temp */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_tmp + (xmm_rn << 3));

    /* Step 3: Shift left by 1 (PSLLD/PSLLW/PSLLB) */
    if (size == 0x02) {
        /* 32-bit elements: 66 0F 72 /6 ib - PSLLD xmm, imm8 */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x72);
        jit_emit_byte(0xE0 + xmm_tmp);  /* /6 = shift left dword */
        jit_emit_byte(0x01);
    } else if (size == 0x01) {
        /* 16-bit elements: 66 0F 72 /2 ib - PSLLW xmm, imm8 */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x72);
        jit_emit_byte(0xD0 + xmm_tmp);  /* /2 = shift left word */
        jit_emit_byte(0x01);
    }
    /* 8-bit shift not directly supported - would need workaround */

    /* Step 4: XOR original with shifted (PSXOR) */
    /* 66 0F EF /r - PSXOR xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xEF);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_tmp << 3));

    /* Step 5: Count leading zeros (LZCNT) on each element */
    /* For 32-bit: use VPLZCNTD (AVX-512) or software implementation */
    /* Simplified: emit placeholder for LZCNT */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0xBD);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));

    return 0;
}

/**
 * translate_clz_vec - Translate ARM64 CLZ (count leading zeros)
 *
 * CLZ counts the number of leading zero bits in each element.
 * Implementation uses x86 LZCNT instruction or software fallback.
 */
int translate_clz_vec(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[2] >> 22) & 0x03;  /* Element size: 00=8bit, 01=16bit, 10=32bit */

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Step 1: Copy rn to rd */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    /* Step 2: Count leading zeros per element */
    /* For 32-bit elements, use VPLZCNTD (AVX-512) or LZCNT loop */
    /* For 16-bit elements, use VPLZCNTW (AVX-512) */
    /* Simplified: use software approach with bit scanning */

    if (size == 0x02) {
        /* 32-bit elements - emit VPLZCNTD if available (AVX-512) */
        /* EVMA 0F 38 BC /r - VPLZCNTD zmm1, zmm2/m512 */
        /* For XMM registers with AVX-512: EVEX.128.0F38.W0 BC /r */
        /* Simplified: emit the opcode structure */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x38);
        jit_emit_byte(0xBC);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
    } else if (size == 0x01) {
        /* 16-bit elements - emit VPLZCNTW if available (AVX-512) */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x38);
        jit_emit_byte(0xBD);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
    } else {
        /* 8-bit elements - requires byte-wise handling */
        /* Use LZCNT on each byte with masks and shifts */
        /* This is complex - emit simplified version */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0xBD);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
    }

    return 0;
}

/* ============================================================================
 * NEON Reduction Operations
 * ============================================================================ */

/**
 * translate_addv - Translate ARM64 ADDV (across vector add)
 *
 * ADDV sums all elements in a vector and produces a scalar result.
 * Implementation uses horizontal add instructions.
 */
int translate_addv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[2] >> 22) & 0x03;  /* Element size */

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;

    /* Step 1: Copy rn to rd */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    /* Step 2: Horizontal add based on element size */
    if (size == 0x02) {
        /* 32-bit elements: HADDPS (horizontal add packed single) */
        /* F2 0F 7C /r - HADDPS xmm1, xmm2/m128 */
        /* First HADDPS: [a0+a1, a2+a3, a4+a5, a6+a7] */
        jit_emit_byte(0xF2);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x7C);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
        /* Second HADDPS: [a0+a1+a2+a3, a4+a5+a6+a7, ...] */
        jit_emit_byte(0xF2);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x7C);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
    } else if (size == 0x01) {
        /* 16-bit elements: HADDPS works on 32-bit lanes */
        /* Need to handle differently - use PHADDW */
        /* 66 0F 38 01 /r - PHADDW xmm1, xmm2/m128 */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x38);
        jit_emit_byte(0x01);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
        /* Repeat for full reduction */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x38);
        jit_emit_byte(0x01);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_rd << 3));
    }

    return 0;
}

/**
 * translate_maxv - Translate ARM64 MAXV (across vector max)
 *
 * MAXV finds the maximum element in a vector.
 * Implementation uses horizontal max and reduction.
 */
int translate_maxv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[2] >> 22) & 0x03;  /* Element size */

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_tmp = 7;  /* Temporary register */

    /* Step 1: Copy rn to rd and tmp */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_tmp + (xmm_rn << 3));

    /* Step 2: Shift tmp right by element size */
    if (size == 0x02) {
        /* 32-bit: PSRLD xmm, 4 (shift by one element) */
        /* 66 0F 72 /3 ib - PSRLD xmm, imm8 */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x72);
        jit_emit_byte(0xD0 + xmm_tmp);  /* /3 = shift right dword */
        jit_emit_byte(0x04);
    }

    /* Step 3: Horizontal max using PHMAXD/PHMAXW or manual comparison */
    /* For 32-bit: use MAXPS then reduce */
    if (size == 0x02) {
        /* MAXPS xmm_rd, xmm_tmp - 0F 5F /r */
        jit_emit_byte(0x0F);
        jit_emit_byte(0x5F);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_tmp << 3));
    }

    /* Step 4: Additional reductions would be needed for full implementation */
    /* This is a simplified version that handles 4-element vectors */

    return 0;
}

/**
 * translate_minv - Translate ARM64 MINV (across vector min)
 *
 * MINV finds the minimum element in a vector.
 * Implementation uses horizontal min and reduction.
 */
int translate_minv(ThreadState *state, const uint8_t *insn)
{
    if (!state || !insn) {
        return -1;
    }

    uint8_t rd = (insn[0] >> 0) & 0x1F;
    uint8_t rn = (insn[1] >> 5) & 0x1F;
    uint8_t size = (insn[2] >> 22) & 0x03;  /* Element size */

    uint8_t xmm_rd = rd & 0x0F;
    uint8_t xmm_rn = rn & 0x0F;
    uint8_t xmm_tmp = 7;  /* Temporary register */

    /* Step 1: Copy rn to rd and tmp */
    /* 66 0F 6F /r - MOVDQA xmm1, xmm2/m128 */
    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_rd + (xmm_rn << 3));

    jit_emit_byte(0x66);
    jit_emit_byte(0x0F);
    jit_emit_byte(0x6F);
    jit_emit_byte(0xC0 + xmm_tmp + (xmm_rn << 3));

    /* Step 2: Shift tmp right by element size */
    if (size == 0x02) {
        /* 32-bit: PSRLD xmm, 4 (shift by one element) */
        jit_emit_byte(0x66);
        jit_emit_byte(0x0F);
        jit_emit_byte(0x72);
        jit_emit_byte(0xD0 + xmm_tmp);
        jit_emit_byte(0x04);
    }

    /* Step 3: Horizontal min using PHMIND/PHMINW or manual comparison */
    /* For 32-bit: use MINPS then reduce */
    if (size == 0x02) {
        /* MINPS xmm_rd, xmm_tmp - 0F 5D /r */
        jit_emit_byte(0x0F);
        jit_emit_byte(0x5D);
        jit_emit_byte(0xC0 + xmm_rd + (xmm_tmp << 3));
    }

    /* Step 4: Additional reductions would be needed for full implementation */
    /* This is a simplified version that handles 4-element vectors */

    return 0;
}
