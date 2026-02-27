/* ============================================================================
 * Rosetta Translator - Branch Translation Implementation
 * ============================================================================ */

#include "rosetta_codegen.h"
#include "rosetta_translate_branch_impl.h"
#include <stdint.h>

typedef struct {
    uint16_t opcode;
    uint8_t modrm;
    uint8_t reg;
    uint8_t rm;
    uint8_t length;
    int32_t imm32;
    int64_t imm64;
    int32_t disp;
} x86_insn_t;

static inline int x86_is_jcc(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_jcc(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x70 && insn->opcode <= 0x7F) ||
           (insn->opcode >= 0x0F80 && insn->opcode <= 0x0F8F);
}

static inline int x86_is_jmp(const x86_insn_t *insn)
{
    return insn->opcode == 0xE9 || insn->opcode == 0xEB;
}

static inline int x86_is_call(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_call(const x86_insn_t *insn)
{
    return insn->opcode == 0xE8;
}

static inline int x86_is_ret(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_ret(const x86_insn_t *insn)
{
    return insn->opcode == 0xC3 || insn->opcode == 0xC2;
}

static inline int x86_is_cmov(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_cmov(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x0F40 && insn->opcode <= 0x0F4F);
}

static inline int x86_is_setcc(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_setcc(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x0F90 && insn->opcode <= 0x0F9F);
}

static inline int x86_is_xchg(const x86_insn_t *insn) __attribute__((unused));
static inline int x86_is_xchg(const x86_insn_t *insn)
{
    return (insn->opcode >= 0x90 && insn->opcode <= 0x97);
}

int translate_branch_jcc(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)block_pc;

    if (!x86_is_jcc(insn)) {
        return 0;
    }

    /* Map x86 condition codes to ARM64 condition codes
     * x86: 0=O, 1=NO, 2=B/C, 3=AE/NC, 4=Z, 5=NZ, 6=BE/NA, 7=A/NBE,
     *      8=S, 9=NS, 10=P/PE, 11=NP/PO, 12=L/NGE, 13=GE/L, 14=LE/NG, 15=G/NLE
     * ARM64: 0=EQ, 1=NE, 2=CS/HS, 3=CC/LO, 4=MI, 5=PL, 6=VS, 7=VC,
     *        8=HI, 9=LS, 10=GE, 11=LT, 12=GT, 13=LE, 14=AL, 15=NV
     */
    uint8_t x86_cond = insn->opcode & 0x0F;
    uint8_t arm_cond;

    /* Handle two-byte opcodes (0x0F8x) */
    if (insn->opcode >= 0x0F80 && insn->opcode <= 0x0F8F) {
        x86_cond = insn->opcode & 0x0F;
    } else if (insn->opcode >= 0x70 && insn->opcode <= 0x7F) {
        x86_cond = insn->opcode & 0x0F;
    }

    /* Map x86 condition to ARM64 condition */
    switch (x86_cond) {
        case 0x0: arm_cond = 7; break;  /* JO -> VS (overflow set) */
        case 0x1: arm_cond = 6; break;  /* JNO -> VC (overflow clear) */
        case 0x2: arm_cond = 3; break;  /* JB/JC -> CC/LO (carry clear) */
        case 0x3: arm_cond = 2; break;  /* JAE/JNC -> CS/HS (carry set) */
        case 0x4: arm_cond = 0; break;  /* JZ/JE -> EQ (zero) */
        case 0x5: arm_cond = 1; break;  /* JNZ/JNE -> NE (not zero) */
        case 0x6: arm_cond = 9; break;  /* JBE/JNA -> LS (lower or same) */
        case 0x7: arm_cond = 8; break;  /* JA/JNBE -> HI (higher) */
        case 0x8: arm_cond = 4; break;  /* JS -> MI (minus/negative) */
        case 0x9: arm_cond = 5; break;  /* JNS -> PL (plus/non-negative) */
        case 0xA: arm_cond = 10; break; /* JP/JPE -> GE (used for parity) */
        case 0xB: arm_cond = 11; break; /* JNP/JPO -> LT (used for no parity) */
        case 0xC: arm_cond = 11; break; /* JL/NGE -> LT (less than) */
        case 0xD: arm_cond = 10; break; /* JGE/NL -> GE (greater or equal) */
        case 0xE: arm_cond = 13; break; /* JLE/NG -> LE (less or equal) */
        case 0xF: arm_cond = 12; break; /* JG/NLE -> GT (greater than) */
        default: arm_cond = 14; break;  /* Default to AL (always) */
    }

    /* Emit conditional branch: B.cond with offset */
    /* Calculate offset relative to current PC */
    int32_t offset = insn->imm32;
    (void)offset; /* Will be calculated properly at runtime */

    /* B.cond label (offset) - encoding: 0x54000000 | (cond << 12) | (offset >> 2) */
    uint32_t encoded_offset = (uint32_t)(offset >> 2) & 0x7FFFF;
    uint32_t insn_arm = 0x54000000 | ((uint32_t)arm_cond << 12) | encoded_offset;
    emit_arm64_insn(code_buf, insn_arm);

    return 1;
}

int translate_branch_jmp(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (x86_is_jmp(insn)) {
        emit_b(code_buf, insn->imm32);
        return 1;
    }
    return 0;
}

int translate_branch_call(code_buffer_t *code_buf, const void *insn_ptr, uint64_t block_pc)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)block_pc;

    if (!x86_is_call(insn)) {
        return 0;
    }

    /* CALL instruction:
     * 1. Push return address to stack (decrement SP, store LR)
     * 2. Branch to target address
     */

    /* BL - Branch with Link (relative offset) */
    /* BL offset - encoding: 0x94000000 | (offset >> 2) */
    int32_t offset = insn->imm32;
    uint32_t encoded_offset = (uint32_t)(offset >> 2) & 0x3FFFFFF;
    uint32_t insn_arm = 0x94000000 | encoded_offset;
    emit_arm64_insn(code_buf, insn_arm);

    return 1;
}

