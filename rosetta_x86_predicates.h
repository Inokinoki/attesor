/* ============================================================================
 * Rosetta Translator - x86_64 Instruction Predicates
 * ============================================================================
 *
 * This module provides predicate functions for identifying x86_64
 * instruction types based on decoded instruction information.
 * ============================================================================ */

#include <stdint.h>

/* x86 instruction info structure */
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
 * Basic Instruction Type Predicates
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

static inline int x86_is_mov(const x86_insn_t *i)
{
    return i->opcode == 0x8B || i->opcode == 0x89 ||
           i->opcode == 0xC7 || (i->opcode >= 0xB8 && i->opcode <= 0xBF);
}

static inline int x86_is_mov_imm64(const x86_insn_t *i)
{
    return (i->opcode >= 0xB8 && i->opcode <= 0xBF) && (i->rex & 0x08);
}

static inline int x86_is_lea(const x86_insn_t *i)
{
    return i->opcode == 0x8D;
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

static inline int x86_is_jcc(const x86_insn_t *i)
{
    return (i->opcode >= 0x70 && i->opcode <= 0x7F) ||
           (i->opcode == 0x0F && i->opcode2 >= 0x80 && i->opcode2 <= 0x8F);
}

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

static inline int x86_is_mul(const x86_insn_t *i)
{
    return i->opcode == 0xF7 && (i->reg == 4 || i->reg == 5);
}

static inline int x86_is_div(const x86_insn_t *i)
{
    return i->opcode == 0xF7 && (i->reg == 6 || i->reg == 7);
}

static inline uint8_t x86_get_jcc_cond(const x86_insn_t *i)
{
    if (i->opcode >= 0x70 && i->opcode <= 0x7F)
        return i->opcode & 0x0F;
    if (i->opcode2 >= 0x80 && i->opcode2 <= 0x8F)
        return i->opcode2 & 0x0F;
    return 0;
}

/* ============================================================================
 * P0 - Essential Instructions
 * ============================================================================ */

static inline int x86_is_movzx(const x86_insn_t *i)
{
    /* MOVZX r16/m8, r8 (0F B6) / MOVZX r32/m8, r8 (0F B6) / MOVZX r32/m16, r16 (0F B7) */
    return i->opcode == 0x0F && (i->opcode2 == 0xB6 || i->opcode2 == 0xB7);
}

static inline int x86_is_movsx(const x86_insn_t *i)
{
    /* MOVSX r16/m8, r8 (0F BE) / MOVSX r32/m8, r8 (0F BE) / MOVSX r32/m16, r16 (0F BF) */
    return i->opcode == 0x0F && (i->opcode2 == 0xBE || i->opcode2 == 0xBF);
}

static inline int x86_is_movsxd(const x86_insn_t *i)
{
    /* MOVSXD r64/m32, r32 (63 / x86-64) */
    return i->opcode == 0x63 && (i->rex & 0x08);  /* REX.W required */
}

static inline int x86_is_inc(const x86_insn_t *i)
{
    /* INC r/m8 (FE /0) | INC r/m64 (FF /0) | INC r64 (40+ for RAX-RCX, 48+ for R8-R15) */
    if (i->opcode == 0xFE && i->reg == 0) return 1;  /* r/m8 */
    if (i->opcode == 0xFF && i->reg == 0) return 1;  /* r/m64 */
    if ((i->opcode >= 0x40 && i->opcode <= 0x47) || (i->opcode >= 0x48 && i->opcode <= 0x4F))
        return 1;  /* register increment */
    return 0;
}

static inline int x86_is_dec(const x86_insn_t *i)
{
    /* DEC r/m8 (FE /1) | DEC r/m64 (FF /1) | DEC r64 (48+ for RAX-RCX, 50+ for R8-R15) */
    if (i->opcode == 0xFE && i->reg == 1) return 1;  /* r/m8 */
    if (i->opcode == 0xFF && i->reg == 1) return 1;  /* r/m64 */
    if ((i->opcode >= 0x48 && i->opcode <= 0x4F) || (i->opcode >= 0x50 && i->opcode <= 0x57))
        return 1;  /* register decrement */
    return 0;
}

static inline int x86_is_neg(const x86_insn_t *i)
{
    /* NEG r/m8 (F6 /3) | NEG r/m64 (F7 /3) */
    return (i->opcode == 0xF6 || i->opcode == 0xF7) && i->reg == 3;
}

static inline int x86_is_not(const x86_insn_t *i)
{
    /* NOT r/m8 (F6 /2) | NOT r/m64 (F7 /2) */
    return (i->opcode == 0xF6 || i->opcode == 0xF7) && i->reg == 2;
}

static inline int x86_is_push(const x86_insn_t *i)
{
    /* PUSH r/m64 (FF /6) | PUSH r64 (50-57) | PUSH imm8/32 (6A/68) */
    if (i->opcode >= 0x50 && i->opcode <= 0x57) return 1;  /* PUSH r64 */
    if (i->opcode == 0x6A || i->opcode == 0x68) return 1;  /* PUSH imm */
    if (i->opcode == 0xFF && i->reg == 6) return 1;  /* PUSH r/m64 */
    return 0;
}

static inline int x86_is_pop(const x86_insn_t *i)
{
    /* POP r64 (58-5F) | POP r/m64 (8F /0) */
    if (i->opcode >= 0x58 && i->opcode <= 0x5F) return 1;  /* POP r64 */
    if (i->opcode == 0x8F && i->reg == 0) return 1;  /* POP r/m64 */
    return 0;
}

static inline int x86_is_nop(const x86_insn_t *i)
{
    /* NOP (90) */
    return i->opcode == 0x90;
}

/* Shift/Rotate instructions */
static inline int x86_is_shl(const x86_insn_t *i)
{
    /* SHL r/m8,1 (D0 /4) | SHL r/m8,CL (D2 /4) | SHL r/m8,imm8 (C0 /4) */
    /* SHL r/m64,1 (D1 /4) | SHL r/m64,CL (D3 /4) | SHL r/m64,imm8 (C1 /4) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 4;
}

static inline int x86_is_shr(const x86_insn_t *i)
{
    /* SHR r/m8,1 (D0 /5) | SHR r/m8,CL (D2 /5) | SHR r/m8,imm8 (C0 /5) */
    /* SHR r/m64,1 (D1 /5) | SHR r/m64,CL (D3 /5) | SHR r/m64,imm8 (C1 /5) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 5;
}

static inline int x86_is_sar(const x86_insn_t *i)
{
    /* SAR r/m8,1 (D0 /7) | SAR r/m8,CL (D2 /7) | SAR r/m8,imm8 (C0 /7) */
    /* SAR r/m64,1 (D1 /7) | SAR r/m64,CL (D3 /7) | SAR r/m64,imm8 (C1 /7) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 7;
}

static inline int x86_is_rol(const x86_insn_t *i)
{
    /* ROL r/m8,1 (D0 /0) | ROL r/m8,CL (D2 /0) | ROL r/m8,imm8 (C0 /0) */
    /* ROL r/m64,1 (D1 /0) | ROL r/m64,CL (D3 /0) | ROL r/m64,imm8 (C1 /0) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 0;
}

static inline int x86_is_ror(const x86_insn_t *i)
{
    /* ROR r/m8,1 (D0 /1) | ROR r/m8,CL (D2 /1) | ROR r/m8,imm8 (C0 /1) */
    /* ROR r/m64,1 (D1 /1) | ROR r/m64,CL (D3 /1) | ROR r/m64,imm8 (C1 /1) */
    return (i->opcode == 0xD0 || i->opcode == 0xD1 || i->opcode == 0xD2 ||
            i->opcode == 0xD3 || i->opcode == 0xC0 || i->opcode == 0xC1) && i->reg == 1;
}

/* ============================================================================
 * P1 - Control Flow Instructions
 * ============================================================================ */

static inline int x86_is_cmov(const x86_insn_t *i)
{
    /* CMOVcc r64, r/m64 (0F 40-4F) */
    return i->opcode == 0x0F && i->opcode2 >= 0x40 && i->opcode2 <= 0x4F;
}

static inline uint8_t x86_get_cmov_cond(const x86_insn_t *i)
{
    if (x86_is_cmov(i))
        return i->opcode2 - 0x40;
    return 0;
}

static inline int x86_is_setcc(const x86_insn_t *i)
{
    /* SETcc r/m8 (0F 90-9F) */
    return i->opcode == 0x0F && i->opcode2 >= 0x90 && i->opcode2 <= 0x9F;
}

static inline uint8_t x86_get_setcc_cond(const x86_insn_t *i)
{
    if (x86_is_setcc(i))
        return i->opcode2 - 0x90;
    return 0;
}

static inline int x86_is_xchg(const x86_insn_t *i)
{
    /* XCHG r8,r8 (86-87) | XCHG r64,r64 (87) | XCHG r32,EAX (90-97) */
    if (i->opcode == 0x86 || i->opcode == 0x87) return 1;
    if (i->opcode >= 0x90 && i->opcode <= 0x97) return 1;  /* XCHG with EAX/RAX */
    return 0;
}

/* ============================================================================
 * P2 - Bit Manipulation Instructions
 * ============================================================================ */

static inline int x86_is_bsf(const x86_insn_t *i)
{
    /* BSF r64,r/m64 (0F BC) */
    return i->opcode == 0x0F && i->opcode2 == 0xBC;
}

static inline int x86_is_bsr(const x86_insn_t *i)
{
    /* BSR r64,r/m64 (0F BD) */
    return i->opcode == 0x0F && i->opcode2 == 0xBD;
}

static inline int x86_is_bt(const x86_insn_t *i)
{
    /* BT r/m64,r64 (0F A3) | BT r/m64,imm8 (0F BA /4) */
    if (i->opcode == 0x0F && i->opcode2 == 0xA3) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 4) return 1;
    return 0;
}

