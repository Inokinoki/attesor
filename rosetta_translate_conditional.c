/* ============================================================================
 * Rosetta Translator - Conditional Instruction Translation Implementation
 * ============================================================================
 *
 * This module implements translation of ARM64 conditional instructions
 * to x86_64 machine code.
 * ============================================================================ */

#include "rosetta_translate_conditional.h"
#include "rosetta_emit_x86.h"
#include <stdbool.h>

/* External register mapping function from ALU module */
extern uint8_t translate_get_x86_reg(uint8_t arm_reg);

/* ============================================================================
 * Condition Code to x86 Jump Opcode Mapping
 * ============================================================================ */

static const uint8_t g_cond_to_jmp[16] = {
    0x84,  /* EQ (Z=1) -> JE/JZ */
    0x85,  /* NE (Z=0) -> JNE/JNZ */
    0x83,  /* CS (C=1) -> JAE/JNB */
    0x82,  /* CC (C=0) -> JB/JNAE */
    0x88,  /* MI (N=1) -> JS */
    0x89,  /* PL (N=0) -> JNS */
    0x80,  /* VS (V=1) -> JO */
    0x81,  /* VC (V=0) -> JNO */
    0x87,  /* HI (C=1 && Z=0) -> JA/JNBE */
    0x86,  /* LS (C=0 || Z=1) -> JBE/JNA */
    0x8D,  /* GE (N=V) -> JGE/JNL */
    0x8C,  /* LT (N!=V) -> JL/JNGE */
    0x8F,  /* GT (Z=0 && N=V) -> JG/JNLE */
    0x8E,  /* LE (Z=1 || N!=V) -> JLE/JNG */
    0xEB,  /* AL (Always) -> JMP */
    0xEB   /* NV (Never) -> JMP (unused) */
};

/* ============================================================================
 * Flag Conversion Helper
 * ============================================================================ */

/**
 * convert_arm64_flags_to_x86 - Convert ARM64 NZCV to x86 EFLAGS
 * @pstate: ARM64 PSTATE (NZCV flags)
 *
 * Sets up x86 flags based on ARM64 NZCV:
 * - ARM N (bit 31) -> x86 SF
 * - ARM Z (bit 30) -> x86 ZF
 * - ARM C (bit 29) -> x86 CF
 * - ARM V (bit 28) -> x86 OF
 */
static void convert_arm64_flags_to_x86(code_buf_t *code_buf, uint32_t pstate)
{
    uint8_t N = (pstate >> 31) & 1;
    uint8_t Z = (pstate >> 30) & 1;
    uint8_t C = (pstate >> 29) & 1;
    uint8_t V = (pstate >> 28) & 1;

    /* For proper conditional execution, we need to set x86 flags
     * to match what ARM64 flags would produce.
     *
     * We'll use a compare against zero technique:
     * - Load a temporary with a value that produces the right flags
     */

    /* Use RAX as temporary */
    /* XOR EAX, EAX - clear EAX and flags */
    emit_x86_xor_reg_reg(code_buf, EMIT_RAX, EMIT_RAX);

    /* If Z flag is set, we want ZF=1, so compare 0,0 */
    /* If Z flag is clear, we want ZF=0, so compare 1,0 */
    if (Z) {
        emit_x86_cmp_reg_imm32(code_buf, EMIT_RAX, 0);
    } else {
        emit_x86_mov_reg_imm32(code_buf, EMIT_RAX, 1);
        emit_x86_cmp_reg_imm32(code_buf, EMIT_RAX, 0);
    }

    (void)N; (void)C; (void)V;  /* Simplified - full implementation would handle all flags */
}

/* ============================================================================
 * Conditional Select (CSEL) and Variants
 * ============================================================================ */

/**
 * translate_cond_csel - Translate CSEL instruction
 * CSEL Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm;
 */
