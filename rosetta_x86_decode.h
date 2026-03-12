/* ============================================================================
 * Rosetta x86_64 Instruction Decoder
 * ============================================================================
 *
 * This module handles decoding of x86_64 instructions into a structured
 * format that can be used for translation to ARM64.
 * ============================================================================ */

#ifndef ROSETTA_X86_DECODE_H
#define ROSETTA_X86_DECODE_H

#include "rosetta_types.h"
#include <stdint.h>

/* ============================================================================
 * x86_64 Instruction Structure
 * ============================================================================ */

/**
 * Decoded x86_64 instruction information
 */
typedef struct {
    uint8_t opcode;         /* Primary opcode byte */
    uint8_t opcode2;        /* Secondary opcode (for 0F xx) */
    uint8_t rex;            /* REX prefix (0 if none) */
    uint8_t modrm;          /* ModR/M byte (0 if none) */
    int32_t disp;           /* Displacement */
    uint8_t disp_size;      /* Displacement size in bytes */
    int64_t imm;            /* Immediate value */
    uint8_t imm_size;       /* Immediate size in bytes */
    uint8_t length;         /* Total instruction length */
    uint8_t mod;            /* ModR/M mod field */
    uint8_t reg;            /* ModR/M reg field */
    uint8_t rm;             /* ModR/M rm field */
    uint8_t simd_prefix;    /* SIMD prefix: 0x66, 0xF2, 0xF3 */
    int has_modrm;          /* Has ModR/M byte */
    int is_64bit;           /* 64-bit operand size */
} x86_insn_t;

/* ============================================================================
 * Instruction Decoder
 * ============================================================================ */

/**
 * Decode an x86_64 instruction
 * @param insn_ptr Pointer to instruction bytes
 * @param insn Output instruction structure
 * @return Instruction length in bytes
 */
int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn);

/* ============================================================================
 * Instruction Type Predicates (P0 - Essential)
 * ============================================================================ */

static inline int x86_is_add(const x86_insn_t *i);
static inline int x86_is_sub(const x86_insn_t *i);
static inline int x86_is_and(const x86_insn_t *i);
static inline int x86_is_or(const x86_insn_t *i);
static inline int x86_is_xor(const x86_insn_t *i);
static inline int x86_is_mov(const x86_insn_t *i);
static inline int x86_is_mov_imm64(const x86_insn_t *i);
static inline int x86_is_lea(const x86_insn_t *i);
static inline int x86_is_cmp(const x86_insn_t *i);
static inline int x86_is_test(const x86_insn_t *i);
static inline int x86_is_jcc(const x86_insn_t *i);
static inline int x86_is_jmp(const x86_insn_t *i);
static inline int x86_is_call(const x86_insn_t *i);
static inline int x86_is_ret(const x86_insn_t *i);
static inline int x86_is_mul(const x86_insn_t *i);
static inline int x86_is_div(const x86_insn_t *i);

static inline uint8_t x86_get_jcc_cond(const x86_insn_t *i);

/* P0 - Essential instructions */
static inline int x86_is_movzx(const x86_insn_t *i);
static inline int x86_is_movsx(const x86_insn_t *i);
static inline int x86_is_movsxd(const x86_insn_t *i);
static inline int x86_is_inc(const x86_insn_t *i);
static inline int x86_is_dec(const x86_insn_t *i);
static inline int x86_is_neg(const x86_insn_t *i);
static inline int x86_is_not(const x86_insn_t *i);
static inline int x86_is_push(const x86_insn_t *i);
static inline int x86_is_pop(const x86_insn_t *i);
static inline int x86_is_nop(const x86_insn_t *i);

/* Shift/Rotate instructions */
static inline int x86_is_shl(const x86_insn_t *i);
static inline int x86_is_shr(const x86_insn_t *i);
static inline int x86_is_sar(const x86_insn_t *i);
static inline int x86_is_rol(const x86_insn_t *i);
static inline int x86_is_ror(const x86_insn_t *i);

/* P1 - Control flow instructions */
static inline int x86_is_cmov(const x86_insn_t *i);
static inline uint8_t x86_get_cmov_cond(const x86_insn_t *i);
static inline int x86_is_setcc(const x86_insn_t *i);
static inline uint8_t x86_get_setcc_cond(const x86_insn_t *i);
static inline int x86_is_xchg(const x86_insn_t *i);

/* P2 - Bit manipulation instructions */
static inline int x86_is_bsf(const x86_insn_t *i);
static inline int x86_is_bsr(const x86_insn_t *i);
static inline int x86_is_bt(const x86_insn_t *i);
static inline int x86_is_bts(const x86_insn_t *i);
static inline int x86_is_btr(const x86_insn_t *i);
static inline int x86_is_btc(const x86_insn_t *i);
static inline int x86_is_popcnt(const x86_insn_t *i);
static inline int x86_is_tzcnt(const x86_insn_t *i);
static inline int x86_is_lzcnt(const x86_insn_t *i);

/* P3 - String operations */
static inline int x86_is_movs(const x86_insn_t *i);
static inline int x86_is_stos(const x86_insn_t *i);
static inline int x86_is_lods(const x86_insn_t *i);
static inline int x86_is_cmps(const x86_insn_t *i);
static inline int x86_is_scas(const x86_insn_t *i);
static inline int x86_has_rep_prefix(const x86_insn_t *i);

/* P4 - Special instructions */
static inline int x86_is_cpuid(const x86_insn_t *i);
static inline int x86_is_rdtsc(const x86_insn_t *i);
static inline int x86_is_rdtscp(const x86_insn_t *i);
static inline int x86_is_shld(const x86_insn_t *i);
static inline int x86_is_shrd(const x86_insn_t *i);
static inline int x86_is_cwd(const x86_insn_t *i);
static inline int x86_is_cqo(const x86_insn_t *i);
static inline int x86_is_cli(const x86_insn_t *i);
static inline int x86_is_sti(const x86_insn_t *i);
static inline int x86_is_cli_sti(const x86_insn_t *i);

