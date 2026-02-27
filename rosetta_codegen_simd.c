/* ============================================================================
 * Rosetta Binary Translator - SIMD Instruction Emission
 * ============================================================================
 *
 * This module contains x86_64 SSE2/SSSE3/AVX SIMD instruction emission.
 * Each function emits machine code bytes for vector/packed operations.
 * ============================================================================ */

#include "rosetta_codegen_simd.h"

/* ============================================================================
 * SSE2/SSSE3 Instructions (Vector/Packed)
 * ============================================================================ */

void emit_paddd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDD xmm1, xmm2: 66 0F FE C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFE);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_paddq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDQ xmm1, xmm2: 66 0F D4 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xD4);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_paddb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDB xmm1, xmm2: 66 0F FC C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFC);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_paddw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PADDW xmm1, xmm2: 66 0F FD C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFD);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBD xmm1, xmm2: 66 0F FA C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFA);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBQ xmm1, xmm2: 66 0F FB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xFB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBB xmm1, xmm2: 66 0F F8 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF8);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psubw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSUBW xmm1, xmm2: 66 0F F9 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF9);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pand_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PAND xmm1, xmm2: 66 0F DB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_por_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* POR xmm1, xmm2: 66 0F EB C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEB);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pxor_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PXOR xmm1, xmm2: 66 0F EF C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEF);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pandn_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PANDN xmm1, xmm2: 66 0F DF C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDF);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmuludq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMULUDQ xmm1, xmm2: 66 0F F4 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF4);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmulld_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMULLD xmm1, xmm2: 66 0F 38 28 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmullw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMULLW xmm1, xmm2: 66 0F D5 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xD5);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINUD xmm1, xmm2: 66 0F 3B C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3B);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxud_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXUD xmm1, xmm2: 66 0F 3F C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINSD xmm1, xmm2: 66 0F 3D C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3D);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXSD xmm1, xmm2: 66 0F 3E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminub_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINUB xmm1, xmm2: 66 0F DA C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDA);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxub_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXUB xmm1, xmm2: 66 0F DE C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xDE);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pminsw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMINSW xmm1, xmm2: 66 0F EA C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEA);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pmaxsw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PMAXSW xmm1, xmm2: 66 0F EE C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xEE);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpgtd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPGTD xmm1, xmm2: 66 0F 3A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpeqd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPEQD xmm1, xmm2: 66 0F 76 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x76);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpeqb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPEQB xmm1, xmm2: 66 0F 74 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x74);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpeqw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPEQW xmm1, xmm2: 66 0F 75 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x75);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpgtb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPGTB xmm1, xmm2: 66 0F 38 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pcmpgtw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PCMPGTW xmm1, xmm2: 66 0F 39 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x39);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

/* ============================================================================
 * Shift Instructions (Vector)
 * ============================================================================ */

void emit_pslld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSLLD xmm1, imm8: 66 0F 72 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psllq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSLLQ xmm1, imm8: 66 0F 73 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_pslldq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSLLDQ xmm1, imm8: 66 0F 73 B0 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xB0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrld_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRLD xmm1, imm8: 66 0F 72 C8 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xC8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrlq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRLQ xmm1, imm8: 66 0F 73 C0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xC0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrldq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRLDQ xmm1, imm8: 66 0F 73 B8 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xB8 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psrad_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRAD xmm1, imm8: 66 0F 72 E0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_byte(buf, imm);
}

void emit_psraq_xmm_imm(code_buffer_t *buf, u8 dst, u8 imm) {
    /* PSRAQ xmm1, imm8: 66 0F 73 E0 + dst iw */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xE0 + (dst & 7));
    emit_byte(buf, imm);
}

/* ============================================================================
 * Memory Operations (Register Indirect)
 * ============================================================================ */

