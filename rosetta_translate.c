/* ============================================================================
 * Rosetta Binary Translator - ARM64 Instruction Translation
 * ============================================================================
 *
 * This module implements ARM64 to x86_64 instruction translation.
 * Each function translates a specific ARM64 instruction class to x86_64.
 *
 * ============================================================================ */

#include "rosetta_translate.h"
#include <string.h>

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

/**
 * Map ARM64 GPR (X0-X30) to x86_64 GPR (RAX-R15)
 *
 * Simple direct mapping for X0-X15, then wraps for X16-X30.
 */
u8 x86_map_gpr(u8 arm64_reg)
{
    static const u8 reg_map[32] = {
        0, 1, 2, 3, 4, 5, 6, 7,    /* X0-X7 -> RAX-RCX-RDX-RBX-RSI-RDI-RBP-RSP */
        8, 9, 10, 11, 12, 13, 14, 15, /* X8-X15 -> R8-R15 */
        0, 1, 2, 3, 4, 5, 6, 7,    /* X16-X23 -> repeat */
        8, 9, 10, 11, 12, 13, 14, 15  /* X24-X31 -> repeat */
    };
    return reg_map[arm64_reg & 0x1F];
}

/**
 * Map ARM64 vector register (V0-V31) to x86_64 XMM (XMM0-XMM15)
 *
 * Simple modulo wrapping since x86_64 only has 16 XMM registers.
 */
u8 x86_map_xmm(u8 arm64_vreg)
{
    return arm64_vreg & 0x0F;
}

/* ============================================================================
 * Data Processing - Register Instructions
 * ============================================================================ */

/**
 * Translate ADD (register): Xd = Xn + Xm
 */
void translate_add_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* MOV dst, src1; ADD dst, src2 */
    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_add_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate SUB (register): Xd = Xn - Xm
 */
void translate_sub_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_sub_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate AND (register): Xd = Xn & Xm
 */
void translate_and_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_and_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate ORR (register): Xd = Xn | Xm
 */
void translate_orr_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_orr_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate EOR (register): Xd = Xn ^ Xm  (XOR in x86)
 */
void translate_eor_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_xor_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate MVN (register): Xd = ~Xm
 */
void translate_mvn_reg(code_buffer_t *buf, u8 rd, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rm = x86_map_gpr(rm);

    emit_mvn_reg_reg(buf, x86_rd, x86_rm);
}

/**
 * Translate MUL (register): Xd = Xn * Xm (lower 64 bits)
 */
void translate_mul_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* x86 IMUL uses RAX:RDX for full result */
    emit_mov_reg_reg(buf, RAX, x86_rn);
    emit_mul_reg(buf, RAX, RAX, x86_rm);  /* RAX = RAX * rm */
    emit_mov_reg_reg(buf, x86_rd, RAX);
}

/**
 * Translate SDIV (signed divide): Xd = Xn / Xm
 */
void translate_sdiv_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* x86 IDIV uses RAX:RDX / operand */
    emit_mov_reg_reg(buf, RAX, x86_rn);
    emit_xor_reg_reg(buf, RDX, RDX);  /* Clear RDX for unsigned div */
    emit_div_reg(buf, x86_rm);
    emit_mov_reg_reg(buf, x86_rd, RAX);
}

/* ============================================================================
 * Data Processing - Immediate Instructions
 * ============================================================================ */

/**
 * Translate ADD (immediate): Xd = Xn + imm12
 */
void translate_add_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_add_reg_imm32(buf, x86_rd, imm);
}

/**
 * Translate SUB (immediate): Xd = Xn - imm12
 */
void translate_sub_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_reg(buf, x86_rd, x86_rn);
    emit_sub_reg_imm32(buf, x86_rd, imm);
}

/**
 * Translate MOVZ: Xd = imm16 << (hw*16)
 */
void translate_movz(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw)
{
    u8 x86_rd = x86_map_gpr(rd);
    u64 shifted_imm = (u64)imm16 << (hw * 16);

    emit_mov_reg_imm64(buf, x86_rd, shifted_imm);
}