static inline int x86_is_bts(const x86_insn_t *i)
{
    /* BTS r/m64,r64 (0F AB) | BTS r/m64,imm8 (0F BA /5) */
    if (i->opcode == 0x0F && i->opcode2 == 0xAB) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 5) return 1;
    return 0;
}

static inline int x86_is_btr(const x86_insn_t *i)
{
    /* BTR r/m64,r64 (0F B3) | BTR r/m64,imm8 (0F BA /6) */
    if (i->opcode == 0x0F && i->opcode2 == 0xB3) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 6) return 1;
    return 0;
}

static inline int x86_is_btc(const x86_insn_t *i)
{
    /* BTC r/m64,r64 (0F BB) | BTC r/m64,imm8 (0F BA /7) */
    if (i->opcode == 0x0F && i->opcode2 == 0xBB) return 1;
    if (i->opcode == 0x0F && i->opcode2 == 0xBA && i->reg == 7) return 1;
    return 0;
}

static inline int x86_is_popcnt(const x86_insn_t *i)
{
    /* POPCNT r64,r/m64 (0F B8) */
    return i->opcode == 0x0F && i->opcode2 == 0xB8;
}

static inline int x86_is_tzcnt(const x86_insn_t *i)
{
    /* TZCNT r64,r/m64 (F3 0F BC) - same as BSF with F3 REP prefix */
    /* BSF is 0F BC, TZCNT adds F3 prefix */
    return i->opcode == 0xF3 && i->opcode2 == 0xBC;
}