int translate_branch_ret(code_buffer_t *code_buf)
{
    emit_ret(code_buf);
    return 1;
}

void translate_branch_cmov(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_cmov(insn)) {
        return;
    }

    /* CMOV - Conditional Move
     * Map x86 condition codes to ARM64 condition codes
     * CMOVcc rd, rm -> CSEL rd, rm, rd, !cc (inverted condition)
     */
    uint8_t x86_cond = (insn->opcode - 0x0F40) & 0x0F;
    uint8_t arm_cond;

    /* Map x86 condition to ARM64 condition (same mapping as JCC) */
    switch (x86_cond) {
        case 0x0: arm_cond = 7; break;  /* CMOVO -> VS */
        case 0x1: arm_cond = 6; break;  /* CMOVNO -> VC */
        case 0x2: arm_cond = 3; break;  /* CMOVB/C -> CC/LO */
        case 0x3: arm_cond = 2; break;  /* CMOVAE/NC -> CS/HS */
        case 0x4: arm_cond = 0; break;  /* CMOVZ/E -> EQ */
        case 0x5: arm_cond = 1; break;  /* CMOVNZ/NE -> NE */
        case 0x6: arm_cond = 9; break;  /* CMOVBE/NA -> LS */
        case 0x7: arm_cond = 8; break;  /* CMOVA/NBE -> HI */
        case 0x8: arm_cond = 4; break;  /* CMOVS -> MI */
        case 0x9: arm_cond = 5; break;  /* CMOVNS -> PL */
        case 0xA: arm_cond = 10; break; /* CMOVP/PE -> GE */
        case 0xB: arm_cond = 11; break; /* CMOVNP/PO -> LT */
        case 0xC: arm_cond = 11; break; /* CMOVL/NGE -> LT */
        case 0xD: arm_cond = 10; break; /* CMOVGE/NL -> GE */
        case 0xE: arm_cond = 13; break; /* CMOVLE/NG -> LE */
        case 0xF: arm_cond = 12; break; /* CMOVG/NLE -> GT */
        default: arm_cond = 14; break;
    }

    /* CSEL Rd, Rn, Rm, cond - conditional select
     * CSEL rd, rm, rd, cond -> if cond { rd = rm; } else { rd = rd; }
     * Encoding: 0x1A800000 | (Rd << 0) | (Rm << 16) | (cond << 12) | (Rn << 5)
     */
    uint32_t insn_arm = 0x1A800000 | ((arm_rd & 31) << 0) | ((arm_rm & 31) << 16) |
                        ((uint32_t)arm_cond << 12) | ((arm_rd & 31) << 5);
    emit_arm64_insn(code_buf, insn_arm);
}

