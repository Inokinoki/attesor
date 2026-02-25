/* ============================================================================
 * Rosetta Translator - JIT SIMD/FP Code Emitter Implementation
 * ============================================================================
 *
 * This module implements x86_64 SIMD/FP instruction emission for JIT
 * compilation. These functions emit x86_64 SSE/AVX instruction bytes.
 * ============================================================================ */

#include "rosetta_jit_emit.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * x86_64 FP/SIMD Emit Helpers - Scalar Operations
 * ============================================================================ */

void emit_movss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x11);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_movsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x11);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_addss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x58);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_addsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x58);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_subss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5C);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_subsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5C);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_mulss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x59);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_mulsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x59);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_divss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_divsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_sqrtss_xmm(CodeBuffer *buf, uint8_t dst)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x51);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + (dst << 3));  /* ModRM: dst, dst */
}

void emit_sqrtsd_xmm(CodeBuffer *buf, uint8_t dst)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x51);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + (dst << 3));  /* ModRM: dst, dst */
}

void emit_ucomiss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x2E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_ucomisd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x2E);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));  /* ModRM */
}

void emit_absps_xmm(CodeBuffer *buf, uint8_t dst)
{
    /* ANDPS with mask to clear sign bit */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x54);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_abspd_xmm(CodeBuffer *buf, uint8_t dst)
{
    /* ANDPD with mask to clear sign bit */
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x54);
    emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
}

void emit_xorps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_xorpd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x57);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_cvtss2sd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF3);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5A);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_cvtsd2ss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0xF2);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0x5A);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * x86_64 NEON/SIMD Emit Helpers - Packed Integer Operations
 * ============================================================================ */

void emit_paddd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFE);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_paddq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xD4);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psubd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFA);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psubq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xFB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pand_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xDB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_por_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xEB);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pxor_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xEF);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pandn_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    emit_byte(buf, 0x66);  /* Prefix */
    emit_byte(buf, 0x0F);  /* Escape */
    emit_byte(buf, 0xDF);  /* Opcode */
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * x86_64 NEON Emit Helpers - Multiply and Shift Operations
 * ============================================================================ */

void emit_pmull_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMULL: 66 0F 38 0C /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0C);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pmuludq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMULUDQ: 66 0F F4 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xF4);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psllq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSLLQ xmm, imm8: 66 0F 73 /6 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /6 = 110 = 0xD0 base */
    emit_byte(buf, imm & 0x3F);
}

void emit_pslld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSLLD xmm, imm8: 66 0F 72 /6 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /6 = 110 */
    emit_byte(buf, imm & 0x1F);
}

void emit_psrlq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRLQ xmm, imm8: 66 0F 73 /2 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x73);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /2 = 010 = 0xD0+2 */
    emit_byte(buf, imm & 0x3F);
}

void emit_psrld_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRLD xmm, imm8: 66 0F 72 /2 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xD0 + (dst & 7));  /* /2 = 010 */
    emit_byte(buf, imm & 0x1F);
}

void emit_psraq_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRAQ: Use PSRAD for 32-bit elements */
    /* PSRAD xmm, imm8: 66 0F 72 /4 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));  /* /4 = 100 */
    emit_byte(buf, imm & 0x3F);
}

void emit_psrad_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t imm)
{
    /* PSRAD xmm, imm8: 66 0F 72 /4 ib */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x72);
    emit_byte(buf, 0xE0 + (dst & 7));  /* /4 = 100 */
    emit_byte(buf, imm & 0x1F);
}

/* ============================================================================
 * x86_64 NEON Emit Helpers - Compare Operations
 * ============================================================================ */

void emit_pcmpgtd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PCMPGTD: 66 0F 39 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x39);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pcmpeqd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PCMPEQD: 66 0F 76 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x76);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * x86_64 NEON Emit Helpers - Min/Max Operations
 * ============================================================================ */

void emit_pminud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMINUD: 66 0F 38 3B /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3B);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pmaxud_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMAXUD: 66 0F 38 3F /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3F);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pminsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMINSD: 66 0F 38 39 /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x39);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pmaxsd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PMAXSD: 66 0F 38 3D /r */
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x3D);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * x86_64 FP Emit Helpers - Reciprocal Operations
 * ============================================================================ */

void emit_rcpss_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* RCPSS: F3 0F 53 /r */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x53);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_rsqrtps_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* RSQRTPS: F3 0F 52 /r */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x52);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

/* ============================================================================
 * x86_64 NEON Load/Store Emit Helpers - Memory Operations
 * ============================================================================ */

void emit_movdqu_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVDQU: F3 0F 6F /r (register form) */
    /* For memory with RIP-relative: F3 0F 6F 05 <disp32> */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    /* Use RIP-relative addressing: 04 25 <addr> */
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;  /* For now, uses fixed addressing */
}

void emit_movdqu_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVDQU: F3 0F 7F /r (register form) */
    /* For memory with RIP-relative: F3 0F 7F 05 <disp32> */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;  /* For now, uses fixed addressing */
}