int translate_cond_csel(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Update guest state based on condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;                    /* EQ */
        case 0x1: cond_true = (Z == 0); break;                    /* NE */
        case 0x2: cond_true = (C == 1); break;                    /* CS */
        case 0x3: cond_true = (C == 0); break;                    /* CC */
        case 0x4: cond_true = (N == 1); break;                    /* MI */
        case 0x5: cond_true = (N == 0); break;                    /* PL */
        case 0x6: cond_true = (V == 1); break;                    /* VS */
        case 0x7: cond_true = (V == 0); break;                    /* VC */
        case 0x8: cond_true = (C == 1 && Z == 0); break;          /* HI */
        case 0x9: cond_true = (C == 0 || Z == 1); break;          /* LS */
        case 0xA: cond_true = (N == V); break;                    /* GE */
        case 0xB: cond_true = (N != V); break;                    /* LT */
        case 0xC: cond_true = (Z == 0 && N == V); break;          /* GT */
        case 0xD: cond_true = (Z == 1 || N != V); break;          /* LE */
        case 0xE: cond_true = true; break;                        /* AL */
        case 0xF: cond_true = false; break;                       /* NV */
    }

    if (cond_true) {
        guest_state[rd] = guest_state[rn];
    } else {
        guest_state[rd] = guest_state[rm];
    }

    /* Emit x86_64 code using CMOV (conditional move) */
    /* MOV rd, rm (default value) */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);

    /* CMOV<cond> rd, rn (conditional move if condition met) */
    /* Map ARM64 condition to x86 CMOV opcode */
    uint8_t jmp_opcode = g_cond_to_jmp[cond];

    /* CMOV uses different encoding than JMP - extract low nibble */
    code_buf_emit_byte(code_buf, 0x48);  /* REX.W */

    /* CMOV opcode: 0F 40+<low_bit> */
    code_buf_emit_byte(code_buf, 0x0F);

    /* CMOV condition encoding differs from JMP - map appropriately */
    uint8_t cmov_cond;
    switch (cond) {
        case 0x0: cmov_cond = 0x44; break;  /* EQ -> CMOVE */
        case 0x1: cmov_cond = 0x45; break;  /* NE -> CMOVNE */
        case 0x2: cmov_cond = 0x43; break;  /* CS -> CMOVAE */
        case 0x3: cmov_cond = 0x42; break;  /* CC -> CMOVB */
        case 0x4: cmov_cond = 0x48; break;  /* MI -> CMOVS */
        case 0x5: cmov_cond = 0x49; break;  /* PL -> CMOVNS */
        case 0x6: cmov_cond = 0x40; break;  /* VS -> CMOVO */
        case 0x7: cmov_cond = 0x41; break;  /* VC -> CMOVNO */
        case 0x8: cmov_cond = 0x47; break;  /* HI -> CMOVA */
        case 0x9: cmov_cond = 0x46; break;  /* LS -> CMOVBE */
        case 0xA: cmov_cond = 0x4D; break;  /* GE -> CMOVGE */
        case 0xB: cmov_cond = 0x4C; break;  /* LT -> CMOVL */
        case 0xC: cmov_cond = 0x4F; break;  /* GT -> CMOVG */
        case 0xD: cmov_cond = 0x4E; break;  /* LE -> CMOVLE */
        case 0xE:                            /* AL -> MOV (already done) */
        case 0xF:                            /* NV -> no move */
        default:
            return 0;  /* Already handled by MOV above */
    }

    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/**
 * translate_cond_csinc - Translate CSINC instruction
 * CSINC Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = Rm + 1;
 */
