/* ============================================================================
 * Rosetta Binary Translator - FP Scalar Instruction Emission Header
 * ============================================================================
 *
 * This header defines the interface for x86_64 SSE/SSSE3 scalar FP emission.
 * ============================================================================ */

#ifndef ROSETTA_CODEGEN_FP_H
#define ROSETTA_CODEGEN_FP_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"

/* ============================================================================
 * SSE/SSSE3 Instructions (Scalar FP)
 * ============================================================================ */

/**
 * emit_movss_xmm_xmm - Move scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_movss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_movsd_xmm_xmm - Move scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_movsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_addss_xmm_xmm - Add scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_addss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_addsd_xmm_xmm - Add scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_addsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_subss_xmm_xmm - Subtract scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_subss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_subsd_xmm_xmm - Subtract scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_subsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_mulss_xmm_xmm - Multiply scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_mulss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_mulsd_xmm_xmm - Multiply scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_mulsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_divss_xmm_xmm - Divide scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_divss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_divsd_xmm_xmm - Divide scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_divsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_sqrtss_xmm - Square root scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 */
void emit_sqrtss_xmm(code_buffer_t *buf, u8 dst);

/**
 * emit_sqrtsd_xmm - Square root scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 */
void emit_sqrtsd_xmm(code_buffer_t *buf, u8 dst);

/**
 * emit_ucomiss_xmm_xmm - Unordered compare scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_ucomiss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_ucomisd_xmm_xmm - Unordered compare scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_ucomisd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_cvtss2sd_xmm_xmm - Convert single to double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_cvtss2sd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_cvtsd2ss_xmm_xmm - Convert double to single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_cvtsd2ss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_rcpss_xmm_xmm - Reciprocal scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_rcpss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_rsqrtps_xmm_xmm - Reciprocal square root
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_rsqrtps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_maxss_xmm_xmm - Maximum scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_maxss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_maxsd_xmm_xmm - Maximum scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_maxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_minss_xmm_xmm - Minimum scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_minss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_minsd_xmm_xmm - Minimum scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 */
void emit_minsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/**
 * emit_roundss_xmm_xmm_imm - Round scalar single-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 * @imm: Rounding control
 */
void emit_roundss_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

/**
 * emit_roundsd_xmm_xmm_imm - Round scalar double-precision
 * @buf: Code buffer
 * @dst: Destination XMM register
 * @src: Source XMM register
 * @imm: Rounding control
 */
void emit_roundsd_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

#endif /* ROSETTA_CODEGEN_FP_H */