static inline int x86_is_lzcnt(const x86_insn_t *i)
{
    /* LZCNT r64,r/m64 (F3 0F BD) - same as BSR with F3 REP prefix */
    /* BSR is 0F BD, LZCNT adds F3 prefix */
    return i->opcode == 0xF3 && i->opcode2 == 0xBD;
}

/* ============================================================================
 * P3 - String Operations
 * ============================================================================ */

static inline int x86_is_movs(const x86_insn_t *i)
{
    /* MOVS: A4 (byte), A5 (dword/qword) */
    return i->opcode == 0xA4 || i->opcode == 0xA5;
}

static inline int x86_is_stos(const x86_insn_t *i)
{
    /* STOS: AA (byte), AB (dword/qword) */
    return i->opcode == 0xAA || i->opcode == 0xAB;
}

static inline int x86_is_lods(const x86_insn_t *i)
{
    /* LODS: AC (byte), AD (dword/qword) */
    return i->opcode == 0xAC || i->opcode == 0xAD;
}

static inline int x86_is_cmps(const x86_insn_t *i)
{
    /* CMPS: A6 (byte), A7 (dword/qword) */
    return i->opcode == 0xA6 || i->opcode == 0xA7;
}

static inline int x86_is_scas(const x86_insn_t *i)
{
    /* SCAS: AE (byte), AF (dword/qword) */
    return i->opcode == 0xAE || i->opcode == 0xAF;
}

static inline int x86_has_rep_prefix(const x86_insn_t *i)
{
    /* REP/REPE/REPNE prefixes: F3 (REP/REPE), F2 (REPNE) */
    /* REP prefix flag is stored in rex field bit 6 (0x40) */
    return i->rex & 0x40;
}

/* ============================================================================
 * P4 - Special Instructions
 * ============================================================================ */

static inline int x86_is_cpuid(const x86_insn_t *i)
{
    /* CPUID: 0F A2 */
    return i->opcode == 0x0F && i->opcode2 == 0xA2;
}

static inline int x86_is_rdtsc(const x86_insn_t *i)
{
    /* RDTSC: 0F 31 */
    return i->opcode == 0x0F && i->opcode2 == 0x31;
}

static inline int x86_is_rdtscp(const x86_insn_t *i)
{
    /* RDTSCP: 0F 33 */
    return i->opcode == 0x0F && i->opcode2 == 0x33;
}

static inline int x86_is_shld(const x86_insn_t *i)
{
    /* SHLD: 0F A4 (imm8), 0F A5 (CL) */
    return i->opcode == 0x0F && (i->opcode2 == 0xA4 || i->opcode2 == 0xA5);
}

static inline int x86_is_shrd(const x86_insn_t *i)
{
    /* SHRD: 0F AC (imm8), 0F AD (CL) */
    return i->opcode == 0x0F && (i->opcode2 == 0xAC || i->opcode2 == 0xAD);
}

static inline int x86_is_cdq(const x86_insn_t *i)
{
    /* CDQ/CQO: 99 (CBW), 98 (CWD), 9A (CWDE), 48 99 (CQO) */
    return i->opcode == 0x99 || i->opcode == 0x98;
}

static inline int x86_is_cwd(const x86_insn_t *i)
{
    /* CWD: 99 */
    return i->opcode == 0x99;
}

static inline int x86_is_cqo(const x86_insn_t *i)
{
    /* CQO: 48 99 */
    return i->opcode == 0x99 && (i->rex & 0x08);
}

static inline int x86_is_cli(const x86_insn_t *i)
{
    /* CLI: FA */
    return i->opcode == 0xFA;
}

static inline int x86_is_sti(const x86_insn_t *i)
{
    /* STI: FB */
    return i->opcode == 0xFB;
}

static inline int x86_is_cli_sti(const x86_insn_t *i)
{
    return x86_is_cli(i) || x86_is_sti(i);
}