int translate_cond_csinc(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = guest_state[rn];
    } else {
        guest_state[rd] = guest_state[rm] + 1;
    }

    /* Emit x86_64 code:
     * MOV rd, rm
     * If condition false, ADD rd, 1
     * If condition true, MOV rd, rn
     */

    /* First, set up flags for conditional execution */
    convert_arm64_flags_to_x86(code_buf, *pstate);

    /* MOV rd, rm (start with Rm value) */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);

    /* We need to conditionally add 1 or move Rn
     * Using CMOV approach:
     * ADD rd, 1 (always)
     * CMOV<cond> rd, rn (if condition true, use Rn instead)
     */

    /* ADD rd, 1 */
    emit_x86_add_reg_imm32(code_buf, x86_rd, 1);

    /* CMOV<cond> rd, rn */
    uint8_t cmov_cond;
    switch (cond) {
        case 0x0: cmov_cond = 0x44; break;  /* EQ -> CMOVE */
        case 0x1: cmov_cond = 0x45; break;  /* NE -> CMOVNE */
        case 0x2: cmov_cond = 0x43; break;  /* CS -> CMOVAE */
        case 0x3: cmov_cond = 0x42; break;  /* CC -> CMOVB */
        case 0x4: cmov_cond = 0x48; break;  /* MI -> CMOVS */
        case 0x5: cmov_cond = 0x49; break;  /* PL -> CMOVNS */
        case 0x6: cmov_cond = 0x40; break;  /* VS -> CMOVO */
        case 0x7: cmov_cond = 0x41; break;  /* VC -> CMOVNO */
        case 0x8: cmov_cond = 0x47; break;  /* HI -> CMOVA */
        case 0x9: cmov_cond = 0x46; break;  /* LS -> CMOVBE */
        case 0xA: cmov_cond = 0x4D; break;  /* GE -> CMOVGE */
        case 0xB: cmov_cond = 0x4C; break;  /* LT -> CMOVL */
        case 0xC: cmov_cond = 0x4F; break;  /* GT -> CMOVG */
        case 0xD: cmov_cond = 0x4E; break;  /* LE -> CMOVLE */
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/**
 * translate_cond_csinv - Translate CSINV instruction
 * CSINV Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = ~Rm;
 */
int translate_cond_csinv(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Check condition and update guest state */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = guest_state[rn];
    } else {
        guest_state[rd] = ~guest_state[rm];
    }

    /* Emit x86_64 code */
    /* MOV rd, rm */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);

    /* NOT rd */
    emit_x86_not_reg(code_buf, x86_rd);

    /* CMOV<cond> rd, rn */
    uint8_t cmov_cond;
    switch (cond) {
        case 0x0: cmov_cond = 0x44; break;
        case 0x1: cmov_cond = 0x45; break;
        case 0x2: cmov_cond = 0x43; break;
        case 0x3: cmov_cond = 0x42; break;
        case 0x4: cmov_cond = 0x48; break;
        case 0x5: cmov_cond = 0x49; break;
        case 0x6: cmov_cond = 0x40; break;
        case 0x7: cmov_cond = 0x41; break;
        case 0x8: cmov_cond = 0x47; break;
        case 0x9: cmov_cond = 0x46; break;
        case 0xA: cmov_cond = 0x4D; break;
        case 0xB: cmov_cond = 0x4C; break;
        case 0xC: cmov_cond = 0x4F; break;
        case 0xD: cmov_cond = 0x4E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/**
 * translate_cond_csneg - Translate CSNEG instruction
 * CSNEG Rd, Rn, Rm, cond  ->  if (cond) Rd = Rn; else Rd = -Rm;
 */
int translate_cond_csneg(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t rm = (encoding >> 16) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);
    uint8_t x86_rm = translate_get_x86_reg(rm);

    /* Check condition and update guest state */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = guest_state[rn];
    } else {
        guest_state[rd] = -guest_state[rm];
    }

    /* Emit x86_64 code */
    /* MOV rd, rm */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rm);

    /* NEG rd */
    emit_x86_neg_reg(code_buf, x86_rd);

    /* CMOV<cond> rd, rn */
    uint8_t cmov_cond;
    switch (cond) {
        case 0x0: cmov_cond = 0x44; break;
        case 0x1: cmov_cond = 0x45; break;
        case 0x2: cmov_cond = 0x43; break;
        case 0x3: cmov_cond = 0x42; break;
        case 0x4: cmov_cond = 0x48; break;
        case 0x5: cmov_cond = 0x49; break;
        case 0x6: cmov_cond = 0x40; break;
        case 0x7: cmov_cond = 0x41; break;
        case 0x8: cmov_cond = 0x47; break;
        case 0x9: cmov_cond = 0x46; break;
        case 0xA: cmov_cond = 0x4D; break;
        case 0xB: cmov_cond = 0x4C; break;
        case 0xC: cmov_cond = 0x4F; break;
        case 0xD: cmov_cond = 0x4E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/* ============================================================================
 * Condition Set Instructions
 * ============================================================================ */

/**
 * translate_cond_cset - Translate CSET instruction
 * CSET Rd, cond  ->  Rd = (cond) ? 1 : 0;
 */
int translate_cond_cset(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    guest_state[rd] = cond_true ? 1 : 0;

    /* Emit x86_64 code */
    /* XOR rd, rd (set to 0) */
    emit_x86_xor_reg_reg(code_buf, x86_rd, x86_rd);

    /* If condition is true, add 1 using ADC with self */
    /* SET<cond> al - sets AL to 1 if condition true, 0 otherwise */
    uint8_t set_cond;
    switch (cond) {
        case 0x0: set_cond = 0x94; break;  /* EQ -> SETE */
        case 0x1: set_cond = 0x95; break;  /* NE -> SETNE */
        case 0x2: set_cond = 0x93; break;  /* CS -> SETAE */
        case 0x3: set_cond = 0x92; break;  /* CC -> SETB */
        case 0x4: set_cond = 0x98; break;  /* MI -> SETS */
        case 0x5: set_cond = 0x99; break;  /* PL -> SETNS */
        case 0x6: set_cond = 0x90; break;  /* VS -> SETO */
        case 0x7: set_cond = 0x91; break;  /* VC -> SETNO */
        case 0x8: set_cond = 0x97; break;  /* HI -> SETA */
        case 0x9: set_cond = 0x96; break;  /* LS -> SETBE */
        case 0xA: set_cond = 0x9D; break;  /* GE -> SETGE */
        case 0xB: set_cond = 0x9C; break;  /* LT -> SETL */
        case 0xC: set_cond = 0x9F; break;  /* GT -> SETG */
        case 0xD: set_cond = 0x9E; break;  /* LE -> SETLE */
        default:
            return 0;
    }

    /* SET<cond> al */
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, set_cond);
    code_buf_emit_byte(code_buf, 0xC0);  /* AL */

    /* MOVZX rd, al - zero extend AL to full register */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xB6);
    code_buf_emit_byte(code_buf, 0xC0);

    return 0;
}

