/* ============================================================================
 * Rosetta SIMD Translation Header
 * ============================================================================
 *
 * This header declares SIMD translation functions for translating x86_64
 * SSE/SSE2/SSE4 instructions to ARM64 NEON instructions.
 * ============================================================================ */

#ifndef ROSETTA_TRANSLATE_SIMD_H
#define ROSETTA_TRANSLATE_SIMD_H

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_codegen.h"

/* ============================================================================
 * SIMD MOV Instructions
 * ============================================================================ */

void translate_simd_mov(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movhl(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movh(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movl(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movdqu(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movdqa(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_movq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Arithmetic Instructions
 * ============================================================================ */

void translate_simd_addps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_addpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_addss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_addsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_subps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_subpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_subss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_subsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_mulps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_mulpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_mulss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_mulsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_divps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_divpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_divss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_divsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Logical Instructions
 * ============================================================================ */

void translate_simd_and(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_andn(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_or(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_xor(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Comparison Instructions
 * ============================================================================ */

void translate_simd_cmpps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cmppd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cmpss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cmpsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_ucomiss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_ucomisd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_comiss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_comisd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Conversion Instructions
 * ============================================================================ */

void translate_simd_cvtps2pd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvtpd2ps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvtss2sd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvtsd2ss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvtdq2ps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvtps2dq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_cvttps2dq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Square Root Instructions
 * ============================================================================ */

void translate_simd_sqrtps(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_sqrtpd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_sqrtss(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_sqrtsd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Shuffle/Unpack Instructions
 * ============================================================================ */

void translate_simd_pshufd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_pshuflw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_pshufhw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_punpckldq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_punpckhdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_punpcklqdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_punpckhqdq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

/* ============================================================================
 * SIMD Integer Instructions
 * ============================================================================ */

void translate_simd_paddb(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_paddw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_paddd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_paddq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_psubb(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_psubw(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_psubd(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_psubq(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

void translate_simd_pand(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_pandn(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_por(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);
void translate_simd_pxor(code_buffer_t *code_buf, const x86_insn_t *insn, uint8_t arm_rd, uint8_t arm_rm);

#endif /* ROSETTA_TRANSLATE_SIMD_H */