/* P5 - SIMD/Floating Point instructions (SSE, SSE2, SSE4, AVX) */
static inline int x86_is_movaps(const x86_insn_t *i);
static inline int x86_is_movups(const x86_insn_t *i);
static inline int x86_is_movss(const x86_insn_t *i);
static inline int x86_is_movsd(const x86_insn_t *i);
static inline int x86_is_movapd(const x86_insn_t *i);
static inline int x86_is_movupd(const x86_insn_t *i);
static inline int x86_is_movhlps(const x86_insn_t *i);
static inline int x86_is_movlhps(const x86_insn_t *i);
static inline int x86_is_movhps(const x86_insn_t *i);
static inline int x86_is_movhpd(const x86_insn_t *i);
static inline int x86_is_movlps(const x86_insn_t *i);
static inline int x86_is_movlpd(const x86_insn_t *i);
static inline int x86_is_addps(const x86_insn_t *i);
static inline int x86_is_addpd(const x86_insn_t *i);
static inline int x86_is_addss(const x86_insn_t *i);
static inline int x86_is_addsd(const x86_insn_t *i);
static inline int x86_is_subps(const x86_insn_t *i);
static inline int x86_is_subpd(const x86_insn_t *i);
static inline int x86_is_subss(const x86_insn_t *i);
static inline int x86_is_subsd(const x86_insn_t *i);
static inline int x86_is_mulps(const x86_insn_t *i);
static inline int x86_is_mulpd(const x86_insn_t *i);
static inline int x86_is_mulss(const x86_insn_t *i);
static inline int x86_is_mulsd(const x86_insn_t *i);
static inline int x86_is_divps(const x86_insn_t *i);
static inline int x86_is_divpd(const x86_insn_t *i);
static inline int x86_is_divss(const x86_insn_t *i);
static inline int x86_is_divsd(const x86_insn_t *i);
static inline int x86_is_maxps(const x86_insn_t *i);
static inline int x86_is_maxpd(const x86_insn_t *i);
static inline int x86_is_maxss(const x86_insn_t *i);
static inline int x86_is_maxsd(const x86_insn_t *i);
static inline int x86_is_minps(const x86_insn_t *i);
static inline int x86_is_minpd(const x86_insn_t *i);
static inline int x86_is_minss(const x86_insn_t *i);
static inline int x86_is_minsd(const x86_insn_t *i);
static inline int x86_is_andps(const x86_insn_t *i);
static inline int x86_is_andpd(const x86_insn_t *i);
static inline int x86_is_andnps(const x86_insn_t *i);
static inline int x86_is_andnpd(const x86_insn_t *i);
static inline int x86_is_orps(const x86_insn_t *i);
static inline int x86_is_orpd(const x86_insn_t *i);
static inline int x86_is_xorps(const x86_insn_t *i);
static inline int x86_is_xorpd(const x86_insn_t *i);
static inline int x86_is_cmpps(const x86_insn_t *i);
static inline int x86_is_cmppd(const x86_insn_t *i);
static inline int x86_is_cmpss(const x86_insn_t *i);
static inline int x86_is_cmpsd(const x86_insn_t *i);
static inline int x86_is_cvtps2pd(const x86_insn_t *i);
static inline int x86_is_cvtpd2ps(const x86_insn_t *i);
static inline int x86_is_cvtss2sd(const x86_insn_t *i);
static inline int x86_is_cvtsd2ss(const x86_insn_t *i);
static inline int x86_is_cvtdq2ps(const x86_insn_t *i);
static inline int x86_is_cvtps2dq(const x86_insn_t *i);
static inline int x86_is_cvttps2dq(const x86_insn_t *i);
static inline int x86_is_cvtdq2pd(const x86_insn_t *i);
static inline int x86_is_cvtpd2dq(const x86_insn_t *i);
static inline int x86_is_cvttpd2dq(const x86_insn_t *i);
static inline int x86_is_cvtsi2ss(const x86_insn_t *i);
static inline int x86_is_cvtsi2sd(const x86_insn_t *i);
static inline int x86_is_cvtss2si(const x86_insn_t *i);
static inline int x86_is_cvtsd2si(const x86_insn_t *i);
static inline int x86_is_cvttss2si(const x86_insn_t *i);
static inline int x86_is_cvttsd2si(const x86_insn_t *i);
static inline int x86_is_sqrtps(const x86_insn_t *i);
static inline int x86_is_sqrtpd(const x86_insn_t *i);
static inline int x86_is_sqrtss(const x86_insn_t *i);
static inline int x86_is_sqrtsd(const x86_insn_t *i);
static inline int x86_is_rsqrtps(const x86_insn_t *i);
static inline int x86_is_rsqrtss(const x86_insn_t *i);
static inline int x86_is_rcpps(const x86_insn_t *i);
static inline int x86_is_rcpss(const x86_insn_t *i);
static inline int x86_is_ucomiss(const x86_insn_t *i);
static inline int x86_is_ucomisd(const x86_insn_t *i);
static inline int x86_is_comiss(const x86_insn_t *i);
static inline int x86_is_comisd(const x86_insn_t *i);
static inline int x86_is_ldmxcsr(const x86_insn_t *i);
static inline int x86_is_stmxcsr(const x86_insn_t *i);
static inline int x86_is_pshufd(const x86_insn_t *i);
static inline int x86_is_pshuflw(const x86_insn_t *i);
static inline int x86_is_pshufhw(const x86_insn_t *i);
static inline int x86_is_punpckldq(const x86_insn_t *i);
static inline int x86_is_punpckhdq(const x86_insn_t *i);
static inline int x86_is_punpcklqdq(const x86_insn_t *i);
static inline int x86_is_punpckhqdq(const x86_insn_t *i);
static inline int x86_is_movdqu(const x86_insn_t *i);
static inline int x86_is_movdqa(const x86_insn_t *i);
static inline int x86_is_movq(const x86_insn_t *i);
static inline int x86_is_movdq2q(const x86_insn_t *i);
static inline int x86_is_movq2dq(const x86_insn_t *i);
static inline int x86_is_pmovmskb(const x86_insn_t *i);
static inline int x86_is_pextrw(const x86_insn_t *i);
static inline int x86_is_pinsrw(const x86_insn_t *i);
static inline int x86_is_pshufw(const x86_insn_t *i);
static inline int x86_is_paddb(const x86_insn_t *i);
static inline int x86_is_paddw(const x86_insn_t *i);
static inline int x86_is_paddd(const x86_insn_t *i);
static inline int x86_is_paddq(const x86_insn_t *i);
static inline int x86_is_psubb(const x86_insn_t *i);
static inline int x86_is_psubw(const x86_insn_t *i);
static inline int x86_is_psubd(const x86_insn_t *i);
static inline int x86_is_psubq(const x86_insn_t *i);
static inline int x86_is_pand(const x86_insn_t *i);
static inline int x86_is_pandn(const x86_insn_t *i);
static inline int x86_is_por(const x86_insn_t *i);
static inline int x86_is_pxor(const x86_insn_t *i);
static inline int x86_is_pcmpgtb(const x86_insn_t *i);
static inline int x86_is_pcmpgtw(const x86_insn_t *i);
static inline int x86_is_pcmpgtd(const x86_insn_t *i);
static inline int x86_is_pcmpeq(const x86_insn_t *i);
static inline int x86_is_pminub(const x86_insn_t *i);
static inline int x86_is_pmaxub(const x86_insn_t *i);
static inline int x86_is_pminsw(const x86_insn_t *i);
static inline int x86_is_pmaxsw(const x86_insn_t *i);
static inline int x86_is_pmullw(const x86_insn_t *i);
static inline int x86_is_pmulhw(const x86_insn_t *i);
static inline int x86_is_pmulhuw(const x86_insn_t *i);
static inline int x86_is_pavgb(const x86_insn_t *i);
static inline int x86_is_pavgw(const x86_insn_t *i);
static inline int x86_is_psadbw(const x86_insn_t *i);
static inline int x86_is_pmovmskb2(const x86_insn_t *i);
static inline int x86_is_pextrb(const x86_insn_t *i);
static inline int x86_is_pextrd(const x86_insn_t *i);
static inline int x86_is_insertps(const x86_insn_t *i);
static inline int x86_is_roundps(const x86_insn_t *i);
static inline int x86_is_roundpd(const x86_insn_t *i);
static inline int x86_is_roundss(const x86_insn_t *i);
static inline int x86_is_roundsd(const x86_insn_t *i);
static inline int x86_is_blendvps(const x86_insn_t *i);
static inline int x86_is_blendvpd(const x86_insn_t *i);
static inline int x86_is_dppd(const x86_insn_t *i);
static inline int x86_is_dpps(const x86_insn_t *i);