/**
 * translate_cond_csetm - Translate CSETM instruction
 * CSETM Rd, cond  ->  Rd = (cond) ? 0xFFFFFFFFFFFFFFFF : 0;
 */
int translate_cond_csetm(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    guest_state[rd] = cond_true ? ~0ULL : 0;

    /* Emit x86_64 code */
    /* XOR rd, rd (set to 0) */
    emit_x86_xor_reg_reg(code_buf, x86_rd, x86_rd);

    /* SET<cond> al */
    uint8_t set_cond;
    switch (cond) {
        case 0x0: set_cond = 0x94; break;
        case 0x1: set_cond = 0x95; break;
        case 0x2: set_cond = 0x93; break;
        case 0x3: set_cond = 0x92; break;
        case 0x4: set_cond = 0x98; break;
        case 0x5: set_cond = 0x99; break;
        case 0x6: set_cond = 0x90; break;
        case 0x7: set_cond = 0x91; break;
        case 0x8: set_cond = 0x97; break;
        case 0x9: set_cond = 0x96; break;
        case 0xA: set_cond = 0x9D; break;
        case 0xB: set_cond = 0x9C; break;
        case 0xC: set_cond = 0x9F; break;
        case 0xD: set_cond = 0x9E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, set_cond);
    code_buf_emit_byte(code_buf, 0xC0);  /* AL */

    /* NEG al - if AL=1, becomes 0xFF; if AL=0, stays 0 */
    code_buf_emit_byte(code_buf, 0xF6);
    code_buf_emit_byte(code_buf, 0xD8);  /* NEG AL */

    /* MOVSXD rd, eax - sign extend to 64 bits */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x63);
    code_buf_emit_byte(code_buf, 0xC0 + (x86_rd & 7));

    return 0;
}

/* ============================================================================
 * Conditional Operation Instructions
 * ============================================================================ */

/**
 * translate_cond_cinc - Translate CINC instruction
 * CINC Rd, Rn, cond  ->  if (cond) Rd = Rn + 1; else Rd = Rn;
 */