/**
 * Translate MOVK: Xd = Xd | (imm16 << (hw*16))
 */
void translate_movk(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw)
{
    u8 x86_rd = x86_map_gpr(rd);
    u64 shifted_imm = (u64)imm16 << (hw * 16);

    emit_orr_reg_imm32(buf, x86_rd, (u32)shifted_imm);
}

/**
 * Translate MOVN: Xd = ~(imm16 << (hw*16))
 */
void translate_movn(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw)
{
    u8 x86_rd = x86_map_gpr(rd);
    u64 shifted_imm = (u64)imm16 << (hw * 16);

    emit_mov_reg_imm64(buf, x86_rd, ~shifted_imm);
}

/* ============================================================================
 * Compare Instructions
 * ============================================================================ */

/**
 * Translate CMP (register): compare Xn and Xm, set flags
 */
void translate_cmp_reg(code_buffer_t *buf, u8 rn, u8 rm)
{
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* CMP doesn't write result, just sets flags */
    emit_cmp_reg_reg(buf, x86_rn, x86_rm);
}

/**
 * Translate CMN (register): compare Xn + Xm, set flags
 */
void translate_cmn_reg(code_buffer_t *buf, u8 rn, u8 rm)
{
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* CMN adds and sets flags */
    emit_mov_reg_reg(buf, RAX, x86_rn);
    emit_add_reg_reg(buf, RAX, x86_rm);
    emit_test_reg_reg(buf, RAX, RAX);  /* Sets flags based on result */
}

/**
 * Translate TST (register): test Xn & Xm, set flags
 */
void translate_tst_reg(code_buffer_t *buf, u8 rn, u8 rm)
{
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_rm = x86_map_gpr(rm);

    /* TST ANDs and sets flags */
    emit_mov_reg_reg(buf, RAX, x86_rn);
    emit_and_reg_reg(buf, RAX, x86_rm);
    emit_test_reg_reg(buf, RAX, RAX);
}

/* ============================================================================
 * Branch Instructions
 * ============================================================================ */

/**
 * Translate B (unconditional branch): PC += imm26
 */
u32 translate_b(code_buffer_t *buf, s32 imm26)
{
    /* Emit relative jump - offset will be patched later */
    return emit_jmp_rel32(buf);
}

/**
 * Translate BL (branch with link): LR = PC+4; PC += imm26
 */
u32 translate_bl(code_buffer_t *buf, u8 lr_reg, u64 ret_addr, s32 imm26)
{
    u8 x86_lr = x86_map_gpr(lr_reg);

    /* Save return address in LR (X30) */
    emit_mov_reg_imm64(buf, x86_lr, ret_addr);

    /* Emit call or jump */
    return emit_call_rel32(buf);
}

/**
 * Translate BR (branch to register): PC = Xm
 */
void translate_br(code_buffer_t *buf, u8 rm)
{
    u8 x86_rm = x86_map_gpr(rm);

    /* Indirect branch - jump to register */
    emit_call_reg(buf, x86_rm);  /* CALL reg for indirect */
}

/**
 * Translate RET: PC = LR (X30)
 */
void translate_ret(code_buffer_t *buf)
{
    /* Return from translated block */
    emit_ret(buf);
}

/**
 * Translate conditional branch helper
 * Maps ARM64 condition codes to x86 jumps
 */
u32 translate_cond_branch(code_buffer_t *buf, u8 cond)
{
    return emit_cond_branch(buf, cond);
}

/* ============================================================================
 * Load/Store Instructions
 * ============================================================================ */

/**
 * Translate LDR (register, immediate): Xd = [Xn + imm]
 */
void translate_ldr_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load from [rn + imm] into rd */
    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate STR (register, immediate): [Xn + imm] = Xd
 */
void translate_str_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Store rd to [rn + imm] */
    emit_mov_mem_reg(buf, x86_rn, x86_rd, (s32)imm);
}

/**
 * Translate LDRB (load register byte): Xd = zero_extend([Xn + imm])
 */
