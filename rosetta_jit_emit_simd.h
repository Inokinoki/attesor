/* ============================================================================
 * Rosetta Translator - JIT SIMD/FP Code Emitter
 * ============================================================================
 *
 * This module provides functions for emitting x86_64 SIMD/FP instructions
 * for JIT compilation.
 * ============================================================================ */

#ifndef ROSETTA_JIT_EMIT_SIMD_H
#define ROSETTA_JIT_EMIT_SIMD_H

#include "rosetta_refactored_types.h"

/* ============================================================================
 * x86_64 FP/SIMD Emit - Scalar Operations
 * ============================================================================ */

/**
 * Emit MOVSS xmm1, xmm2 (Move Scalar Single-Precision)
 */
void emit_movss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit MOVSD xmm1, xmm2 (Move Scalar Double-Precision)
 */
void emit_movsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit ADDSS xmm1, xmm2 (Add Scalar Single-Precision)
 */
void emit_addss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit ADDSD xmm1, xmm2 (Add Scalar Double-Precision)
 */
void emit_addsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit SUBSS xmm1, xmm2 (Subtract Scalar Single-Precision)
 */
void emit_subss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit SUBSD xmm1, xmm2 (Subtract Scalar Double-Precision)
 */
void emit_subsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit MULSS xmm1, xmm2 (Multiply Scalar Single-Precision)
 */
void emit_mulss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit MULSD xmm1, xmm2 (Multiply Scalar Double-Precision)
 */
void emit_mulsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit DIVSS xmm1, xmm2 (Divide Scalar Single-Precision)
 */
void emit_divss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit DIVSD xmm1, xmm2 (Divide Scalar Double-Precision)
 */
void emit_divsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit SQRTSS xmm1 (Square Root Scalar Single-Precision)
 */
void emit_sqrtss_xmm(CodeBuffer *buf, uint8_t dst);

/**
 * Emit SQRSD xmm1 (Square Root Scalar Double-Precision)
 */
void emit_sqrtsd_xmm(CodeBuffer *buf, uint8_t dst);

/**
 * Emit UCOMISS xmm1, xmm2 (Unordered Compare Scalar Single-Precision)
 */
void emit_ucomiss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit UCOMISD xmm1, xmm2 (Unordered Compare Scalar Double-Precision)
 */
void emit_ucomisd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit ABSPS xmm1 (Absolute Value Packed Single-Precision)
 */
void emit_absps_xmm(CodeBuffer *buf, uint8_t dst);

/**
 * Emit ABSPD xmm1 (Absolute Value Packed Double-Precision)
 */
void emit_abspd_xmm(CodeBuffer *buf, uint8_t dst);

/**
 * Emit XORPS xmm1, xmm2 (XOR Packed Single-Precision)
 */
void emit_xorps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit XORPD xmm1, xmm2 (XOR Packed Double-Precision)
 */
void emit_xorpd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit CVTSS2SD xmm1, xmm2 (Convert Scalar Single to Double)
 */
void emit_cvtss2sd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit CVTSD2SS xmm1, xmm2 (Convert Scalar Double to Single)
 */
void emit_cvtsd2ss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 SIMD Emit - Packed Integer Operations
 * ============================================================================ */

/**
 * Emit PADDD xmm1, xmm2 (Packed Add Doublewords)
 */
void emit_paddd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PADDQ xmm1, xmm2 (Packed Add Quadwords)
 */
void emit_paddq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSUBD xmm1, xmm2 (Packed Subtract Doublewords)
 */
void emit_psubd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSUBQ xmm1, xmm2 (Packed Subtract Quadwords)
 */
void emit_psubq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PAND xmm1, xmm2 (AND)
 */
void emit_pand_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit POR xmm1, xmm2 (OR)
 */
void emit_por_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PXOR xmm1, xmm2 (XOR)
 */
void emit_pxor_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PANDN xmm1, xmm2 (AND NOT)
 */
void emit_pandn_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 SIMD Emit - Multiply and Shift Operations
 * ============================================================================ */

/**
 * Emit PMULL xmm1, xmm2 (Polynomial Multiply Long)
 */
void emit_pmull_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PMULUDQ xmm1, xmm2 (Multiply Packed Unsigned DW to QW)
 */
void emit_pmuludq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSLLQ xmm1, imm8 (Shift Left Logical QW by immediate)
 */
void emit_psllq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/**
 * Emit PSLLD xmm1, imm8 (Shift Left Logical DW by immediate)
 */
void emit_pslld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/**
 * Emit PSRLQ xmm1, imm8 (Shift Right Logical QW by immediate)
 */
void emit_psrlq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/**
 * Emit PSRLD xmm1, imm8 (Shift Right Logical DW by immediate)
 */
void emit_psrld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/**
 * Emit PSRAQ xmm1, imm8 (Shift Right Arithmetic QW by immediate)
 */
void emit_psraq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/**
 * Emit PSRAD xmm1, imm8 (Shift Right Arithmetic DW by immediate)
 */
void emit_psrad_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm);

/* ============================================================================
 * x86_64 SIMD Emit - Compare Operations
 * ============================================================================ */

