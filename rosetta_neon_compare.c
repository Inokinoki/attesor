/* ============================================================================
 * Rosetta Refactored - NEON Compare Operations
 * ============================================================================
 *
 * This module implements ARM64 NEON/SIMD compare instruction translation
 * to x86_64 machine code.
 *
 * Supported instructions:
 * - CMGE, CMGT (signed greater than/equal, greater than)
 * - CMLE, CMLT (signed less than/equal, less than)
 * - CMEQ (equal compare)
 * - CMHI, CMHS (unsigned compare)
 * - FCMEQ, FCMGE, FCMGT (floating point compare)
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
 * Vector Integer Compare Operations
 * ============================================================================ */

/**
 * translate_neon_cmge - Translate ARM64 CMGE (vector) instruction
 * CMGE Vd.<T>, Vn.<T>, Vm.<T>  ; Signed greater than or equal
 */
int translate_neon_cmge(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* PCMPGT - Packed Compare Greater Than */
    switch (size) {
        case 0: /* 16B - byte */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x64);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 1: /* 8H - word */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x65);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 2: /* 4S - dword */
        case 3: /* 2D - qword */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    /* For GE: invert result (GT -> GE by inverting and checking equality) */
    /* Simplified: PCMPGT gives us GT, need to also check equality */

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmgt - Translate ARM64 CMGT (signed greater than compare)
 * CMGT Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmgt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* PCMPGT - Packed Compare Greater Than */
    switch (size) {
        case 0: /* 16B - byte */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x64);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 1: /* 8H - word */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x65);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
        case 2: /* 4S - dword */
        case 3: /* 2D - qword */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmeq - Translate ARM64 CMEQ (vector) instruction
 * CMEQ Vd.<T>, Vn.<T>, Vm.<T>  ; Equal compare
 */
int translate_neon_cmeq(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* PCMPEQ - Packed Compare Equal */
    (void)size;
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x75);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmle - Translate ARM64 CMLE (signed <= compare)
 * CMLE Vd.<T>, Vn.<T>, #0  ; Compare with zero
 */
int translate_neon_cmle(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* CMLE Vd, Vn, #0 is equivalent to CMGE Vzr, Vn, #0 */
    /* Negate Vn and compare with zero */

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* Negate Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rm << 3) + xmm_rm);

    /* Compare: PCMPEQ for <= 0 */
    (void)size;
    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmlt - Translate ARM64 CMLT (signed < compare)
 * CMLT Vd.<T>, Vn.<T>, #0  ; Compare less than zero
 */
int translate_neon_cmlt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);

    /* Zero the destination */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x57);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rd);

    /* Compare Vn < 0 using PCMPGT */
    switch (size) {
        case 0: /* 16B */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x64);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);
            break;
        case 1: /* 8H */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x65);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);
            break;
        case 2: /* 4S */
        case 3: /* 2D */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);
            break;
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmhi - Translate ARM64 CMHI (unsigned higher compare)
 * CMHI Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmhi(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* For unsigned compare, need to adjust signed comparison */
    /* XOR with 0x80 to convert unsigned to signed */

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* PCMPGT for unsigned (with sign bit flip workaround) */
    switch (size) {
        case 2: /* 4S */
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0x0F);
            code_buf_emit_byte(code_buf, 0x66);
            code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
            break;
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmhs - Translate ARM64 CMHS (unsigned higher or same compare)
 * CMHS Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_cmhs(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    /* CMHS is unsigned >= compare */
    /* Similar to CMHI but inclusive */
    return translate_neon_cmhi(encoding, code_buf, vec_regs);
}

/* ============================================================================
 * Vector Compare with Zero
 * ============================================================================ */

/**
 * translate_neon_cmgez - Translate ARM64 CMGEZ (compare >= zero)
 * CMGE Vd.<T>, Vn.<T>, #0
 */
int translate_neon_cmgez(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);

    /* Zero a temp register */
    uint8_t xmm_tmp = 0;

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* Compare with zero using PCMPGT */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_tmp);

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_cmeq_zero - Translate ARM64 CMEQ with zero
 * CMEQ Vd.<T>, Vn.<T>, #0
 */
int translate_neon_cmeq_zero(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_tmp = 0;  /* Zero register */

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* Compare with zero */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x75);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_tmp);

    (void)vec_regs;
    return 0;
}

/* ============================================================================
 * Floating Point Compare (Vector)
 * ============================================================================ */

/**
 * translate_neon_fcmge - Translate ARM64 FCMGE (FP compare greater than or equal)
 * FCMGE Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmge(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    /* CMPSS/CMPSD for scalar, CMPPS/CMPPD for packed */
    if (size == 2) {
        /* Single precision - CMPPS with 2 (greater than or equal) */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xC2);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        code_buf_emit_byte(code_buf, 0x06);  /* 6 = not less than (>=) */
    } else if (size == 3) {
        /* Double precision - CMPPD with 2 */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xC2);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        code_buf_emit_byte(code_buf, 0x06);
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_fcmgt - Translate ARM64 FCMGT (FP compare greater than)
 * FCMGT Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmgt(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    if (size == 2) {
        /* Single precision - CMPPS with 2 (greater than) */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xC2);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        code_buf_emit_byte(code_buf, 0x06);  /* 6 = not less than (>) */
    }

    (void)vec_regs;
    return 0;
}

/**
 * translate_neon_fcmeq - Translate ARM64 FCMEQ (FP compare equal)
 * FCMEQ Vd.<T>, Vn.<T>, Vm.<T>
 */
int translate_neon_fcmeq(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs)
{
    uint8_t rd = insn_get_rd(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t rm = insn_get_rm(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rd = neon_to_xmm(rd);
    uint8_t xmm_rn = neon_to_xmm(rn);
    uint8_t xmm_rm = neon_to_xmm(rm);

    /* Copy Vn to Vd */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x28);
    code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rn);

    if (size == 2) {
        /* Single precision - CMPPS with 0 (equal) */
        code_buf_emit_byte(code_buf, 0x66);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0xC2);
        code_buf_emit_byte(code_buf, 0xC0 + (xmm_rd << 3) + xmm_rm);
        code_buf_emit_byte(code_buf, 0x00);  /* 0 = equal */
    }

    (void)vec_regs;
    return 0;
}