/* Helper predicates for SIMD/FP classification */
static inline int x86_is_simd(const x86_insn_t *i);
static inline int x86_is_fp(const x86_insn_t *i);
static inline int x86_is_simd_mov(const x86_insn_t *i);
static inline int x86_is_simd_arith(const x86_insn_t *i);
static inline int x86_is_simd_cmp(const x86_insn_t *i);
static inline int x86_is_simd_convert(const x86_insn_t *i);
static inline int x86_is_simd_shuffle(const x86_insn_t *i);

/* ============================================================================
 * Inline Implementations
 * ============================================================================ */

static inline int x86_is_add(const x86_insn_t *i) {
    /* ADD r/m8, r8 (0x00) | ADD r/m64, r64 (0x01) | ADD r8, r/m8 (0x02) | ADD r64, r/m64 (0x03) */
    /* ADD AL, imm8 (0x04) | ADD RAX, imm32 (0x05) */
    /* ADD r/m64, imm32 (0x81) | ADD r/m64, imm8 (0x83) */
    return (i->opcode >= 0x00 && i->opcode <= 0x05) ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_sub(const x86_insn_t *i) {
    /* SUB r/m8, r8 (0x28) | SUB r/m64, r64 (0x29) | SUB r8, r/m8 (0x2A) | SUB r64, r/m64 (0x2B) */
    /* SUB AL, imm8 (0x2C) | SUB RAX, imm32 (0x2D) */
    /* SUB r/m64, imm32 (0x81) | SUB r/m64, imm8 (0x83) */
    return (i->opcode >= 0x28 && i->opcode <= 0x2D) ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_and(const x86_insn_t *i) {
    /* AND r/m8, r8 (0x20) | AND r/m64, r64 (0x21) | AND r8, r/m8 (0x22) | AND r64, r/m64 (0x23) */
    /* AND AL, imm8 (0x24) | AND RAX, imm32 (0x25) */
    /* AND r/m64, imm32 (0x81) */
    return (i->opcode >= 0x20 && i->opcode <= 0x25) || i->opcode == 0x81;
}
static inline int x86_is_or(const x86_insn_t *i) {
    /* OR r/m8, r8 (0x08) | OR r/m64, r64 (0x09) | OR r8, r/m8 (0x0A) | OR r64, r/m64 (0x0B) */
    /* OR AL, imm8 (0x0C) | OR RAX, imm32 (0x0D) */
    /* OR r/m64, imm32 (0x81) */
    return (i->opcode >= 0x08 && i->opcode <= 0x0D) || i->opcode == 0x81;
}
static inline int x86_is_xor(const x86_insn_t *i) {
    /* XOR r/m8, r8 (0x30) | XOR r/m64, r64 (0x31) | XOR r8, r/m8 (0x32) | XOR r64, r/m64 (0x33) */
    /* XOR AL, imm8 (0x34) | XOR RAX, imm32 (0x35) */
    /* XOR r/m64, imm32 (0x81) */
    return (i->opcode >= 0x30 && i->opcode <= 0x35) || i->opcode == 0x81;
}
static inline int x86_is_mov(const x86_insn_t *i) {
    return i->opcode == 0x8B || i->opcode == 0x89 ||
           i->opcode == 0xC7 || (i->opcode >= 0xB8 && i->opcode <= 0xBF);
}
static inline int x86_is_mov_imm64(const x86_insn_t *i) {
    return (i->opcode >= 0xB8 && i->opcode <= 0xBF) && (i->rex & 0x08);
}
static inline int x86_is_lea(const x86_insn_t *i) {
    return i->opcode == 0x8D;
}
static inline int x86_is_cmp(const x86_insn_t *i) {
    return i->opcode == 0x39 || i->opcode == 0x3B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_test(const x86_insn_t *i) {
    return i->opcode == 0x85 || i->opcode == 0xF6 || i->opcode == 0xF7;
}
static inline int x86_is_jcc(const x86_insn_t *i) {
    return (i->opcode >= 0x70 && i->opcode <= 0x7F) ||
           (i->opcode == 0x0F && i->opcode2 >= 0x80 && i->opcode2 <= 0x8F);
}
static inline int x86_is_jmp(const x86_insn_t *i) {
    return i->opcode == 0xE9 || i->opcode == 0xEB;
}
static inline int x86_is_call(const x86_insn_t *i) {
    return i->opcode == 0xE8;
}
static inline int x86_is_ret(const x86_insn_t *i) {
    return i->opcode == 0xC3 || i->opcode == 0xC2;
}
static inline int x86_is_mul(const x86_insn_t *i) {
    return i->opcode == 0xF7 && (i->reg == 4 || i->reg == 5);
}
static inline int x86_is_div(const x86_insn_t *i) {
    return i->opcode == 0xF7 && (i->reg == 6 || i->reg == 7);
}

/* Shift/Rotate predicates */
static inline int x86_is_shl(const x86_insn_t *i) {
    return i->opcode == 0xD1 || i->opcode == 0xD3 ||
           i->opcode == 0xC1 || (i->opcode == 0xD0 && i->reg == 4);
}
static inline int x86_is_shr(const x86_insn_t *i) {
    return i->opcode == 0xD1 || i->opcode == 0xD3 ||
           i->opcode == 0xC1 || (i->opcode == 0xD0 && i->reg == 5);
}
static inline int x86_is_sar(const x86_insn_t *i) {
    return i->opcode == 0xD1 || i->opcode == 0xD3 ||
           i->opcode == 0xC1 || (i->opcode == 0xD0 && i->reg == 7);
}
static inline int x86_is_rol(const x86_insn_t *i) {
    return i->opcode == 0xD1 || i->opcode == 0xD3 ||
           i->opcode == 0xC1 || (i->opcode == 0xD0 && i->reg == 0);
}
static inline int x86_is_ror(const x86_insn_t *i) {
    return i->opcode == 0xD1 || i->opcode == 0xD3 ||
           i->opcode == 0xC1 || (i->opcode == 0xD0 && i->reg == 1);
}

static inline uint8_t x86_get_jcc_cond(const x86_insn_t *i) {
    if (i->opcode >= 0x70 && i->opcode <= 0x7F)
        return i->opcode & 0x0F;
    if (i->opcode2 >= 0x80 && i->opcode2 <= 0x8F)
        return i->opcode2 & 0x0F;
    return 0;
}

static inline uint8_t x86_get_cmov_cond(const x86_insn_t *i) {
    /* CMOVcc: 0F 40-4F */
    if (i->opcode == 0x0F && i->opcode2 >= 0x40 && i->opcode2 <= 0x4F)
        return i->opcode2 & 0x0F;
    return 0;
}

static inline uint8_t x86_get_setcc_cond(const x86_insn_t *i) {
    /* SETcc: 0F 90-9F */
    if (i->opcode == 0x0F && i->opcode2 >= 0x90 && i->opcode2 <= 0x9F)
        return i->opcode2 & 0x0F;
    return 0;
}

/* P0 - Essential instructions */
static inline int x86_is_movzx(const x86_insn_t *i) {
    return i->opcode == 0x0F && (i->opcode2 == 0xB6 || i->opcode2 == 0xB7);
}
static inline int x86_is_movsx(const x86_insn_t *i) {
    return i->opcode == 0x0F && (i->opcode2 == 0xBE || i->opcode2 == 0xBF);
}
static inline int x86_is_movsxd(const x86_insn_t *i) {
    return i->opcode == 0x63;
}
static inline int x86_is_inc(const x86_insn_t *i) {
    return i->opcode == 0xFF && i->reg == 0;
}
static inline int x86_is_dec(const x86_insn_t *i) {
    return i->opcode == 0xFF && i->reg == 1;
}
static inline int x86_is_neg(const x86_insn_t *i) {
    return i->opcode == 0xF7 && i->reg == 3;
}
static inline int x86_is_not(const x86_insn_t *i) {
    return i->opcode == 0xF7 && i->reg == 2;
}
static inline int x86_is_push(const x86_insn_t *i) {
    /* PUSH r64 (50-57) | PUSH r/m64 (FF /6) | PUSH imm32 (68) | PUSH imm8 (6A) */
    if (i->opcode >= 0x50 && i->opcode <= 0x57) return 1;
    if (i->opcode == 0x68 || i->opcode == 0x6A) return 1;
    if (i->opcode == 0xFF && i->reg == 6) return 1;
    return 0;
}
static inline int x86_is_pop(const x86_insn_t *i) {
    /* POP r64 (58-5F) | POP r/m64 (8F /0) */
    if (i->opcode >= 0x58 && i->opcode <= 0x5F) return 1;
    if (i->opcode == 0x8F && i->reg == 0) return 1;
    return 0;
}
static inline int x86_is_nop(const x86_insn_t *i) {
    return i->opcode == 0x90;
}

/* P1 - Control flow instructions */
static inline int x86_is_cmov(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 >= 0x40 && i->opcode2 <= 0x4F;
}
static inline int x86_is_setcc(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 >= 0x90 && i->opcode2 <= 0x9F;
}
static inline int x86_is_xchg(const x86_insn_t *i) {
    return i->opcode == 0x87 || (i->opcode >= 0x90 && i->opcode <= 0x97);
}

/* P2 - Bit manipulation instructions */
static inline int x86_is_bsf(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xBC;
}
static inline int x86_is_bsr(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xBD;
}
static inline int x86_is_popcnt(const x86_insn_t *i) {
    return i->opcode == 0xF3 && i->opcode2 == 0xB8;
}
static inline int x86_is_bt(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xA3;
}
static inline int x86_is_bts(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xAB;
}
static inline int x86_is_btr(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xB3;
}
static inline int x86_is_btc(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xBB;
}
static inline int x86_is_tzcnt(const x86_insn_t *i) {
    return i->opcode == 0xF3 && i->opcode2 == 0xBC;
}
static inline int x86_is_lzcnt(const x86_insn_t *i) {
    return i->opcode == 0xF3 && i->opcode2 == 0xBD;
}

/* P3 - String operations */
static inline int x86_is_movs(const x86_insn_t *i) {
    return i->opcode == 0xA4 || i->opcode == 0xA5;
}
static inline int x86_is_stos(const x86_insn_t *i) {
    return i->opcode == 0xAA || i->opcode == 0xAB;
}
static inline int x86_is_lods(const x86_insn_t *i) {
    return i->opcode == 0xAC || i->opcode == 0xAD;
}
static inline int x86_is_cmps(const x86_insn_t *i) {
    return i->opcode == 0xA6 || i->opcode == 0xA7;
}
static inline int x86_is_scas(const x86_insn_t *i) {
    return i->opcode == 0xAE || i->opcode == 0xAF;
}
static inline int x86_has_rep_prefix(const x86_insn_t *i) {
    return i->rex & 0x40;  /* Using rex field for rep prefix flag */
}

/* P4 - Special instructions */
static inline int x86_is_cpuid(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0xA2;
}
static inline int x86_is_rdtsc(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0x31;
}
static inline int x86_is_rdtscp(const x86_insn_t *i) {
    return i->opcode == 0x0F && i->opcode2 == 0x33;
}
static inline int x86_is_shld(const x86_insn_t *i) {
    return i->opcode == 0x0F && (i->opcode2 == 0xA4 || i->opcode2 == 0xA5);
}
static inline int x86_is_shrd(const x86_insn_t *i) {
    return i->opcode == 0x0F && (i->opcode2 == 0xAC || i->opcode2 == 0xAD);
}
static inline int x86_is_cwd(const x86_insn_t *i) {
    return i->opcode == 0x99;
}
static inline int x86_is_cqo(const x86_insn_t *i) {
    return i->opcode == 0x48 && i->opcode2 == 0x99;
}
static inline int x86_is_cli(const x86_insn_t *i) {
    return i->opcode == 0xFA;
}
static inline int x86_is_sti(const x86_insn_t *i) {
    return i->opcode == 0xFB;
}
static inline int x86_is_cli_sti(const x86_insn_t *i) {
    return i->opcode == 0xFA || i->opcode == 0xFB;
}

/* ============================================================================
 * SIMD/Floating Point Predicate Implementations
 * ============================================================================ */

/* Check if instruction has SIMD prefix */
static inline int x86_has_simd_prefix(const x86_insn_t *i) {
    return i->simd_prefix == 0x66 || i->simd_prefix == 0xF2 || i->simd_prefix == 0xF3;
}

/* SSE/SSE2 SIMD instructions (0F opcode escape) */
static inline int x86_is_movaps(const x86_insn_t *i) {
    return i->opcode2 == 0x28 || i->opcode2 == 0x29;
}
static inline int x86_is_movups(const x86_insn_t *i) {
    return i->opcode2 == 0x10 || i->opcode2 == 0x11;
}
static inline int x86_is_movss(const x86_insn_t *i) {
    return i->opcode2 == 0x10 && i->simd_prefix == 0xF3;
}
static inline int x86_is_movsd(const x86_insn_t *i) {
    return i->opcode2 == 0x10 && i->simd_prefix == 0xF2;
}
static inline int x86_is_movapd(const x86_insn_t *i) {
    return i->opcode2 == 0x28 && i->simd_prefix == 0x66;
}
static inline int x86_is_movupd(const x86_insn_t *i) {
    return i->opcode2 == 0x10 && i->simd_prefix == 0x66;
}
static inline int x86_is_movhlps(const x86_insn_t *i) {
    return i->opcode2 == 0x12;
}
static inline int x86_is_movlhps(const x86_insn_t *i) {
    return i->opcode2 == 0x16;
}
static inline int x86_is_movhps(const x86_insn_t *i) {
    return i->opcode2 == 0x16;
}
static inline int x86_is_movhpd(const x86_insn_t *i) {
    return i->opcode2 == 0x16 && i->simd_prefix == 0x66;
}
static inline int x86_is_movlps(const x86_insn_t *i) {
    return i->opcode2 == 0x12 && i->simd_prefix == 0x66;
}
static inline int x86_is_movlpd(const x86_insn_t *i) {
    return i->opcode2 == 0x12 && i->simd_prefix == 0x66;
}

/* Arithmetic SIMD instructions */
static inline int x86_is_addps(const x86_insn_t *i) {
    return i->opcode2 == 0x58 && i->simd_prefix == 0x00;
}
static inline int x86_is_addpd(const x86_insn_t *i) {
    return i->opcode2 == 0x58 && i->simd_prefix == 0x66;
}
static inline int x86_is_addss(const x86_insn_t *i) {
    return i->opcode2 == 0x58 && i->simd_prefix == 0xF3;
}
static inline int x86_is_addsd(const x86_insn_t *i) {
    return i->opcode2 == 0x58 && i->simd_prefix == 0xF2;
}
static inline int x86_is_subps(const x86_insn_t *i) {
    return i->opcode2 == 0x5C && i->simd_prefix == 0x00;
}
static inline int x86_is_subpd(const x86_insn_t *i) {
    return i->opcode2 == 0x5C && i->simd_prefix == 0x66;
}
static inline int x86_is_subss(const x86_insn_t *i) {
    return i->opcode2 == 0x5C && i->simd_prefix == 0xF3;
}
static inline int x86_is_subsd(const x86_insn_t *i) {
    return i->opcode2 == 0x5C && i->simd_prefix == 0xF2;
}
static inline int x86_is_mulps(const x86_insn_t *i) {
    return i->opcode2 == 0x59 && i->simd_prefix == 0x00;
}
static inline int x86_is_mulpd(const x86_insn_t *i) {
    return i->opcode2 == 0x59 && i->simd_prefix == 0x66;
}
static inline int x86_is_mulss(const x86_insn_t *i) {
    return i->opcode2 == 0x59 && i->simd_prefix == 0xF3;
}
static inline int x86_is_mulsd(const x86_insn_t *i) {
    return i->opcode2 == 0x59 && i->simd_prefix == 0xF2;
}
static inline int x86_is_divps(const x86_insn_t *i) {
    return i->opcode2 == 0x5E && i->simd_prefix == 0x00;
}
static inline int x86_is_divpd(const x86_insn_t *i) {
    return i->opcode2 == 0x5E && i->simd_prefix == 0x66;
}
static inline int x86_is_divss(const x86_insn_t *i) {
    return i->opcode2 == 0x5E && i->simd_prefix == 0xF3;
}
static inline int x86_is_divsd(const x86_insn_t *i) {
    return i->opcode2 == 0x5E && i->simd_prefix == 0xF2;
}
static inline int x86_is_maxps(const x86_insn_t *i) {
    return i->opcode2 == 0x5F && i->simd_prefix == 0x00;
}
static inline int x86_is_maxpd(const x86_insn_t *i) {
    return i->opcode2 == 0x5F && i->simd_prefix == 0x66;
}
static inline int x86_is_maxss(const x86_insn_t *i) {
    return i->opcode2 == 0x5F && i->simd_prefix == 0xF3;
}
static inline int x86_is_maxsd(const x86_insn_t *i) {
    return i->opcode2 == 0x5F && i->simd_prefix == 0xF2;
}
static inline int x86_is_minps(const x86_insn_t *i) {
    return i->opcode2 == 0x5D && i->simd_prefix == 0x00;
}
static inline int x86_is_minpd(const x86_insn_t *i) {
    return i->opcode2 == 0x5D && i->simd_prefix == 0x66;
}
static inline int x86_is_minss(const x86_insn_t *i) {
    return i->opcode2 == 0x5D && i->simd_prefix == 0xF3;
}
static inline int x86_is_minsd(const x86_insn_t *i) {
    return i->opcode2 == 0x5D && i->simd_prefix == 0xF2;
}

/* Logical SIMD instructions */
static inline int x86_is_andps(const x86_insn_t *i) {
    return i->opcode2 == 0x54;
}
static inline int x86_is_andpd(const x86_insn_t *i) {
    return i->opcode2 == 0x54 && i->simd_prefix == 0x66;
}
static inline int x86_is_andnps(const x86_insn_t *i) {
    return i->opcode2 == 0x55;
}
static inline int x86_is_andnpd(const x86_insn_t *i) {
    return i->opcode2 == 0x55 && i->simd_prefix == 0x66;
}
static inline int x86_is_orps(const x86_insn_t *i) {
    return i->opcode2 == 0x56;
}
static inline int x86_is_orpd(const x86_insn_t *i) {
    return i->opcode2 == 0x56 && i->simd_prefix == 0x66;
}
static inline int x86_is_xorps(const x86_insn_t *i) {
    return i->opcode2 == 0x57;
}
static inline int x86_is_xorpd(const x86_insn_t *i) {
    return i->opcode2 == 0x57 && i->simd_prefix == 0x66;
}

/* Comparison SIMD instructions */
static inline int x86_is_cmpps(const x86_insn_t *i) {
    return i->opcode2 == 0xC2 && i->simd_prefix == 0x00;
}
static inline int x86_is_cmppd(const x86_insn_t *i) {
    return i->opcode2 == 0xC2 && i->simd_prefix == 0x66;
}
static inline int x86_is_cmpss(const x86_insn_t *i) {
    return i->opcode2 == 0xC2 && i->simd_prefix == 0xF3;
}
static inline int x86_is_cmpsd(const x86_insn_t *i) {
    return i->opcode2 == 0xC2 && i->simd_prefix == 0xF2;
}
static inline int x86_is_ucomiss(const x86_insn_t *i) {
    return i->opcode2 == 0x2E;
}
static inline int x86_is_ucomisd(const x86_insn_t *i) {
    return i->opcode2 == 0x2E && i->simd_prefix == 0x66;
}
static inline int x86_is_comiss(const x86_insn_t *i) {
    return i->opcode2 == 0x2F;
}
static inline int x86_is_comisd(const x86_insn_t *i) {
    return i->opcode2 == 0x2F && i->simd_prefix == 0x66;
}

/* Conversion instructions */
static inline int x86_is_cvtps2pd(const x86_insn_t *i) {
    return i->opcode2 == 0x5A && i->simd_prefix == 0x00;
}
static inline int x86_is_cvtpd2ps(const x86_insn_t *i) {
    return i->opcode2 == 0x5A && i->simd_prefix == 0x66;
}
static inline int x86_is_cvtss2sd(const x86_insn_t *i) {
    return i->opcode2 == 0x5A && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvtsd2ss(const x86_insn_t *i) {
    return i->opcode2 == 0x5A && i->simd_prefix == 0xF2;
}
static inline int x86_is_cvtdq2ps(const x86_insn_t *i) {
    return i->opcode2 == 0x5B && i->simd_prefix == 0x00;
}
static inline int x86_is_cvtps2dq(const x86_insn_t *i) {
    return i->opcode2 == 0x5B && i->simd_prefix == 0x66;
}
static inline int x86_is_cvttps2dq(const x86_insn_t *i) {
    return i->opcode2 == 0x5B && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvtdq2pd(const x86_insn_t *i) {
    return i->opcode2 == 0xE6 && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvtpd2dq(const x86_insn_t *i) {
    return i->opcode2 == 0xE6 && i->simd_prefix == 0xF2;
}
static inline int x86_is_cvttpd2dq(const x86_insn_t *i) {
    return i->opcode2 == 0xE6 && i->simd_prefix == 0x66;
}
static inline int x86_is_cvtsi2ss(const x86_insn_t *i) {
    return i->opcode2 == 0x2C && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvtsi2sd(const x86_insn_t *i) {
    return i->opcode2 == 0x2C && i->simd_prefix == 0xF2;
}
static inline int x86_is_cvtss2si(const x86_insn_t *i) {
    return i->opcode2 == 0x2D && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvtsd2si(const x86_insn_t *i) {
    return i->opcode2 == 0x2D && i->simd_prefix == 0xF2;
}
static inline int x86_is_cvttss2si(const x86_insn_t *i) {
    return i->opcode2 == 0x2C && i->simd_prefix == 0xF3;
}
static inline int x86_is_cvttsd2si(const x86_insn_t *i) {
    return i->opcode2 == 0x2C && i->simd_prefix == 0xF2;
}

/* Square root and reciprocal */
static inline int x86_is_sqrtps(const x86_insn_t *i) {
    return i->opcode2 == 0x51 && i->simd_prefix == 0x00;
}
static inline int x86_is_sqrtpd(const x86_insn_t *i) {
    return i->opcode2 == 0x51 && i->simd_prefix == 0x66;
}
static inline int x86_is_sqrtss(const x86_insn_t *i) {
    return i->opcode2 == 0x51 && i->simd_prefix == 0xF3;
}
static inline int x86_is_sqrtsd(const x86_insn_t *i) {
    return i->opcode2 == 0x51 && i->simd_prefix == 0xF2;
}
static inline int x86_is_rsqrtps(const x86_insn_t *i) {
    return i->opcode2 == 0x52;
}
static inline int x86_is_rsqrtss(const x86_insn_t *i) {
    return i->opcode2 == 0x52 && i->simd_prefix == 0xF3;
}
static inline int x86_is_rcpps(const x86_insn_t *i) {
    return i->opcode2 == 0x53;
}
static inline int x86_is_rcpss(const x86_insn_t *i) {
    return i->opcode2 == 0x53 && i->simd_prefix == 0xF3;
}

/* MXCSR control */
static inline int x86_is_ldmxcsr(const x86_insn_t *i) {
    return i->opcode2 == 0xAE && i->modrm == 0x1A;
}
static inline int x86_is_stmxcsr(const x86_insn_t *i) {
    return i->opcode2 == 0xAE && i->modrm == 0x1E;
}

/* Integer SIMD instructions */
static inline int x86_is_movdqu(const x86_insn_t *i) {
    return i->opcode2 == 0x6F && i->simd_prefix == 0xF3;
}
static inline int x86_is_movdqa(const x86_insn_t *i) {
    return i->opcode2 == 0x6F && i->simd_prefix == 0x66;
}
static inline int x86_is_movq(const x86_insn_t *i) {
    return i->opcode2 == 0x7E && i->simd_prefix == 0xF3;
}
static inline int x86_is_pshufd(const x86_insn_t *i) {
    return i->opcode2 == 0x70 && i->simd_prefix == 0x66;
}
static inline int x86_is_pshuflw(const x86_insn_t *i) {
    return i->opcode2 == 0x70 && i->simd_prefix == 0xF2;
}
static inline int x86_is_pshufhw(const x86_insn_t *i) {
    return i->opcode2 == 0x70 && i->simd_prefix == 0xF3;
}
static inline int x86_is_punpckldq(const x86_insn_t *i) {
    return i->opcode2 == 0x6C && i->simd_prefix == 0x66;
}
static inline int x86_is_punpckhdq(const x86_insn_t *i) {
    return i->opcode2 == 0x6D && i->simd_prefix == 0x66;
}
static inline int x86_is_punpcklqdq(const x86_insn_t *i) {
    return i->opcode2 == 0x6C && i->simd_prefix == 0x66;
}
static inline int x86_is_punpckhqdq(const x86_insn_t *i) {
    return i->opcode2 == 0x6D && i->simd_prefix == 0x66;
}
static inline int x86_is_pmovmskb(const x86_insn_t *i) {
    return i->opcode2 == 0xD7;
}
static inline int x86_is_pextrw(const x86_insn_t *i) {
    return i->opcode2 == 0xC5;
}
static inline int x86_is_pinsrw(const x86_insn_t *i) {
    return i->opcode2 == 0xC4;
}
static inline int x86_is_pshufw(const x86_insn_t *i) {
    return i->opcode2 == 0x70 && i->simd_prefix == 0x00;
}
static inline int x86_is_paddb(const x86_insn_t *i) {
    return i->opcode2 == 0xFC && i->simd_prefix == 0x66;
}
static inline int x86_is_paddw(const x86_insn_t *i) {
    return i->opcode2 == 0xFD && i->simd_prefix == 0x66;
}
static inline int x86_is_paddd(const x86_insn_t *i) {
    return i->opcode2 == 0xFE && i->simd_prefix == 0x66;
}
static inline int x86_is_paddq(const x86_insn_t *i) {
    return i->opcode2 == 0xD4 && i->simd_prefix == 0x66;
}
static inline int x86_is_psubb(const x86_insn_t *i) {
    return i->opcode2 == 0xF8 && i->simd_prefix == 0x66;
}
static inline int x86_is_psubw(const x86_insn_t *i) {
    return i->opcode2 == 0xF9 && i->simd_prefix == 0x66;
}
static inline int x86_is_psubd(const x86_insn_t *i) {
    return i->opcode2 == 0xFA && i->simd_prefix == 0x66;
}
static inline int x86_is_psubq(const x86_insn_t *i) {
    return i->opcode2 == 0xFB && i->simd_prefix == 0x66;
}
static inline int x86_is_pand(const x86_insn_t *i) {
    return i->opcode2 == 0xDB && i->simd_prefix == 0x66;
}
static inline int x86_is_pandn(const x86_insn_t *i) {
    return i->opcode2 == 0xDF && i->simd_prefix == 0x66;
}
static inline int x86_is_por(const x86_insn_t *i) {
    return i->opcode2 == 0xEB && i->simd_prefix == 0x66;
}
static inline int x86_is_pxor(const x86_insn_t *i) {
    return i->opcode2 == 0xEF && i->simd_prefix == 0x66;
}
static inline int x86_is_pcmpgtb(const x86_insn_t *i) {
    return i->opcode2 == 0x64 && i->simd_prefix == 0x66;
}
static inline int x86_is_pcmpgtw(const x86_insn_t *i) {
    return i->opcode2 == 0x65 && i->simd_prefix == 0x66;
}
static inline int x86_is_pcmpgtd(const x86_insn_t *i) {
    return i->opcode2 == 0x66 && i->simd_prefix == 0x66;
}
static inline int x86_is_pcmpeq(const x86_insn_t *i) {
    return i->opcode2 == 0x74 || i->opcode2 == 0x75 || i->opcode2 == 0x76;
}
static inline int x86_is_pminub(const x86_insn_t *i) {
    return i->opcode2 == 0xDA;
}
static inline int x86_is_pmaxub(const x86_insn_t *i) {
    return i->opcode2 == 0xDE;
}
static inline int x86_is_pminsw(const x86_insn_t *i) {
    return i->opcode2 == 0xEA;
}
static inline int x86_is_pmaxsw(const x86_insn_t *i) {
    return i->opcode2 == 0xEE;
}
static inline int x86_is_pmullw(const x86_insn_t *i) {
    return i->opcode2 == 0xD5 && i->simd_prefix == 0x66;
}
static inline int x86_is_pmulhw(const x86_insn_t *i) {
    return i->opcode2 == 0xE5 && i->simd_prefix == 0x66;
}
static inline int x86_is_pmulhuw(const x86_insn_t *i) {
    return i->opcode2 == 0xE4 && i->simd_prefix == 0x66;
}
static inline int x86_is_pavgb(const x86_insn_t *i) {
    return i->opcode2 == 0xE0;
}
static inline int x86_is_pavgw(const x86_insn_t *i) {
    return i->opcode2 == 0xE3;
}
static inline int x86_is_psadbw(const x86_insn_t *i) {
    return i->opcode2 == 0xF6;
}

/* SSE4 instructions */
static inline int x86_is_pmovmskb2(const x86_insn_t *i) {
    return i->opcode2 == 0xD7 && i->simd_prefix == 0x66;
}
static inline int x86_is_pextrb(const x86_insn_t *i) {
    return i->opcode2 == 0x14 && i->simd_prefix == 0x66;
}
static inline int x86_is_pextrd(const x86_insn_t *i) {
    return i->opcode2 == 0x16 && i->simd_prefix == 0x66;
}
static inline int x86_is_insertps(const x86_insn_t *i) {
    return i->opcode2 == 0x21 && i->simd_prefix == 0x66;
}
static inline int x86_is_roundps(const x86_insn_t *i) {
    return i->opcode2 == 0x0C && i->simd_prefix == 0x66;
}
static inline int x86_is_roundpd(const x86_insn_t *i) {
    return i->opcode2 == 0x0D && i->simd_prefix == 0x66;
}
static inline int x86_is_roundss(const x86_insn_t *i) {
    return i->opcode2 == 0x0A && i->simd_prefix == 0x66;
}
static inline int x86_is_roundsd(const x86_insn_t *i) {
    return i->opcode2 == 0x0B && i->simd_prefix == 0x66;
}
static inline int x86_is_blendvps(const x86_insn_t *i) {
    return i->opcode2 == 0x50 && i->simd_prefix == 0x66;
}
static inline int x86_is_blendvpd(const x86_insn_t *i) {
    return i->opcode2 == 0x50 && i->simd_prefix == 0x66;
}
static inline int x86_is_dppd(const x86_insn_t *i) {
    return i->opcode2 == 0x41 && i->simd_prefix == 0x66;
}
static inline int x86_is_dpps(const x86_insn_t *i) {
    return i->opcode2 == 0x40 && i->simd_prefix == 0x66;
}

/* Helper predicates */
static inline int x86_is_simd(const x86_insn_t *i) {
    return i->opcode2 >= 0x10 && i->opcode2 <= 0x7F;
}

static inline int x86_is_fp(const x86_insn_t *i) {
    return x86_is_addps(i) || x86_is_addpd(i) || x86_is_addss(i) || x86_is_addsd(i) ||
           x86_is_subps(i) || x86_is_subpd(i) || x86_is_subss(i) || x86_is_subsd(i) ||
           x86_is_mulps(i) || x86_is_mulpd(i) || x86_is_mulss(i) || x86_is_mulsd(i) ||
           x86_is_divps(i) || x86_is_divpd(i) || x86_is_divss(i) || x86_is_divsd(i) ||
           x86_is_sqrtps(i) || x86_is_sqrtpd(i) || x86_is_sqrtss(i) || x86_is_sqrtsd(i);
}

static inline int x86_is_simd_mov(const x86_insn_t *i) {
    return x86_is_movaps(i) || x86_is_movups(i) || x86_is_movss(i) || x86_is_movsd(i) ||
           x86_is_movapd(i) || x86_is_movupd(i) || x86_is_movhlps(i) || x86_is_movlhps(i) ||
           x86_is_movhps(i) || x86_is_movhpd(i) || x86_is_movlps(i) || x86_is_movlpd(i) ||
           x86_is_movdqu(i) || x86_is_movdqa(i) || x86_is_movq(i);
}

static inline int x86_is_simd_arith(const x86_insn_t *i) {
    return x86_is_addps(i) || x86_is_addpd(i) || x86_is_addss(i) || x86_is_addsd(i) ||
           x86_is_subps(i) || x86_is_subpd(i) || x86_is_subss(i) || x86_is_subsd(i) ||
           x86_is_mulps(i) || x86_is_mulpd(i) || x86_is_mulss(i) || x86_is_mulsd(i) ||
           x86_is_divps(i) || x86_is_divpd(i) || x86_is_divss(i) || x86_is_divsd(i) ||
           x86_is_maxps(i) || x86_is_maxpd(i) || x86_is_maxss(i) || x86_is_maxsd(i) ||
           x86_is_minps(i) || x86_is_minpd(i) || x86_is_minss(i) || x86_is_minsd(i);
}

static inline int x86_is_simd_cmp(const x86_insn_t *i) {
    return x86_is_cmpps(i) || x86_is_cmppd(i) || x86_is_cmpss(i) || x86_is_cmpsd(i) ||
           x86_is_ucomiss(i) || x86_is_ucomisd(i) || x86_is_comiss(i) || x86_is_comisd(i);
}

static inline int x86_is_simd_convert(const x86_insn_t *i) {
    return x86_is_cvtps2pd(i) || x86_is_cvtpd2ps(i) || x86_is_cvtss2sd(i) || x86_is_cvtsd2ss(i) ||
           x86_is_cvtdq2ps(i) || x86_is_cvtps2dq(i) || x86_is_cvttps2dq(i) ||
           x86_is_cvtdq2pd(i) || x86_is_cvtpd2dq(i) || x86_is_cvttpd2dq(i) ||
           x86_is_cvtsi2ss(i) || x86_is_cvtsi2sd(i) || x86_is_cvtss2si(i) || x86_is_cvtsd2si(i) ||
           x86_is_cvttss2si(i) || x86_is_cvttsd2si(i);
}

static inline int x86_is_simd_shuffle(const x86_insn_t *i) {
    return x86_is_pshufd(i) || x86_is_pshuflw(i) || x86_is_pshufhw(i) ||
           x86_is_pshufw(i) || x86_is_punpckldq(i) || x86_is_punpckhdq(i) ||
           x86_is_punpcklqdq(i) || x86_is_punpckhqdq(i);
}

/**
 * Map x86 condition code to ARM64 condition code
 * @param x86_jcc x86 condition code (0-15)
 * @return ARM64 condition code
 */
static inline uint8_t map_x86_jcc_to_arm_cond(uint8_t x86_jcc)
{
    switch (x86_jcc & 0x0F) {
        case 0x0: return 6;   /* JO -> VS */
        case 0x1: return 7;   /* JNO -> VC */
        case 0x2: return 3;   /* JB/JNAE/CF -> CC */
        case 0x3: return 2;   /* JAE/JNB/NC -> CS */
        case 0x4: return 0;   /* JE/JZ -> EQ */
        case 0x5: return 1;   /* JNE/JNZ -> NE */
        case 0x6: return 10;  /* JBE/JNA -> LE */
        case 0x7: return 11;  /* JA/JNBE -> GT */
        case 0x8: return 4;   /* JS -> MI */
        case 0x9: return 5;   /* JNS -> PL */
        case 0xA: return 6;   /* JP/JPE -> VS */
        case 0xB: return 7;   /* JNP/JPO -> VC */
        case 0xC: return 8;   /* JL/JNGE -> LT */
        case 0xD: return 9;   /* JGE/JNL -> GE */
        case 0xE: return 10;  /* JLE/JNG -> LE */
        case 0xF: return 11;  /* JG/JNLE -> GT */
        default: return 14;
    }
}

#endif /* ROSETTA_X86_DECODE_H */
