/* ============================================================================
 * Rosetta Binary Translator - FP Scalar Instruction Emission
 * ============================================================================
 *
 * This module contains x86_64 SSE/SSSE3 scalar floating-point instruction
 * emission. Each function emits machine code bytes for FP operations.
 * ============================================================================ */

#include "rosetta_codegen_fp.h"

/* ============================================================================
 * SSE/SSSE3 Instructions (Scalar FP)
 * ============================================================================ */

void emit_movss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MOVSS xmm1, xmm2: F3 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_movsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MOVSD xmm1, xmm2: F2 0F 10 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_addss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* ADDSS xmm1, xmm2: F3 0F 58 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x58);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_addsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* ADDSD xmm1, xmm2: F2 0F 58 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x58);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_subss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* SUBSS xmm1, xmm2: F3 0F 5C C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5C);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_subsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* SUBSD xmm1, xmm2: F2 0F 5C C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5C);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_mulss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MULSS xmm1, xmm2: F3 0F 59 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x59);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_mulsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MULSD xmm1, xmm2: F2 0F 59 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x59);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_divss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* DIVSS xmm1, xmm2: F3 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_divsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* DIVSD xmm1, xmm2: F2 0F 5E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_sqrtss_xmm(code_buffer_t *buf, u8 dst) {
    /* SQRTSS xmm1, xmm2: F3 0F 51 C0 + dst*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x51);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_sqrtsd_xmm(code_buffer_t *buf, u8 dst) {
    /* SQRSD xmm1, xmm2: F2 0F 51 C0 + dst*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x51);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_ucomiss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* UCOMISS xmm1, xmm2: 0F 2E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x2E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_ucomisd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* UCOMISD xmm1, xmm2: 66 0F 2E C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x2E);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_cvtss2sd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* CVTSS2SD xmm1, xmm2: F3 0F 5A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_cvtsd2ss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* CVTSD2SS xmm1, xmm2: F2 0F 5A C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_rcpss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* RCPSS xmm1, xmm2: F3 0F 53 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x53);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_rsqrtps_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* RSQRTPS xmm1, xmm2: F3 0F 52 C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x52);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_maxss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MAXSS xmm1, xmm2: F3 0F 5F C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_maxsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MAXSD xmm1, xmm2: F2 0F 5F C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5F);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_minss_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MINSS xmm1, xmm2: F3 0F 5D C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5D);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_minsd_xmm_xmm(code_buffer_t *buf, u8 dst, u8 src) {
    /* MINSD xmm1, xmm2: F2 0F 5D C0 + src*8 + dst */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0xF2);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x5D);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
}

void emit_roundss_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* ROUNDSS xmm1, xmm2, imm8: 66 0F 3A 0A C0 + src*8 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0A);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}

void emit_roundsd_xmm_xmm_imm(code_buffer_t *buf, u8 dst, u8 src, u8 imm) {
    /* ROUNDSD xmm1, xmm2, imm8: 66 0F 3A 0B C0 + src*8 + dst imm */
    u8 rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (rex != 0x40) emit_byte(buf, rex);
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0B);
    emit_byte(buf, 0xC0 + (src & 7) + ((dst & 7) << 3));
    emit_byte(buf, imm);
}