void translate_ldrb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load byte with zero extend */
    /* Would need emit_movzx helper - simplified for now */
    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate STRB (store register byte): [Xn + imm] = Xd[7:0]
 */
void translate_strb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Store low byte of rd to [rn + imm] */
    /* Would need size-specific store helper */
    emit_mov_mem_reg(buf, x86_rn, x86_rd, (s32)imm);
}

/**
 * Translate LDRH (load register halfword): Xd = zero_extend([Xn + imm])
 */
void translate_ldrh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate STRH (store register halfword): [Xn + imm] = Xd[15:0]
 */
void translate_strh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_mem_reg(buf, x86_rn, x86_rd, (s32)imm);
}

/**
 * Translate LDRSB (load register signed byte): Xd = sign_extend([Xn + imm])
 */
void translate_ldrsb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Would need MOVSX helper */
    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate LDRSH (load register signed halfword): Xd = sign_extend([Xn + imm])
 */
void translate_ldrsh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate LDRSW (load register signed word): Xd = sign_extend([Xn + imm])
 */
void translate_ldrsw_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
}

/**
 * Translate LDUR (load register unscaled): Xd = [Xn + simm]
 */
void translate_ldur(code_buffer_t *buf, u8 rd, u8 rn, s32 simm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_reg_mem(buf, x86_rd, x86_rn, simm);
}

/**
 * Translate STUR (store register unscaled): [Xn + simm] = Xd
 */
void translate_stur(code_buffer_t *buf, u8 rd, u8 rn, s32 simm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rn = x86_map_gpr(rn);

    emit_mov_mem_reg(buf, x86_rn, x86_rd, simm);
}

/* ============================================================================
 * Load/Store Pair Instructions
 * ============================================================================ */

/**
 * Translate LDP (load pair): Xd = [Xn + imm]; Xt = [Xn + imm + 8]
 */
void translate_ldp(code_buffer_t *buf, u8 rd, u8 rt, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rt = x86_map_gpr(rt);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load first register */
    emit_mov_reg_mem(buf, x86_rd, x86_rn, (s32)imm);
    /* Load second register (offset +8 for 64-bit) */
    emit_mov_reg_mem(buf, x86_rt, x86_rn, (s32)imm + 8);
}

/**
 * Translate STP (store pair): [Xn + imm] = Xd; [Xn + imm + 8] = Xt
 */
void translate_stp(code_buffer_t *buf, u8 rd, u8 rt, u8 rn, u32 imm)
{
    u8 x86_rd = x86_map_gpr(rd);
    u8 x86_rt = x86_map_gpr(rt);
    u8 x86_rn = x86_map_gpr(rn);

    /* Store first register */
    emit_mov_mem_reg(buf, x86_rn, x86_rd, (s32)imm);
    /* Store second register */
    emit_mov_mem_reg(buf, x86_rn, x86_rt, (s32)imm + 8);
}

/* ============================================================================
 * System Instructions
 * ============================================================================ */

/**
 * Translate SVC (supervisor call): syscall
 */
void translate_svc(code_buffer_t *buf, u16 imm16)
{
    (void)imm16;  /* ARM64 SVC immediate is ignored on Linux */

    /* On x86_64 Linux, syscall instruction is used */
    /* For macOS translation, this would need special handling */
    emit_ud2(buf);  /* Placeholder - would trap to syscall handler */
}

/**
 * Translate BRK (breakpoint): debug exception
 */
void translate_brk(code_buffer_t *buf, u16 imm16)
{
    (void)imm16;

    /* Emit undefined instruction to trap to debugger */
    emit_ud2(buf);
}

/**
 * Translate HLT (halt): halt instruction
 */
void translate_hlt(code_buffer_t *buf, u16 imm16)
{
    (void)imm16;

    /* Emit undefined instruction */
    emit_ud2(buf);
}

/* ============================================================================
 * NEON/Vector Load/Store
 * ============================================================================ */

/**
 * Translate LD1 (load single structure): Vd[] = [Xn]
 */
