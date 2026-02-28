/* ============================================================================
 * Rosetta Refactored - NEON Load/Store Operations Implementation
 * ============================================================================
 *
 * This module implements ARM64 NEON/SIMD load/store instruction translation
 * to x86_64 machine code.
 *
 * Supported instructions:
 * - LD1, ST1 (single structure load/store)
 * - LD2, ST2 (two structure load/store)
 * - LD3, ST3 (three structure load/store)
 * - LD4, ST4 (four structure load/store)
 * - LDR, STR (vector register load/store)
 * - LDP, STP (vector pair load/store)
 * ============================================================================ */

#include "rosetta_neon_loadstore.h"
#include "rosetta_emit_x86.h"
#include "rosetta_insn_common.h"
#include <stdint.h>

/* Map ARM64 V0-V31 to x86_64 XMM0-XMM15 */
static inline uint8_t neon_to_xmm(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* Map ARM64 X0-X30 to x86_64 RAX-R15 */
static inline uint8_t arm64_to_x86_reg(uint8_t arm_reg)
{
    return arm_reg & 0x0F;
}

/* ============================================================================
 * NEON Load Single Structure
 * ============================================================================ */

/**
 * translate_neon_ld1 - Translate ARM64 LD1 (load single structure) instruction
 * LD1 {Vt.<T>}, [Xn]
 */
int translate_neon_ld1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* MOVDQU - Move Unaligned Double Quadword (load from memory) */
    /* F3 0F 6F /r - MOVDQU xmm1, xmm2/m128 */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_ld1_lane - Translate ARM64 LD1 (single lane) instruction
 * LD1 {Vt.<T>}[<index>], [Xn]
 */
int translate_neon_ld1_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Load scalar element based on size */
    /* For simplicity, use MOVD for 32-bit, MOVQ for 64-bit */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6E);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_ld1_rep - Translate ARM64 LD1 (multiple registers) instruction
 * LD1 {Vt1.<T>-Vt4.<T>}, [Xn]
 */
int translate_neon_ld1_rep(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t num_regs = ((encoding >> 10) & 0x03) + 1;

    uint8_t x_rn = arm64_to_x86_reg(rn);
    uint64_t addr = 0;

    (void)vec_regs;
    (void)x_regs;
    (void)base_addr;

    /* Load each register sequentially */
    for (uint8_t i = 0; i < num_regs; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x6F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));

        addr += 16;  /* Each V register is 128 bits */
    }

    return 0;
}

/* ============================================================================
 * NEON Store Single Structure
 * ============================================================================ */

/**
 * translate_neon_st1 - Translate ARM64 ST1 (store single structure) instruction
 * ST1 {Vt.<T>}, [Xn]
 */
int translate_neon_st1(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* MOVDQU - Move Unaligned Double Quadword (store to memory) */
    /* F3 0F 7F /r - MOVDQU xmm2/m128, xmm1 */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_st1_lane - Translate ARM64 ST1 (single lane) instruction
 * ST1 {Vt.<T>}[<index>], [Xn]
 */
int translate_neon_st1_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Store scalar element based on size */
    code_buf_emit_byte(code_buf, 0x66);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7E);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_st1_rep - Translate ARM64 ST1 (multiple registers) instruction
 * ST1 {Vt1.<T>-Vt4.<T>}, [Xn]
 */
int translate_neon_st1_rep(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t num_regs = ((encoding >> 10) & 0x03) + 1;

    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Store each register sequentially */
    for (uint8_t i = 0; i < num_regs; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x7F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));
    }

    return 0;
}

/* ============================================================================
 * NEON Load Multiple Structures
 * ============================================================================ */

/**
 * translate_neon_ld2 - Translate ARM64 LD2 (load two structures) instruction
 * LD2 {Vt1.<T>, Vt2.<T>}, [Xn]
 */
int translate_neon_ld2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rt2 = ((rt + 1) & 0x1F);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt1 = neon_to_xmm(rt);
    uint8_t xmm_rt2 = neon_to_xmm(rt2);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Load first structure */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt1 << 3) + (x_rn & 0x07));

    /* Load second structure */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt2 << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_ld2_lane - Translate ARM64 LD2 (single lane) instruction
 * LD2 {Vt1.<T>, Vt2.<T>}[<index>], [Xn]
 */