void emit_movdqu_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVDQU xmm, [base]: F3 0F 6F 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movdqu_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVDQU [base], xmm: F3 0F 7F 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movups_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVUPS xmm, [base]: 0F 10 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movups_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVUPS [base], xmm: 0F 11 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movaps_xmm_mem_reg(code_buffer_t *buf, u8 dst, u8 base) {
    /* MOVAPS xmm, [base]: 0F 28 00 + dst*8 + base */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movaps_mem_reg_xmm(code_buffer_t *buf, u8 base, u8 src) {
    /* MOVAPS [base], xmm: 0F 29 00 + src*8 + base */
    u8 rex = 0x40;
    if (src >= 8) rex |= 0x04;
    if (base >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/* ============================================================================
 * SSSE3 Shuffle/Unpack Instructions
 * ============================================================================ */

void emit_punpcklbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLBW xmm1, xmm2: 66 0F 60 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x60);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhbw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHBW xmm1, xmm2: 66 0F 68 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x68);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpcklwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLWD xmm1, xmm2: 66 0F 61 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x61);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhwd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHWD xmm1, xmm2: 66 0F 69 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x69);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckldq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKLDQ xmm1, xmm2: 66 0F 62 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x62);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_punpckhdq_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PUNPCKHDQ xmm1, xmm2: 66 0F 6A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_palignr_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PALIGNR xmm1, xmm2, imm8: 66 0F 3A 0F C0 + src*8 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pshufb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSHUFB xmm1, xmm2: 66 0F 38 00 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x00);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignb_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGNB xmm1, xmm2: 66 0F 38 08 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x08);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignw_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGNW xmm1, xmm2: 66 0F 38 09 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x09);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_psignd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* PSIGND xmm1, xmm2: 66 0F 38 0A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_pextrb_reg_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PEXTRB r32, xmm, imm8: 66 0F 3A 14 C0 + src*8 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x01;
    if (src >= 8) rex |= 0x04;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x14);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pinsrb_xmm_reg_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* PINSRB xmm1, r32, imm8: 66 0F 3A 20 C0 + dst*8 + src imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x20);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

/* ============================================================================
 * AVX Instructions (3-operand)
 * ============================================================================ */

void emit_vmovaps_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2) {
    /* VMOVAPS ymm1, ymm2, ymm3: C5 FC 28 C0 + src2*8 + dst (src1 ignored for move) */
    u8 vex2 = 0xC5;
    u8 vex1 = 0xFC;  /* VEX.L=1, VEX.vvvv=src1 */
    if (dst >= 8) vex1 |= 0x04;
    if (src2 >= 8) vex1 |= 0x01;
    vex1 |= ((~src1) & 0x0F) << 3;

    emit_byte(buf, vex2);
    emit_byte(buf, vex1);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0xC0 + (src2 & 7) + ((dst & 7) << 3));
}

void emit_vpaddd_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2) {
    /* VPADDD ymm1, ymm2, ymm3: C5 FC FE C0 + src2*8 + dst */
    u8 vex2 = 0xC5;
    u8 vex1 = 0xFC;
    if (dst >= 8) vex1 |= 0x04;
    if (src2 >= 8) vex1 |= 0x01;
    vex1 |= ((~src1) & 0x0F) << 3;

    emit_byte(buf, vex2);
    emit_byte(buf, vex1);
    emit_byte(buf, 0xFE);
    emit_byte(buf, 0xC0 + (src2 & 7) + ((dst & 7) << 3));
}

void emit_vpxor_ymm_ymm_ymm(code_buffer_t *buf, u8 dst, u8 src1, u8 src2) {
    /* VPXOR ymm1, ymm2, ymm3: C5 FD EF C0 + src2*8 + dst */
    u8 vex2 = 0xC5;
    u8 vex1 = 0xFD;
    if (dst >= 8) vex1 |= 0x04;
    if (src2 >= 8) vex1 |= 0x01;
    vex1 |= ((~src1) & 0x0F) << 3;

    emit_byte(buf, vex2);
    emit_byte(buf, vex1);
    emit_byte(buf, 0xEF);
    emit_byte(buf, 0xC0 + (src2 & 7) + ((dst & 7) << 3));
}