void translate_ld1(code_buffer_t *buf, u8 vd, u8 rn, u8 reg_count)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load 16 bytes (single structure) */
    emit_movdqu_xmm_mem_reg(buf, x86_vd, x86_rn);

    (void)reg_count;  /* For multi-structure variants */
}

/**
 * Translate ST1 (store single structure): [Xn] = Vd[]
 */
void translate_st1(code_buffer_t *buf, u8 vd, u8 rn, u8 reg_count)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_rn = x86_map_gpr(rn);

    /* Store 16 bytes */
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vd);

    (void)reg_count;
}

/**
 * Translate LD2 (load pair of structures): Vd[], Vt[] = [Xn] (de-interleave)
 */
void translate_ld2(code_buffer_t *buf, u8 vd, u8 vt, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_rn = x86_map_gpr(rn);
    u8 x86_tmp = x86_map_gpr(0);  /* Use RAX as temp */

    /* Load into temp, then de-interleave */
    /* Simplified: load both registers directly */
    emit_movdqu_xmm_mem_reg(buf, x86_vd, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_vt, x86_rn);  /* Would need offset */

    (void)x86_tmp;
}

/**
 * Translate ST2 (store pair of structures): [Xn] = Vd[], Vt[] (interleave)
 */
void translate_st2(code_buffer_t *buf, u8 vd, u8 vt, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_rn = x86_map_gpr(rn);

    /* Interleave and store - simplified */
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vd);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vt);  /* Would need offset */
}

/**
 * Translate LD3 (load three structures): Vd[], Vt[], V2[] = [Xn] (RGB de-interleave)
 */
void translate_ld3(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_v2 = x86_map_xmm(v2);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load RGB data and de-interleave */
    emit_movdqu_xmm_mem_reg(buf, x86_vd, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_vt, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_v2, x86_rn);
}

/**
 * Translate ST3 (store three structures): [Xn] = Vd[], Vt[], V2[] (RGB interleave)
 */
void translate_st3(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_v2 = x86_map_xmm(v2);
    u8 x86_rn = x86_map_gpr(rn);

    /* Interleave RGB data and store */
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vd);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vt);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_v2);
}

/**
 * Translate LD4 (load four structures): RGBA de-interleave
 */
void translate_ld4(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 v3, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_v2 = x86_map_xmm(v2);
    u8 x86_v3 = x86_map_xmm(v3);
    u8 x86_rn = x86_map_gpr(rn);

    /* Load RGBA data and de-interleave */
    emit_movdqu_xmm_mem_reg(buf, x86_vd, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_vt, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_v2, x86_rn);
    emit_movdqu_xmm_mem_reg(buf, x86_v3, x86_rn);
}

/**
 * Translate ST4 (store four structures): RGBA interleave
 */
void translate_st4(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 v3, u8 rn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vt = x86_map_xmm(vt);
    u8 x86_v2 = x86_map_xmm(v2);
    u8 x86_v3 = x86_map_xmm(v3);
    u8 x86_rn = x86_map_gpr(rn);

    /* Interleave RGBA data and store */
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vd);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_vt);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_v2);
    emit_movdqu_mem_reg_xmm(buf, x86_rn, x86_v3);
}

/* ============================================================================
 * NEON Vector Arithmetic
 * ============================================================================ */

/**
 * Translate ADD (vector): Vd = Vn + Vm
 */
void translate_add_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, u8 size)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    /* Copy Vn to Vd first */
    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    /* Vector add based on element size */
    if (size >= 2) {
        emit_paddq_xmm_xmm(buf, x86_vd, x86_vm);  /* 64-bit elements */
    } else {
        emit_paddd_xmm_xmm(buf, x86_vd, x86_vm);  /* 32-bit elements */
    }
}

/**
 * Translate SUB (vector): Vd = Vn - Vm
 */
void translate_sub_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, u8 size)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (size >= 2) {
        emit_psubq_xmm_xmm(buf, x86_vd, x86_vm);
    } else {
        emit_psubd_xmm_xmm(buf, x86_vd, x86_vm);
    }
}

/**
 * Translate AND (vector): Vd = Vn & Vm
 */