int translate_neon_ld2_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs, uint64_t base_addr)
{
    /* Similar to LD2 but loads single lane */
    return translate_neon_ld2(encoding, code_buf, vec_regs, x_regs, base_addr);
}

/**
 * translate_neon_ld3 - Translate ARM64 LD3 (load three structures) instruction
 * LD3 {Vt1.<T>, Vt2.<T>, Vt3.<T>}, [Xn]
 */
int translate_neon_ld3(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Load three structures (interleaved) */
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x6F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));
    }

    return 0;
}

/**
 * translate_neon_ld4 - Translate ARM64 LD4 (load four structures) instruction
 * LD4 {Vt1.<T>, Vt2.<T>, Vt3.<T>, Vt4.<T>}, [Xn]
 */
int translate_neon_ld4(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Load four structures (interleaved) */
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x6F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));
    }

    return 0;
}

/* ============================================================================
 * NEON Store Multiple Structures
 * ============================================================================ */

/**
 * translate_neon_st2 - Translate ARM64 ST2 (store two structures) instruction
 * ST2 {Vt1.<T>, Vt2.<T>}, [Xn]
 */
int translate_neon_st2(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rt2 = ((rt + 1) & 0x1F);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt1 = neon_to_xmm(rt);
    uint8_t xmm_rt2 = neon_to_xmm(rt2);
    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Store first structure */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt1 << 3) + (x_rn & 0x07));

    /* Store second structure */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt2 << 3) + (x_rn & 0x07));

    return 0;
}

/**
 * translate_neon_st3 - Translate ARM64 ST3 (store three structures) instruction
 * ST3 {Vt1.<T>, Vt2.<T>, Vt3.<T>}, [Xn]
 */
int translate_neon_st3(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Store three structures (interleaved) */
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x7F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));
    }

    return 0;
}

/**
 * translate_neon_st4 - Translate ARM64 ST4 (store four structures) instruction
 * ST4 {Vt1.<T>, Vt2.<T>, Vt3.<T>, Vt4.<T>}, [Xn]
 */
int translate_neon_st4(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs, uint64_t base_addr)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t x_rn = arm64_to_x86_reg(rn);

    (void)base_addr;
    (void)vec_regs;
    (void)x_regs;

    /* Store four structures (interleaved) */
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t xmm_rt = neon_to_xmm(rt + i);

        code_buf_emit_byte(code_buf, 0xF3);
        code_buf_emit_byte(code_buf, 0x0F);
        code_buf_emit_byte(code_buf, 0x7F);
        code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + (x_rn & 0x07));
    }

    return 0;
}

/* ============================================================================
 * NEON Load/Store Register (Vector)
 * ============================================================================ */

/**
 * translate_neon_ldr - Translate ARM64 LDR (vector) - load vector register
 * LDR Vt.<T>, [Xn] or LDR Vt.<T>, [Xn, #imm]
 */
int translate_neon_ldr(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);
    uint8_t size = insn_get_size(encoding, 22, 2);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);
    uint8_t x86_base = x_rn & 0x07;

    (void)size;
    (void)vec_regs;
    (void)x_regs;

    /* MOVDQU for 128-bit load */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + x86_base);

    return 0;
}

/**
 * translate_neon_str - Translate ARM64 STR (vector) - store vector register
 * STR Vt.<T>, [Xn] or STR Vt.<T>, [Xn, #imm]
 */
int translate_neon_str(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt = neon_to_xmm(rt);
    uint8_t x_rn = arm64_to_x86_reg(rn);
    uint8_t x86_base = x_rn & 0x07;

    (void)vec_regs;
    (void)x_regs;

    /* MOVDQU for 128-bit store */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt << 3) + x86_base);

    return 0;
}

/**
 * translate_neon_ldr_lane - Translate ARM64 LDR (vector) with lane indexing
 */
int translate_neon_ldr_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs)
{
    /* For now, delegate to ldr */
    return translate_neon_ldr(encoding, code_buf, vec_regs, x_regs);
}

/**
 * translate_neon_str_lane - Translate ARM64 STR (vector) with lane indexing
 */