void emit_movups_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVUPS: 0F 10 /r (load) */
    /* RIP-relative: 0F 10 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;
}

void emit_movups_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVUPS: 0F 11 /r (store) */
    /* RIP-relative: 0F 11 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;
}

void emit_movaps_xmm_mem(CodeBuffer *buf, uint8_t dst, uint32_t addr)
{
    /* MOVAPS: 0F 28 /r (load) */
    /* RIP-relative: 0F 28 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)dst;
}

void emit_movaps_mem_xmm(CodeBuffer *buf, uint32_t addr, uint8_t src)
{
    /* MOVAPS: 0F 29 /r (store) */
    /* RIP-relative: 0F 29 05 <disp32> */
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x04);
    emit_byte(buf, 0x25);
    emit_byte(buf, (addr >> 0) & 0xFF);
    emit_byte(buf, (addr >> 8) & 0xFF);
    emit_byte(buf, (addr >> 16) & 0xFF);
    emit_byte(buf, (addr >> 24) & 0xFF);
    (void)src;
}

/* ============================================================================
 * x86_64 NEON Load/Store Register Indirect Helpers
 * ============================================================================ */

void emit_movdqu_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVDQU: F3 0F 6F /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6F);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movdqu_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVDQU: F3 0F 7F /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    emit_byte(buf, 0xF3);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x7F);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movups_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVUPS: 0F 10 /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (dst >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x10);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movups_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVUPS: 0F 11 /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    uint8_t rex = 0x40;
    if (src >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (src >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x11);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

void emit_movaps_xmm_mem_reg(CodeBuffer *buf, uint8_t dst, uint8_t base)
{
    /* MOVAPS: 0F 28 /r */
    /* ModR/M: mod=00, reg=dst, rm=base */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (dst >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x28);
    emit_byte(buf, 0x00 + (dst & 7) + ((base & 7) << 3));
}

void emit_movaps_mem_reg_xmm(CodeBuffer *buf, uint8_t base, uint8_t src)
{
    /* MOVAPS: 0F 29 /r */
    /* ModR/M: mod=00, reg=src, rm=base */
    uint8_t rex = 0x40;
    if (src >= 8) rex |= 0x04;  /* REX.R */
    if (base >= 8) rex |= 0x01; /* REX.B */

    if (src >= 8 || base >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x29);
    emit_byte(buf, 0x00 + (src & 7) + ((base & 7) << 3));
}

/* ============================================================================
 * x86_64 SSSE3 Shuffle/Permutation Emit Helpers
 * ============================================================================ */

void emit_punpcklbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLBW: 66 0F 60 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x60);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_punpckhbw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHBW: 66 0F 68 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x68);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_punpcklwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLWD: 66 0F 61 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x61);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_punpckhwd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHWD: 66 0F 69 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x69);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_punpckldq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKLDQ: 66 0F 62 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x62);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_punpckhdq_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PUNPCKHDQ: 66 0F 6A /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x6A);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_palignr_xmm_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PALIGNR: 66 0F 3A 0F /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pshufb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSHUFB: 66 0F 38 00 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x00);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psignb_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGNB: 66 0F 38 08 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x08);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psignw_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGNW: 66 0F 38 09 /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x09);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_psignd_xmm_xmm(CodeBuffer *buf, uint8_t dst, uint8_t src)
{
    /* PSIGND: 66 0F 38 0A /r */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;
    if (src >= 8) rex |= 0x01;

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x38);
    emit_byte(buf, 0x0A);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
}

void emit_pextrb_reg_xmm_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PEXTRB: 66 0F 3A 14 /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x14);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

void emit_pinsrb_xmm_reg_imm(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t imm)
{
    /* PINSRB: 66 0F 3A 20 /r ib */
    uint8_t rex = 0x40;
    if (dst >= 8) rex |= 0x04;  /* REX.R */
    if (src >= 8) rex |= 0x01;  /* REX.B */

    if (dst >= 8 || src >= 8) {
        emit_byte(buf, rex);
    }
    emit_byte(buf, 0x66);
    emit_byte(buf, 0x0F);
    emit_byte(buf, 0x3A);
    emit_byte(buf, 0x20);
    emit_byte(buf, 0xC0 + (dst & 7) + ((src & 7) << 3));
    emit_byte(buf, imm);
}

/* ============================================================================
 * FP Helper Functions
 * ============================================================================ */

void emit_fabs_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FABS, clear the sign bit using ANDPS/ANDPD */
    /* This is a simplified version - full implementation needs mask loading */
    if (is_double) {
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
    } else {
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x54);
        emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
    }
}

void emit_fneg_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, int is_double)
{
    /* Copy src to dst first */
    if (is_double) {
        emit_movsd_xmm_xmm(buf, dst, src);
    } else {
        emit_movss_xmm_xmm(buf, dst, src);
    }

    /* For FNEG, XOR with sign bit mask */
    if (is_double) {
        emit_byte(buf, 0x66);
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
    } else {
        emit_byte(buf, 0x0F);
        emit_byte(buf, 0x57);
        emit_byte(buf, 0xC0 + (dst & 7) + ((dst & 7) << 3));
    }
}

void emit_fcsel_scalar(CodeBuffer *buf, uint8_t dst, uint8_t src, uint8_t src2, uint8_t cond)
{
    /* FCSEL - Floating point conditional select
     * Implementation depends on condition flags
     * This is a simplified version
     */
    (void)cond;  /* Condition handling needs flag check */

    /* For now, just copy src to dst */
    emit_movsd_xmm_xmm(buf, dst, src);
    (void)src2;
}