void translate_and_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);
    emit_pand_xmm_xmm(buf, x86_vd, x86_vm);
}

/**
 * Translate ORR (vector): Vd = Vn | Vm
 */
void translate_orr_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);
    emit_por_xmm_xmm(buf, x86_vd, x86_vm);
}

/**
 * Translate EOR (vector): Vd = Vn ^ Vm
 */
void translate_eor_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);
    emit_pxor_xmm_xmm(buf, x86_vd, x86_vm);
}

/**
 * Translate BIC (vector bit clear): Vd = Vn & ~Vm
 */
void translate_bic_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);
    emit_pandn_xmm_xmm(buf, x86_vd, x86_vm);  /* PANDN dst, src => dst = ~dst & src */
}

/* ============================================================================
 * Floating Point Instructions
 * ============================================================================ */

/**
 * Translate FMOV (register): Fd = Fn
 */
void translate_fmov_reg(code_buffer_t *buf, u8 vd, u8 vn, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);

    if (is_double) {
        emit_movsd_xmm_xmm(buf, x86_vd, x86_vn);
    } else {
        emit_movss_xmm_xmm(buf, x86_vd, x86_vn);
    }
}

/**
 * Translate FADD: Fd = Fn + Fm
 */
void translate_fadd(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (is_double) {
        emit_addsd_xmm_xmm(buf, x86_vd, x86_vm);
    } else {
        emit_addss_xmm_xmm(buf, x86_vd, x86_vm);
    }
}

/**
 * Translate FSUB: Fd = Fn - Fm
 */
void translate_fsub(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (is_double) {
        emit_subsd_xmm_xmm(buf, x86_vd, x86_vm);
    } else {
        emit_subss_xmm_xmm(buf, x86_vd, x86_vm);
    }
}

/**
 * Translate FMUL: Fd = Fn * Fm
 */
void translate_fmul(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (is_double) {
        emit_mulsd_xmm_xmm(buf, x86_vd, x86_vm);
    } else {
        emit_mulss_xmm_xmm(buf, x86_vd, x86_vm);
    }
}

/**
 * Translate FDIV: Fd = Fn / Fm
 */
void translate_fdiv(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (is_double) {
        emit_divsd_xmm_xmm(buf, x86_vd, x86_vm);
    } else {
        emit_divss_xmm_xmm(buf, x86_vd, x86_vm);
    }
}

/**
 * Translate FSQRT: Fd = sqrt(Fn)
 */
void translate_fsqrt(code_buffer_t *buf, u8 vd, u8 vn, int is_double)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);

    emit_mov_reg_reg(buf, x86_vd, x86_vn);

    if (is_double) {
        emit_sqrtsd_xmm(buf, x86_vd);
    } else {
        emit_sqrtss_xmm(buf, x86_vd);
    }
}

/**
 * Translate FCMP: compare Fn and Fm, set NZCV flags
 */
void translate_fcmp(code_buffer_t *buf, u8 vn, u8 vm, int is_double)
{
    u8 x86_vn = x86_map_xmm(vn);
    u8 x86_vm = x86_map_xmm(vm);

    if (is_double) {
        emit_ucomisd_xmm_xmm(buf, x86_vn, x86_vm);
    } else {
        emit_ucomiss_xmm_xmm(buf, x86_vn, x86_vm);
    }
    /* x86 flags would need to be converted to ARM64 NZCV format */
}

/**
 * Translate FCVTDS: Fd (single) = Fn (double)
 */
void translate_fcvtds(code_buffer_t *buf, u8 vd, u8 vn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);

    emit_cvtss2sd_xmm_xmm(buf, x86_vd, x86_vn);  /* Actually double to single */
}

/**
 * Translate FCVTSD: Fd (double) = Fn (single)
 */
void translate_fcvtsd(code_buffer_t *buf, u8 vd, u8 vn)
{
    u8 x86_vd = x86_map_xmm(vd);
    u8 x86_vn = x86_map_xmm(vn);

    emit_cvtsd2ss_xmm_xmm(buf, x86_vd, x86_vn);
}