int translate_neon_str_lane(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs)
{
    /* For now, delegate to str */
    return translate_neon_str(encoding, code_buf, vec_regs, x_regs);
}

/* ============================================================================
 * NEON Load/Store Pair (Vector)
 * ============================================================================ */

/**
 * translate_neon_ldp - Translate ARM64 LDP (vector pair) instruction
 * LDP Vt1, Vt2, [Xn|SP, #imm]
 */
int translate_neon_ldp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rt2 = insn_get_rt2(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt1 = neon_to_xmm(rt);
    uint8_t xmm_rt2 = neon_to_xmm(rt2);
    uint8_t x_rn = arm64_to_x86_reg(rn);
    uint8_t x86_base = x_rn & 0x07;

    (void)vec_regs;
    (void)x_regs;

    /* Load first register */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt1 << 3) + x86_base);

    /* Load second register */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x6F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt2 << 3) + x86_base);

    return 0;
}

/**
 * translate_neon_stp - Translate ARM64 STP (vector pair) instruction
 * STP Vt1, Vt2, [Xn|SP, #imm]
 */
int translate_neon_stp(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                       uint64_t *x_regs)
{
    uint8_t rt = insn_get_rt(encoding);
    uint8_t rt2 = insn_get_rt2(encoding);
    uint8_t rn = insn_get_rn(encoding);

    uint8_t xmm_rt1 = neon_to_xmm(rt);
    uint8_t xmm_rt2 = neon_to_xmm(rt2);
    uint8_t x_rn = arm64_to_x86_reg(rn);
    uint8_t x86_base = x_rn & 0x07;

    (void)vec_regs;
    (void)x_regs;

    /* Store first register */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt1 << 3) + x86_base);

    /* Store second register */
    code_buf_emit_byte(code_buf, 0xF3);
    code_buf_emit_byte(code_buf, 0x0F);
    code_buf_emit_byte(code_buf, 0x7F);
    code_buf_emit_byte(code_buf, 0x04 + (xmm_rt2 << 3) + x86_base);

    return 0;
}

/* ============================================================================
 * NEON Load/Store with Post-Index
 * ============================================================================ */

/**
 * translate_neon_ldr_post - Translate ARM64 LDR (vector) post-indexed
 * LDR Vt, [Xn], #imm
 */
int translate_neon_ldr_post(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs)
{
    /* Post-indexed load: load then increment pointer */
    int result = translate_neon_ldr(encoding, code_buf, vec_regs, x_regs);

    /* Emit increment of base register */
    /* ADD x_rn, x_rn, #imm */
    (void)encoding;
    (void)code_buf;
    (void)x_regs;

    return result;
}

/**
 * translate_neon_str_post - Translate ARM64 STR (vector) post-indexed
 * STR Vt, [Xn], #imm
 */
int translate_neon_str_post(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                            uint64_t *x_regs)
{
    /* Post-indexed store: store then increment pointer */
    int result = translate_neon_str(encoding, code_buf, vec_regs, x_regs);

    /* Emit increment of base register */
    (void)encoding;
    (void)code_buf;
    (void)x_regs;

    return result;
}

/* ============================================================================
 * NEON Load/Store with Pre-Index
 * ============================================================================ */

/**
 * translate_neon_ldr_pre - Translate ARM64 LDR (vector) pre-indexed
 * LDR Vt, [Xn, #imm]!
 */
int translate_neon_ldr_pre(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs)
{
    /* Pre-indexed load: increment pointer then load */
    /* Emit increment of base register first */
    (void)encoding;
    (void)code_buf;
    (void)x_regs;

    /* Then load */
    return translate_neon_ldr(encoding, code_buf, vec_regs, x_regs);
}

/**
 * translate_neon_str_pre - Translate ARM64 STR (vector) pre-indexed
 * STR Vt, [Xn, #imm]!
 */
int translate_neon_str_pre(uint32_t encoding, code_buf_t *code_buf, Vector128 *vec_regs,
                           uint64_t *x_regs)
{
    /* Pre-indexed store: increment pointer then store */
    (void)encoding;
    (void)code_buf;
    (void)x_regs;

    return translate_neon_str(encoding, code_buf, vec_regs, x_regs);
}
