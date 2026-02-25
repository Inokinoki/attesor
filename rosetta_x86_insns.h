/* ============================================================================
 * Rosetta Translator - x86 Instruction Decoding Header
 * ============================================================================
 *
 * x86_64 instruction decoding helpers and predicates.
 * ============================================================================ */

#ifndef ROSETTA_X86_INSNS_H
#define ROSETTA_X86_INSNS_H

#include <stdint.h>
#include <string.h>

/* ============================================================================
 * x86 Instruction Structure
 * ============================================================================ */

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
 * Register Constants
 * ============================================================================ */

#ifndef X86_RAX
#define X86_RAX 0
#endif
#ifndef X86_RCX
#define X86_RCX 1
#endif
#ifndef X86_RDX
#define X86_RDX 2
#endif
#ifndef X86_RBX
#define X86_RBX 3
#endif
#ifndef X86_RSP
#define X86_RSP 4
#endif
#ifndef X86_RBP
#define X86_RBP 5
#endif
#ifndef X86_RSI
#define X86_RSI 6
#endif
#ifndef X86_RDI
#define X86_RDI 7
#endif
#ifndef X86_R8
#define X86_R8  8
#endif
#ifndef X86_R9
#define X86_R9  9
#endif
#ifndef X86_R10
#define X86_R10 10
#endif
#ifndef X86_R11
#define X86_R11 11
#endif
#ifndef X86_R12
#define X86_R12 12
#endif
#ifndef X86_R13
#define X86_R13 13
#endif
#ifndef X86_R14
#define X86_R14 14
#endif
#ifndef X86_R15
#define X86_R15 15
#endif

/* ============================================================================
 * Instruction Decoding
 * ============================================================================ */

int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn);

/* ============================================================================
 * Condition Code Mapping
 * ============================================================================ */

uint8_t map_x86_jcc_to_arm_cond(uint8_t x86_jcc);

/* ============================================================================
 * ALU Instruction Predicates
 * ============================================================================ */

static inline int x86_is_add(const x86_insn_t *i)
{
    return i->opcode == 0x01 || i->opcode == 0x03 ||
           i->opcode == 0x81 || i->opcode == 0x83;
}

static inline int x86_is_sub(const x86_insn_t *i)
{
    return i->opcode == 0x29 || i->opcode == 0x2B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}

static inline int x86_is_and(const x86_insn_t *i)
{
    return i->opcode == 0x21 || i->opcode == 0x23 || i->opcode == 0x81;
}

static inline int x86_is_or(const x86_insn_t *i)
{
    return i->opcode == 0x09 || i->opcode == 0x0B || i->opcode == 0x81;
}

static inline int x86_is_xor(const x86_insn_t *i)
{
    return i->opcode == 0x31 || i->opcode == 0x33 || i->opcode == 0x81;
}

static inline int x86_is_cmp(const x86_insn_t *i)
{
    return i->opcode == 0x39 || i->opcode == 0x3B ||
           i->opcode == 0x81 || i->opcode == 0x83;
}

static inline int x86_is_test(const x86_insn_t *i)
{
    return i->opcode == 0x85 || i->opcode == 0xF6 || i->opcode == 0xF7;
}

static inline int x86_is_mul(const x86_insn_t *i)
{
    return i->opcode == 0xF7 && (i->reg == 4 || i->reg == 5);
}

static inline int x86_is_div(const x86_insn_t *i)
{
    return i->opcode == 0xF7 && (i->reg == 6 || i->reg == 7);
}

static inline int x86_is_inc(const x86_insn_t *i)
{
    if (i->opcode == 0xFE && i->reg == 0) return 1;
    if (i->opcode == 0xFF && i->reg == 0) return 1;
    if ((i->opcode >= 0x40 && i->opcode <= 0x47) ||
        (i->opcode >= 0x48 && i->opcode <= 0x4F)) return 1;
    return 0;
}