int translate_cond_cinc(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = guest_state[rn] + 1;
    } else {
        guest_state[rd] = guest_state[rn];
    }

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* CMOV<cond> is not suitable here since we want to ADD when condition is true */
    /* Instead: use conditional jump or SET + ADD approach */

    /* SET<cond> al - AL = 1 if condition true */
    uint8_t set_cond;
    switch (cond) {
        case 0x0: set_cond = 0x94; break;
        case 0x1: set_cond = 0x95; break;
        case 0x2: set_cond = 0x93; break;
        case 0x3: set_cond = 0x92; break;
        case 0x4: set_cond = 0x98; break;
        case 0x5: set_cond = 0x99; break;
        case 0x6: set_cond = 0x90; break;
        case 0x7: set_cond = 0x91; break;
        case 0x8: set_cond = 0x97; break;
        case 0x9: set_cond = 0x96; break;
        case 0xA: set_cond = 0x9D; break;
        case 0xB: set_cond = 0x9C; break;
        case 0xC: set_cond = 0x9F; break;
        case 0xD: set_cond = 0x9E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, set_cond);
    code_buf_emit_byte(code_buf, 0xC0);  /* AL */

    /* MOVZX rcx, al - extend to 64 bits */
    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xB6);
    code_buf_emit_byte(code_buf, 0xC8);  /* RCX, AL */

    /* ADD rd, rcx */
    emit_x86_add_reg_reg(code_buf, x86_rd, EMIT_RCX);

    return 0;
}

/**
 * translate_cond_cinvert - Translate CINVERT instruction
 * CINVERT Rd, Rn, cond  ->  if (cond) Rd = ~Rn; else Rd = Rn;
 */
int translate_cond_cinvert(uint32_t encoding, code_buf_t *code_buf,
                           uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = ~guest_state[rn];
    } else {
        guest_state[rd] = guest_state[rn];
    }

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* CMOV<inverted_cond> rd, rd_copy - but simpler to use NOT then CMOV */
    /* NOT rd */
    emit_x86_not_reg(code_buf, x86_rd);

    /* Get inverted condition */
    uint8_t inv_cond = cond ^ 1;

    /* CMOV<inv_cond> rd, rn */
    uint8_t cmov_cond;
    switch (inv_cond) {
        case 0x0: cmov_cond = 0x44; break;
        case 0x1: cmov_cond = 0x45; break;
        case 0x2: cmov_cond = 0x43; break;
        case 0x3: cmov_cond = 0x42; break;
        case 0x4: cmov_cond = 0x48; break;
        case 0x5: cmov_cond = 0x49; break;
        case 0x6: cmov_cond = 0x40; break;
        case 0x7: cmov_cond = 0x41; break;
        case 0x8: cmov_cond = 0x47; break;
        case 0x9: cmov_cond = 0x46; break;
        case 0xA: cmov_cond = 0x4D; break;
        case 0xB: cmov_cond = 0x4C; break;
        case 0xC: cmov_cond = 0x4F; break;
        case 0xD: cmov_cond = 0x4E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/**
 * translate_cond_cneg - Translate CNEG instruction
 * CNEG Rd, Rn, cond  ->  if (cond) Rd = -Rn; else Rd = Rn;
 */
int translate_cond_cneg(uint32_t encoding, code_buf_t *code_buf,
                        uint64_t *guest_state, uint32_t *pstate)
{
    uint8_t rd = encoding & 0x1F;
    uint8_t rn = (encoding >> 5) & 0x1F;
    uint8_t cond = (encoding >> 12) & 0x0F;

    uint8_t x86_rd = translate_get_x86_reg(rd);
    uint8_t x86_rn = translate_get_x86_reg(rn);

    /* Check condition */
    uint8_t N = (*pstate >> 31) & 1;
    uint8_t Z = (*pstate >> 30) & 1;
    uint8_t C = (*pstate >> 29) & 1;
    uint8_t V = (*pstate >> 28) & 1;

    bool cond_true = false;
    switch (cond) {
        case 0x0: cond_true = (Z == 1); break;
        case 0x1: cond_true = (Z == 0); break;
        case 0x2: cond_true = (C == 1); break;
        case 0x3: cond_true = (C == 0); break;
        case 0x4: cond_true = (N == 1); break;
        case 0x5: cond_true = (N == 0); break;
        case 0x6: cond_true = (V == 1); break;
        case 0x7: cond_true = (V == 0); break;
        case 0x8: cond_true = (C == 1 && Z == 0); break;
        case 0x9: cond_true = (C == 0 || Z == 1); break;
        case 0xA: cond_true = (N == V); break;
        case 0xB: cond_true = (N != V); break;
        case 0xC: cond_true = (Z == 0 && N == V); break;
        case 0xD: cond_true = (Z == 1 || N != V); break;
        case 0xE: cond_true = true; break;
        case 0xF: cond_true = false; break;
        default: break;
    }

    if (cond_true) {
        guest_state[rd] = -guest_state[rn];
    } else {
        guest_state[rd] = guest_state[rn];
    }

    /* Emit x86_64 code */
    /* MOV rd, rn */
    emit_x86_mov_reg_reg(code_buf, x86_rd, x86_rn);

    /* NEG rd */
    emit_x86_neg_reg(code_buf, x86_rd);

    /* Get inverted condition */
    uint8_t inv_cond = cond ^ 1;

    /* CMOV<inv_cond> rd, rn */
    uint8_t cmov_cond;
    switch (inv_cond) {
        case 0x0: cmov_cond = 0x44; break;
        case 0x1: cmov_cond = 0x45; break;
        case 0x2: cmov_cond = 0x43; break;
        case 0x3: cmov_cond = 0x42; break;
        case 0x4: cmov_cond = 0x48; break;
        case 0x5: cmov_cond = 0x49; break;
        case 0x6: cmov_cond = 0x40; break;
        case 0x7: cmov_cond = 0x41; break;
        case 0x8: cmov_cond = 0x47; break;
        case 0x9: cmov_cond = 0x46; break;
        case 0xA: cmov_cond = 0x4D; break;
        case 0xB: cmov_cond = 0x4C; break;
        case 0xC: cmov_cond = 0x4F; break;
        case 0xD: cmov_cond = 0x4E; break;
        default:
            return 0;
    }

    code_buf_emit_byte(code_buf, 0x48);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, cmov_cond);
    code_buf_emit_byte(code_buf, 0xC0 + ((x86_rd & 7) << 3) + (x86_rn & 7));

    return 0;
}