/**
 * Emit PCMPGTD xmm1, xmm2 (Compare Greater Than DW)
 */
void emit_pcmpgtd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PCMPEQD xmm1, xmm2 (Compare Equal DW)
 */
void emit_pcmpeqd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 SIMD Emit - Min/Max Operations
 * ============================================================================ */

/**
 * Emit PMINUD xmm1, xmm2 (Packed Unsigned Integer Minimum)
 */
void emit_pminud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PMAXUD xmm1, xmm2 (Packed Unsigned Integer Maximum)
 */
void emit_pmaxud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PMINSD xmm1, xmm2 (Packed Signed Integer Minimum)
 */
void emit_pminsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PMAXSD xmm1, xmm2 (Packed Signed Integer Maximum)
 */
void emit_pmaxsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 FP Emit - Reciprocal Operations
 * ============================================================================ */

/**
 * Emit RCPSS xmm1, xmm2 (Reciprocal Scalar Single-Precision)
 */
void emit_rcpss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit RSQRTPS xmm1, xmm2 (Reciprocal Square Root Packed Single-Precision)
 */
void emit_rsqrtps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * x86_64 SIMD Emit - Load/Store Memory Operations
 * ============================================================================ */

/**
 * Emit MOVDQU xmm1, [mem] (Move Unaligned - load)
 */
void emit_movdqu_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr);

/**
 * Emit MOVDQU [mem], xmm1 (Move Unaligned - store)
 */
void emit_movdqu_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src);

/**
 * Emit MOVUPS xmm1, [mem] (Move Unaligned FP - load)
 */
void emit_movups_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr);

/**
 * Emit MOVUPS [mem], xmm1 (Move Unaligned FP - store)
 */
void emit_movups_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src);

/**
 * Emit MOVAPS xmm1, [mem] (Move Aligned FP - load)
 */
void emit_movaps_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr);

/**
 * Emit MOVAPS [mem], xmm1 (Move Aligned FP - store)
 */
void emit_movaps_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src);

/* ============================================================================
 * x86_64 SIMD Emit - Load/Store Register Indirect
 * ============================================================================ */

/**
 * Emit MOVDQU xmm, [base] (Move Unaligned - register indirect load)
 */
void emit_movdqu_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base);

/**
 * Emit MOVDQU [base], xmm (Move Unaligned - register indirect store)
 */
void emit_movdqu_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src);

/**
 * Emit MOVUPS xmm, [base] (Move Unaligned FP - register indirect load)
 */
void emit_movups_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base);

/**
 * Emit MOVUPS [base], xmm (Move Unaligned FP - register indirect store)
 */
void emit_movups_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src);

/**
 * Emit MOVAPS xmm, [base] (Move Aligned FP - register indirect load)
 */
void emit_movaps_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base);

/**
 * Emit MOVAPS [base], xmm (Move Aligned FP - register indirect store)
 */
void emit_movaps_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src);

/* ============================================================================
 * x86_64 SSSE3 Emit - Shuffle/Permutation Operations
 * ============================================================================ */

/**
 * Emit PUNPCKLBW xmm1, xmm2 (Unpack Low Bytes)
 */
void emit_punpcklbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUNPCKHBW xmm1, xmm2 (Unpack High Bytes)
 */
void emit_punpckhbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUNPCKLWD xmm1, xmm2 (Unpack Low Words)
 */
void emit_punpcklwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUNPCKHWD xmm1, xmm2 (Unpack High Words)
 */
void emit_punpckhwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUNPCKLDQ xmm1, xmm2 (Unpack Low Doublewords)
 */
void emit_punpckldq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PUNPCKHDQ xmm1, xmm2 (Unpack High Doublewords)
 */
void emit_punpckhdq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PALIGNR xmm1, xmm2, imm8 (Packed Align Right)
 */
void emit_palignr_xmm_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm);

/**
 * Emit PSHUFB xmm1, xmm2 (Packed Shuffle Bytes)
 */
void emit_pshufb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSIGNB xmm1, xmm2 (Packed Sign Byte)
 */
void emit_psignb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSIGNW xmm1, xmm2 (Packed Sign Word)
 */
void emit_psignw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PSIGND xmm1, xmm2 (Packed Sign Doubleword)
 */
void emit_psignd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src);

/**
 * Emit PEXTRB r32, xmm, imm8 (Extract Byte)
 */
void emit_pextrb_reg_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm);

/**
 * Emit PINSRB xmm, r32, imm8 (Insert Byte)
 */
void emit_pinsrb_xmm_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm);

/* ============================================================================
 * FP Helper Functions
 * ============================================================================ */

/**
 * Emit FABS (Floating-point Absolute Value) for scalar
 */
void emit_fabs_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double);

/**
 * Emit FNEG (Floating-point Negate) for scalar
 */
void emit_fneg_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double);

/**
 * Emit FCSEL (Floating-point Conditional Select) for scalar
 */
void emit_fcsel_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t src2, uint8_t cond);

#endif /* ROSETTA_JIT_EMIT_SIMD_H */
