/* ============================================================================
 * Rosetta SIMD Translation Implementation
 * ============================================================================
 *
 * This module implements translation of x86_64 SSE/SSE2/SSE4 instructions
 * to ARM64 NEON instructions.
 * ============================================================================ */

#include "rosetta_translate_simd.h"
#include "rosetta_arm64_emit.h"
#include <string.h>

/* ============================================================================
 * SIMD MOV Instructions
 * ============================================================================ */

void translate_simd_mov(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVAPS/MOVUPS/MOVAPD/MOVUPD: Move aligned/unaligned packed single/double */
    /* Translate to ARM64 FMOV (vector) or MOV (vector) */
    emit_simd_mov(code_buf, arm_rd, arm_rm);
}

void translate_simd_movss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVSS: Move scalar single-precision */
    emit_simd_mov_scalar(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_movsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVSD: Move scalar double-precision */
    emit_simd_mov_scalar(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_movhl(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVHLPS/MOVLHPS: Move high/low to low/high */
    emit_simd_movhl(code_buf, arm_rd, arm_rm);
}

void translate_simd_movh(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVHPS/MOVHPD: Move high packed */
    emit_simd_movh(code_buf, arm_rd, arm_rm);
}

void translate_simd_movl(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVLPS/MOVLPD: Move low packed */
    emit_simd_movl(code_buf, arm_rd, arm_rm);
}

void translate_simd_movdqu(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVDQU: Move unaligned double quadword */
    emit_simd_movd(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_movdqa(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVDQA: Move aligned double quadword */
    emit_simd_movd(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_movq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MOVQ: Move quadword */
    emit_simd_movq(code_buf, arm_rd, arm_rm);
}

/* ============================================================================
 * SIMD Arithmetic Instructions
 * ============================================================================ */

void translate_simd_addps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ADDPS: Add packed single-precision */
    emit_simd_fadd(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_addpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ADDPD: Add packed double-precision */
    emit_simd_fadd(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_addss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ADDSS: Add scalar single-precision */
    emit_simd_fadd_scalar(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_addsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ADDSD: Add scalar double-precision */
    emit_simd_fadd_scalar(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_subps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SUBPS: Subtract packed single-precision */
    emit_simd_fsub(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_subpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SUBPD: Subtract packed double-precision */
    emit_simd_fsub(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_subss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SUBSS: Subtract scalar single-precision */
    emit_simd_fsub_scalar(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_subsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SUBSD: Subtract scalar double-precision */
    emit_simd_fsub_scalar(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_mulps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MULPS: Multiply packed single-precision */
    emit_simd_fmul(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_mulpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MULPD: Multiply packed double-precision */
    emit_simd_fmul(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_mulss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MULSS: Multiply scalar single-precision */
    emit_simd_fmul_scalar(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_mulsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* MULSD: Multiply scalar double-precision */
    emit_simd_fmul_scalar(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_divps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* DIVPS: Divide packed single-precision */
    emit_simd_fdiv(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_divpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* DIVPD: Divide packed double-precision */
    emit_simd_fdiv(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_divss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* DIVSS: Divide scalar single-precision */
    emit_simd_fdiv_scalar(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_divsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* DIVSD: Divide scalar double-precision */
    emit_simd_fdiv_scalar(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

/* ============================================================================
 * SIMD Logical Instructions
 * ============================================================================ */

void translate_simd_and(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ANDPS/ANDPD: Bitwise AND */
    emit_simd_and(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_andn(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ANDNPS/ANDNPD: Bitwise AND NOT */
    emit_simd_bic(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_or(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* ORPS/ORPD: Bitwise OR */
    emit_simd_orr(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_xor(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* XORPS/XORPD: Bitwise XOR */
    emit_simd_eor(code_buf, arm_rd, arm_rd, arm_rm);
}

/* ============================================================================
 * SIMD Comparison Instructions
 * ============================================================================ */

void translate_simd_cmpps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CMPPS: Compare packed single-precision */
    emit_simd_fcmp(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cmppd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CMPPD: Compare packed double-precision */
    emit_simd_fcmp(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_cmpss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CMPSS: Compare scalar single-precision */
    emit_simd_fcmp_scalar(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cmpsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CMPSD: Compare scalar double-precision */
    emit_simd_fcmp_scalar(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_ucomiss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* UCOMISS: Unordered compare scalar single-precision */
    emit_simd_ucomi(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_ucomisd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* UCOMISD: Unordered compare scalar double-precision */
    emit_simd_ucomi(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_comiss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* COMISS: Ordered compare scalar single-precision */
    emit_simd_comi(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_comisd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* COMISD: Ordered compare scalar double-precision */
    emit_simd_comi(code_buf, arm_rd, arm_rm, 1);
}

/* ============================================================================
 * SIMD Conversion Instructions
 * ============================================================================ */

void translate_simd_cvtps2pd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTPS2PD: Convert packed single to double */
    emit_simd_fcvt(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cvtpd2ps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTPD2PS: Convert packed double to single */
    emit_simd_fcvt(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_cvtss2sd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTSS2SD: Convert scalar single to double */
    emit_simd_fcvt_scalar(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cvtsd2ss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTSD2SS: Convert scalar double to single */
    emit_simd_fcvt_scalar(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_cvtdq2ps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTDQ2PS: Convert packed doubleword to single */
    emit_simd_scvtf(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cvtps2dq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTPS2DQ: Convert packed single to doubleword */
    emit_simd_fcvtzs(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_cvttps2dq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* CVTTPS2DQ: Convert with truncation packed single to doubleword */
    emit_simd_fcvtxzs(code_buf, arm_rd, arm_rm, 0);
}

/* ============================================================================
 * SIMD Square Root Instructions
 * ============================================================================ */

void translate_simd_sqrtps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SQRTPS: Square root packed single-precision */
    emit_simd_fsqrt(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_sqrtpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SQRTPD: Square root packed double-precision */
    emit_simd_fsqrt(code_buf, arm_rd, arm_rm, 1);
}

void translate_simd_sqrtss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SQRTSS: Square root scalar single-precision */
    emit_simd_fsqrt_scalar(code_buf, arm_rd, arm_rm, 0);
}

void translate_simd_sqrtsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* SQRTSD: Square root scalar double-precision */
    emit_simd_fsqrt_scalar(code_buf, arm_rd, arm_rm, 1);
}

/* ============================================================================
 * SIMD Shuffle/Unpack Instructions
 * ============================================================================ */

void translate_simd_pshufd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    /* PSHUFD: Shuffle packed doublewords */
    uint8_t imm = (uint8_t)(insn->imm & 0xFF);
    emit_simd_shuf(code_buf, arm_rd, arm_rm, imm);
}

void translate_simd_pshuflw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    /* PSHUFLW: Shuffle low words */
    uint8_t imm = (uint8_t)(insn->imm & 0xFF);
    emit_simd_shuf_lw(code_buf, arm_rd, arm_rm, imm);
}

void translate_simd_pshufhw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    (void)arm_rd;
    (void)arm_rm;
    /* PSHUFHW: Shuffle high words */
    uint8_t imm = (uint8_t)(insn->imm & 0xFF);
    emit_simd_shuf_hw(code_buf, arm_rd, arm_rm, imm);
}

void translate_simd_punpckldq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PUNPCKLDQ: Unpack low doublewords */
    emit_simd_unpckl(code_buf, arm_rd, arm_rm, 2);
}

void translate_simd_punpckhdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PUNPCKHDQ: Unpack high doublewords */
    emit_simd_unpckh(code_buf, arm_rd, arm_rm, 2);
}

void translate_simd_punpcklqdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PUNPCKLQDQ: Unpack low quadwords */
    emit_simd_unpckl(code_buf, arm_rd, arm_rm, 3);
}

void translate_simd_punpckhqdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PUNPCKHQDQ: Unpack high quadwords */
    emit_simd_unpckh(code_buf, arm_rd, arm_rm, 3);
}

/* ============================================================================
 * SIMD Integer Instructions
 * ============================================================================ */

void translate_simd_paddb(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PADDB: Add packed bytes */
    emit_simd_add(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_paddw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PADDW: Add packed words */
    emit_simd_add(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_paddd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PADDD: Add packed doublewords */
    emit_simd_add(code_buf, arm_rd, arm_rd, arm_rm, 2);
}

void translate_simd_paddq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PADDQ: Add packed quadwords */
    emit_simd_add(code_buf, arm_rd, arm_rd, arm_rm, 3);
}

void translate_simd_psubb(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PSUBB: Subtract packed bytes */
    emit_simd_sub(code_buf, arm_rd, arm_rd, arm_rm, 0);
}

void translate_simd_psubw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PSUBW: Subtract packed words */
    emit_simd_sub(code_buf, arm_rd, arm_rd, arm_rm, 1);
}

void translate_simd_psubd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PSUBD: Subtract packed doublewords */
    emit_simd_sub(code_buf, arm_rd, arm_rd, arm_rm, 2);
}

void translate_simd_psubq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PSUBQ: Subtract packed quadwords */
    emit_simd_sub(code_buf, arm_rd, arm_rd, arm_rm, 3);
}

void translate_simd_pand(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PAND: Bitwise AND */
    emit_simd_and(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_pandn(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PANDN: Bitwise AND NOT */
    emit_simd_bic(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_por(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* POR: Bitwise OR */
    emit_simd_orr(code_buf, arm_rd, arm_rd, arm_rm);
}

void translate_simd_pxor(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm)
{
    (void)insn;
    /* PXOR: Bitwise XOR */
    emit_simd_eor(code_buf, arm_rd, arm_rd, arm_rm);
}

/* End of rosetta_translate_simd.c */