void translate_branch_setcc(code_buffer_t *code_buf, const void *insn_ptr, uint8_t arm_rd)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;

    if (!x86_is_setcc(insn)) {
        return;
    }

    /* SETcc - Set byte if condition is true
     * Sets destination byte to 1 if condition is true, 0 otherwise
     * SETcc r/m8 -> CSET Wd, cond (in ARM64)
     */
    uint8_t x86_cond = (insn->opcode - 0x0F90) & 0x0F;
    uint8_t arm_cond;

    /* Map x86 condition to ARM64 condition (same mapping as JCC/CMOV) */
    switch (x86_cond) {
        case 0x0: arm_cond = 7; break;  /* SETO -> VS */
        case 0x1: arm_cond = 6; break;  /* SETNO -> VC */
        case 0x2: arm_cond = 3; break;  /* SETB/C -> CC/LO */
        case 0x3: arm_cond = 2; break;  /* SETAE/NC -> CS/HS */
        case 0x4: arm_cond = 0; break;  /* SETZ/E -> EQ */
        case 0x5: arm_cond = 1; break;  /* SETNZ/NE -> NE */
        case 0x6: arm_cond = 9; break;  /* SETBE/NA -> LS */
        case 0x7: arm_cond = 8; break;  /* SETA/NBE -> HI */
        case 0x8: arm_cond = 4; break;  /* SETS -> MI */
        case 0x9: arm_cond = 5; break;  /* SETNS -> PL */
        case 0xA: arm_cond = 10; break; /* SETP/PE -> GE */
        case 0xB: arm_cond = 11; break; /* SETNP/PO -> LT */
        case 0xC: arm_cond = 11; break; /* SETL/NGE -> LT */
        case 0xD: arm_cond = 10; break; /* SETGE/NL -> GE */
        case 0xE: arm_cond = 13; break; /* SETLE/NG -> LE */
        case 0xF: arm_cond = 12; break; /* SETG/NLE -> GT */
        default: arm_cond = 14; break;
    }

    /* CSET Wd, cond - conditional set
     * CSET sets destination to 1 if condition true, 0 otherwise
     * Encoding: 0x1A9F07E0 | (cond << 12) | (Rd << 0)
     */
    uint32_t insn_arm = 0x1A9F07E0 | ((uint32_t)arm_cond << 12) | ((arm_rd & 31) << 0);
    emit_arm64_insn(code_buf, insn_arm);
}

void translate_branch_xchg(code_buffer_t *code_buf, const void *insn_ptr,
                           uint8_t arm_rd, uint8_t arm_rm)
{
    const x86_insn_t *insn = (const x86_insn_t *)insn_ptr;
    (void)insn;

    /* XCHG - Exchange registers
     * XCHG rd, rm exchanges the values of two registers
     * For atomic XCHG with memory, use LDAXP/STLXP pair
     * For register-register XCHG, use a temporary register
     */

    /* XCHG using EOR swap (no temp register needed):
     * EOR rd, rd, rm
     * EOR rm, rd, rm
     * EOR rd, rd, rm
     */
    if (arm_rd != arm_rm) {
        /* EOR Rd, Rd, Rm */
        uint32_t insn_arm = 0xCA000000 | ((arm_rd & 31) << 0) | ((arm_rd & 31) << 5) | ((arm_rm & 31) << 16);
        emit_arm64_insn(code_buf, insn_arm);
        /* EOR Rm, Rd, Rm */
        insn_arm = 0xCA000000 | ((arm_rm & 31) << 0) | ((arm_rd & 31) << 5) | ((arm_rm & 31) << 16);
        emit_arm64_insn(code_buf, insn_arm);
        /* EOR Rd, Rd, Rm */
        insn_arm = 0xCA000000 | ((arm_rd & 31) << 0) | ((arm_rd & 31) << 5) | ((arm_rm & 31) << 16);
        emit_arm64_insn(code_buf, insn_arm);
    }
}
