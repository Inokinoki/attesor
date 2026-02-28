/* ============================================================================
 * Rosetta Refactored - NEON Arithmetic Operations
 * ============================================================================
 *
 * This module implements ARM64 NEON/SIMD arithmetic instruction translation
 * to x86_64 machine code.
 *
 * Supported instructions:
 * - ADD, SUB (integer vector)
 * - MUL, MLA, MLS (integer multiply)
 * - SMULL, SMULH, UMULL, UMULH (widening multiply)
 * - SADALP, SADDL, SADDW (saturating/add long)
 * ============================================================================ */

#include "rosetta_refactored_neon.h"
#include "rosetta_emit_x86.h"
#include "rosetta_insn_common.h"
#include <stdint.h>

/* Map ARM64 V0-V31 to x86_64 XMM0-XMM15 */
static inline uint8_t neon_to_xmm(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* ============================================================================
 * Vector Integer Addition
 * ============================================================================ */

/**
 * translate_neon_add - Translate ARM64 ADD (vector) instruction
 * ADD Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_add(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* MOVAPD - Copy Vn to Vd first */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* Select instruction based on element size */
    switch (size) {
        case 0: /* 8B/16B - byte */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xFC);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 1: /* 4H/8H - halfword */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xFD);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 2: /* 4S - single */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xFE);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 3: /* 2D - double */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xD4);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_sub - Translate ARM64 SUB (vector) instruction
 * SUB Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_sub(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* MOVAPD - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    switch (size) {
        case 0: /* 8B/16B */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xF8);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 1: /* 4H/8H */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xF9);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 2: /* 4S */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xFA);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 3: /* 2D */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xFB);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    (void)vec_regs;
    return 0;
}

/* ============================================================================
 * Vector Integer Multiplication
 * ============================================================================ */

/**
 * translate_neon_mul - Translate ARM64 MUL (vector) instruction
 * MUL Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_mul(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* MOVAPD - Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    switch (size) {
        case 0: /* 8B/16B - byte multiply */
        case 1: /* 4H/8H - halfword multiply */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xD5);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 2: /* 4S - word multiply */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x38);
            code_buf_emit_byte(code_buf, 0x40);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_mla - Translate ARM64 MLA (vector) instruction
 * MLA Vd.<T>, Vn.<T>, Vm.<T>  ; Vd = Vd + Vn * Vm
 */
int translate_neon_mla(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);
    uint8_t xmm_tmp = 0;  /* Use XMM0 as temp */

    /* Copy Vn to temp register */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x41);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rn);

    /* Multiply temp by Vm */
    if (size == 2) {
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x38);
        code_buf_emit_byte(code_buf, 0x40);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rm);
    } else {
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xD5);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rm);
    }

    /* Add result to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xFE);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_tmp);

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_mls - Translate ARM64 MLS (vector) instruction
 * MLS Vd.<T>, Vn.<T>, Vm.<T>  ; Vd = Vd - Vn * Vm
 */
int translate_neon_mls(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);
    uint8_t xmm_tmp = 0;

    /* Copy Vn to temp */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x41);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rn);

    /* Multiply temp by Vm */
    if (size == 2) {
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x38);
        code_buf_emit_byte(code_buf, 0x40);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rm);
    } else {
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xD5);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_tmp << 3) + xmm_rm);
    }

    /* Subtract result from Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xFA);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_tmp);

    (void)vec_regs;
    return 0;
}

/* ============================================================================
 * Widening Multiplication (SMULL, UMULL, etc.)
 * ============================================================================ */

/**
 * translate_neon_smull - Translate ARM64 SMULL (signed widening multiply long)
 * SMULL Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_smull(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);
    uint8_t q = (encoding >> 30) & 1;

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    if (!q) {
        /* SMULL - lower half widening multiply */
        /* PMULLW for 16-bit, PMULLD for 32-bit */
        if (size == 1) {
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0xD5);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        } else if (size == 2) {
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x38);
            code_buf_emit_byte(code_buf, 0x40);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        }
    }

    (void)vec_regs;
    (void)size;
    return 0;
}

/**
 * translate_neon_umull - Translate ARM64 UMULL (unsigned widening multiply long)
 * UMULL Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_umull(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* For unsigned, need to use PMULLU or emulate */
    /* Simplified: same as signed for lower bits */
    return translate_neon_smull(encoding, code_buf, vec_regs);
}

/**
 * translate_neon_smulh - Translate ARM64 SMULH (signed multiply high)
 * SMULH Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_smulh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* PMULHW - Packed Multiply High Word */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xE5);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    (void)vec_regs;
    (void)xmm_rn;
    return 0;
}

/**
 * translate_neon_umulh - Translate ARM64 UMULH (unsigned multiply high)
 * UMULH Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_umulh(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* PMULHUW - Packed Multiply High Unsigned Word */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0xE4);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    (void)vec_regs;
    (void)xmm_rn;
    return 0;
}

/* ============================================================================
 * Saturating/Add Long Operations
 * ============================================================================ */

/**
 * translate_neon_saddl - Translate ARM64 SADDL (signed add long)
 * SADDL Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_saddl(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* MOVD - First move to clear upper bits */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* PADDW/PADDD for widening add */
    if (size == 0) {
        /* 8B -> 8H: MOVSX then ADD */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xFB);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
    } else if (size == 1) {
        /* 4H -> 4S */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xFE);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_sadalp - Translate ARM64 SADALP (signed saturating add long pairwise)
 * SADALP Vd.<T>, Vn.<T>
 */
int translate_neon_sadalp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);

    /* PHADDW/PHADDD for pairwise add */
    if (size == 0 || size == 1) {
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x38);
        code_buf_emit_byte(code_buf, 0x01);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);
    }

    (void)vec_regs;
    return 0;
}
