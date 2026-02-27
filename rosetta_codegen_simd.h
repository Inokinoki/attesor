/* ============================================================================
 * Rosetta Binary Translator - SIMD Instruction Emission Header
 * ============================================================================
 *
 * This header defines the interface for x86_64 SSE2/SSSE3/AVX SIMD emission.
 * ============================================================================ */

#ifndef ROSETTA_CODEGEN_SIMD_H
#define ROSETTA_CODEGEN_SIMD_H

#include "rosetta_types.h"
#include "rosetta_codegen_buf.h"

/* ============================================================================
 * SSE2/SSSE3 Instructions (Vector/Packed)
 * ============================================================================ */

/* Packed Add */
void emit_paddd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_paddq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_paddb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_paddw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Packed Subtract */
void emit_psubd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psubq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psubb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psubw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Packed Logical */
void emit_pand_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_por_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pxor_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pandn_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Packed Multiply */
void emit_pmuludq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmulld_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmullw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Packed Min/Max */
void emit_pminud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmaxud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pminsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmaxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pminub_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmaxub_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pminsw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pmaxsw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Packed Compare */
void emit_pcmpgtd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pcmpeqd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pcmpeqb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pcmpeqw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pcmpgtb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pcmpgtw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);

/* Vector Shift */
void emit_pslld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psllq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_pslldq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psrld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psrlq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psrldq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psrad_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);
void emit_psraq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm);

/* Memory Operations */
void emit_movdqu_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);
void emit_movdqu_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);
void emit_movups_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);
void emit_movups_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);
void emit_movaps_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base);
void emit_movaps_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src);

/* SSSE3 Shuffle/Unpack */
void emit_punpcklbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_punpckhbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_punpcklwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_punpckhwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_punpckldq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_punpckhdq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_palignr_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);
void emit_pshufb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psignb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psignw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_psignd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src);
void emit_pextrb_reg_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);
void emit_pinsrb_xmm_reg_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm);

/* AVX Instructions (3-operand) */
void emit_vmovaps_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2);
void emit_vpaddd_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2);
void emit_vpxor_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2);

#endif /* ROSETTA_CODEGEN_SIMD_H */