/* ============================================================================
 * Floating Point Conditional Select
 * ============================================================================ */

/**
 * translate_cond_fcsel - Translate FCSEL instruction
 * FCSEL Vd, Vn, Vm, cond  ->  if (cond) Vd = Vn; else Vd = Vm;
 */
int translate_cond_fcsel(uint32_t encoding, code_buf_t *code_buf,
                         uint64_t *guest_state, uint32_t *pstate)
{
    /* Placeholder for floating point conditional select */
    /* Full implementation would handle SIMD/FP registers */
    (void)encoding;
    (void)code_buf;
    (void)guest_state;
    (void)pstate;

    /* For now, treat as NOP - FP support requires additional infrastructure */
    return 0;
}

/* ============================================================================
 * Dispatch Function
 * ============================================================================ */

/**
 * translate_cond_dispatch - Dispatch conditional instruction
 */
int translate_cond_dispatch(uint32_t encoding, code_buf_t *code_buf,
                            uint64_t *guest_state, uint32_t *pstate)
{
    /* CSEL: 1A800000 (conditional select) */
    if ((encoding & COND_CSEL_MASK) == COND_CSEL_VAL) {
        return translate_cond_csel(encoding, code_buf, guest_state, pstate);
    }

    /* CSET: 1A9F07E0 (condition set to 1 or 0) */
    if ((encoding & COND_CSET_MASK) == COND_CSET_VAL) {
        return translate_cond_cset(encoding, code_buf, guest_state, pstate);
    }

    /* CSETM: 1A9F03E0 (condition set to all-ones or zero) */
    if ((encoding & COND_CSETM_MASK) == COND_CSETM_VAL) {
        return translate_cond_csetm(encoding, code_buf, guest_state, pstate);
    }

    return -1;  /* Not a conditional instruction */
}
