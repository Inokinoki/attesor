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

/* ============================================================================
 * Inline Implementations
 * ============================================================================ */

static inline int x86_is_add(const x86_insn_t *i) {
    return i->opcode == 0x01 || i->opcode == 0x03 ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_sub(const x86_insn_t *i) {
    return i->opcode == 0x29 || i->opcode == 0x2B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}
static inline int x86_is_and(const x86_insn_t *i) {
    return i->opcode == 0x21 || i->opcode == 0x23 || i->opcode == 0x81;
}
static inline int x86_is_or(const x86_insn_t *i) {
    return i->opcode == 0x09 || i->opcode == 0x0B || i->opcode == 0x81;
}
static inline int x86_is_xor(const x86_insn_t *i) {
    return i->opcode == 0x31 || i->opcode == 0x33 || i->opcode == 0x81;
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
    return i->opcode == 0x50 || i->opcode == 0xFF;
}
static inline int x86_is_pop(const x86_insn_t *i) {
    return i->opcode == 0x58 || i->opcode == 0x8F;
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