static inline int x86_is_dec(const x86_insn_t *i)
{
    if (i->opcode == 0xFE && i->reg == 1) return 1;
    if (i->opcode == 0xFF && i->reg == 1) return 1;
    if ((i->opcode >= 0x48 && i->opcode <= 0x4F) ||
        (i->opcode >= 0x50 && i->opcode <= 0x57)) return 1;
    return 0;
}

static inline int x86_is_neg(const x86_insn_t *i)
{
    return (i->opcode == 0xF6 || i->opcode == 0xF7) && i->reg == 3;
}

static inline int x86_is_not(const x86_insn_t *i)
{
    return (i->opcode == 0xF6 || i->opcode == 0xF7) && i->reg == 2;
}

/* ============================================================================
 * Shift/Rotate Instruction Predicates
 * ============================================================================ */

static inline int x86_is_shl(const x86_insn_t *i)
{
    return (i->opcode == 0xD0 || i->opcode == 0xD1 ||
            i->opcode == 0xD2 || i->opcode == 0xD3 ||
            i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 4;
}

static inline int x86_is_shr(const x86_insn_t *i)
{
    return (i->opcode == 0xD0 || i->opcode == 0xD1 ||
            i->opcode == 0xD2 || i->opcode == 0xD3 ||
            i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 5;
}

static inline int x86_is_sar(const x86_insn_t *i)
{
    return (i->opcode == 0xD0 || i->opcode == 0xD1 ||
            i->opcode == 0xD2 || i->opcode == 0xD3 ||
            i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 7;
}

static inline int x86_is_rol(const x86_insn_t *i)
{
    return (i->opcode == 0xD0 || i->opcode == 0xD1 ||
            i->opcode == 0xD2 || i->opcode == 0xD3 ||
            i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 0;
}

static inline int x86_is_ror(const x86_insn_t *i)
{
    return (i->opcode == 0xD0 || i->opcode == 0xD1 ||
            i->opcode == 0xD2 || i->opcode == 0xD3 ||
            i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 1;
}

/* ============================================================================
 * Memory Instruction Predicates
 * ============================================================================ */

static inline int x86_is_mov(const x86_insn_t *i)
{
    return i->opcode == 0x8B || i->opcode == 0x89 ||
           i->opcode == 0xC7 || (i->opcode >= 0xB8 && i->opcode <= 0xBF);
}

static inline int x86_is_mov_imm64(const x86_insn_t *i)
{
    return (i->opcode >= 0xB8 && i->opcode <= 0xBF) && (i->rex & 0x08);
}

static inline int x86_is_movzx(const x86_insn_t *i)
{
    return i->opcode == 0x0F && (i->opcode2 == 0xB6 || i->opcode2 == 0xB7);
}

static inline int x86_is_movsx(const x86_insn_t *i)
{
    return i->opcode == 0x0F && (i->opcode2 == 0xBE || i->opcode2 == 0xBF);
}

static inline int x86_is_movsxd(const x86_insn_t *i)
{
    return i->opcode == 0x63 && (i->rex & 0x08);
}

static inline int x86_is_lea(const x86_insn_t *i)
{
    return i->opcode == 0x8D;
}

static inline int x86_is_push(const x86_insn_t *i)
{
    if (i->opcode >= 0x50 && i->opcode <= 0x57) return 1;
    if (i->opcode == 0x6A || i->opcode == 0x68) return 1;
    if (i->opcode == 0xFF && i->reg == 6) return 1;
    return 0;
}

static inline int x86_is_pop(const x86_insn_t *i)
{
    if (i->opcode >= 0x58 && i->opcode <= 0x5F) return 1;
    if (i->opcode == 0x8F && i->reg == 0) return 1;
    return 0;
}

/* ============================================================================
 * Branch Instruction Predicates
 * ============================================================================ */

static inline int x86_is_jmp(const x86_insn_t *i)
{
    return i->opcode == 0xE9 || i->opcode == 0xEB;
}

static inline int x86_is_call(const x86_insn_t *i)
{
    return i->opcode == 0xE8;
}

static inline int x86_is_ret(const x86_insn_t *i)
{
    return i->opcode == 0xC3 || i->opcode == 0xC2;
}

static inline int x86_is_jcc(const x86_insn_t *i)
{
    if (i->opcode >= 0x70 && i->opcode <= 0x7F) return 1;
    if (i->opcode == 0x0F && i->opcode2 >= 0x80 && i->opcode2 <= 0x8F) return 1;
    return 0;
}

static inline int x86_is_cmov(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 >= 0x40 && i->opcode2 <= 0x4F;
}

static inline int x86_is_setcc(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 >= 0x90 && i->opcode2 <= 0x9F;
}

static inline int x86_is_xchg(const x86_insn_t *i)
{
    if (i->opcode == 0x86 || i->opcode == 0x87) return 1;
    if (i->opcode >= 0x90 && i->opcode <= 0x97) return 1;
    return 0;
}

static inline uint8_t x86_get_jcc_cond(const x86_insn_t *i)
{
    if (i->opcode >= 0x70 && i->opcode <= 0x7F)
        return i->opcode & 0x0F;
    if (i->opcode2 >= 0x80 && i->opcode2 <= 0x8F)
        return i->opcode2 & 0x0F;
    return 0;
}

static inline uint8_t x86_get_cmov_cond(const x86_insn_t *i)
{
    if (x86_is_cmov(i))
        return i->opcode2 - 0x40;
    return 0;
}

static inline uint8_t x86_get_setcc_cond(const x86_insn_t *i)
{
    if (x86_is_setcc(i))
        return i->opcode2 - 0x90;
    return 0;
}

/* ============================================================================
 * Bit Manipulation Instruction Predicates
 * ============================================================================ */

static inline int x86_is_bsf(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xBC;
}

static inline int x86_is_bsr(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xBD;
}

static inline int x86_is_bt(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xA3;
}

static inline int x86_is_bts(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xAB;
}

static inline int x86_is_btr(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xB3;
}

static inline int x86_is_btc(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xBB;
}

static inline int x86_is_popcnt(const x86_insn_t *i)
{
    return i->opcode == 0xF3 && i->opcode2 == 0xB8;
}

/* ============================================================================
 * Special Instruction Predicates
 * ============================================================================ */

static inline int x86_is_nop(const x86_insn_t *i)
{
    return i->opcode == 0x90;
}

static inline int x86_is_cpuid(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xA2;
}

static inline int x86_is_rdtsc(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0x31;
}

static inline int x86_is_cqo(const x86_insn_t *i)
{
    return i->opcode == 0x48 && i->opcode2 == 0x99;
}

static inline int x86_is_cli(const x86_insn_t *i)
{
    return i->opcode == 0xFA;
}

static inline int x86_is_sti(const x86_insn_t *i)
{
    return i->opcode == 0xFB;
}

static inline int x86_is_movs(const x86_insn_t *i)
{
    return i->opcode == 0xA4 || i->opcode == 0xA5;
}

static inline int x86_is_stos(const x86_insn_t *i)
{
    return i->opcode == 0xAA || i->opcode == 0xAB;
}

static inline int x86_is_lods(const x86_insn_t *i)
{
    return i->opcode == 0xAC || i->opcode == 0xAD;
}

static inline int x86_is_cmps(const x86_insn_t *i)
{
    return i->opcode == 0xA6 || i->opcode == 0xA7;
}

static inline int x86_is_scas(const x86_insn_t *i)
{
    return i->opcode == 0xAE || i->opcode == 0xAF;
}

static inline int x86_is_shld(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xA4;
}

static inline int x86_is_shrd(const x86_insn_t *i)
{
    return i->opcode == 0x0F && i->opcode2 == 0xAC;
}

#endif /* ROSETTA_X86_INSNS_H */
