#ifndef ROSETTA_TRANSLATE_H
#define ROSETTA_TRANSLATE_H

/* ============================================================================
 * Rosetta Binary Translator - Instruction Translation Layer
 * ============================================================================ */

#include "rosetta_types.h"
#include "rosetta_codegen.h"

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

/**
 * Map ARM64 GPR to x86_64 GPR
 */
u8 x86_map_gpr(u8 arm64_reg);

/**
 * Map ARM64 vector register to x86_64 XMM
 */
u8 x86_map_xmm(u8 arm64_vreg);

/* ============================================================================
 * Data Processing - Register Instructions
 * ============================================================================ */

void translate_add_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_sub_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_and_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_orr_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_eor_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_mvn_reg(code_buffer_t *buf, u8 rd, u8 rm);
void translate_mul_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);
void translate_sdiv_reg(code_buffer_t *buf, u8 rd, u8 rn, u8 rm);

/* ============================================================================
 * Data Processing - Immediate Instructions
 * ============================================================================ */

void translate_add_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_sub_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_movz(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw);
void translate_movk(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw);
void translate_movn(code_buffer_t *buf, u8 rd, u16 imm16, u8 hw);

/* ============================================================================
 * Compare Instructions
 * ============================================================================ */

void translate_cmp_reg(code_buffer_t *buf, u8 rn, u8 rm);
void translate_cmn_reg(code_buffer_t *buf, u8 rn, u8 rm);
void translate_tst_reg(code_buffer_t *buf, u8 rn, u8 rm);

/* ============================================================================
 * Branch Instructions
 * ============================================================================ */

u32 translate_b(code_buffer_t *buf, s32 imm26);
u32 translate_bl(code_buffer_t *buf, u8 lr_reg, u64 ret_addr, s32 imm26);
void translate_br(code_buffer_t *buf, u8 rm);
void translate_ret(code_buffer_t *buf);
u32 translate_cond_branch(code_buffer_t *buf, u8 cond);

/* ============================================================================
 * Load/Store Instructions
 * ============================================================================ */

void translate_ldr_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_str_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldrb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_strb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldrh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_strh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldrsb_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldrsh_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldrsw_imm(code_buffer_t *buf, u8 rd, u8 rn, u32 imm);
void translate_ldur(code_buffer_t *buf, u8 rd, u8 rn, s32 simm);
void translate_stur(code_buffer_t *buf, u8 rd, u8 rn, s32 simm);

/* ============================================================================
 * Load/Store Pair Instructions
 * ============================================================================ */

void translate_ldp(code_buffer_t *buf, u8 rd, u8 rt, u8 rn, u32 imm);
void translate_stp(code_buffer_t *buf, u8 rd, u8 rt, u8 rn, u32 imm);

/* ============================================================================
 * System Instructions
 * ============================================================================ */

void translate_svc(code_buffer_t *buf, u16 imm16);
void translate_brk(code_buffer_t *buf, u16 imm16);
void translate_hlt(code_buffer_t *buf, u16 imm16);

/* ============================================================================
 * NEON Load/Store Instructions
 * ============================================================================ */

void translate_ld1(code_buffer_t *buf, u8 vd, u8 rn, u8 reg_count);
void translate_st1(code_buffer_t *buf, u8 vd, u8 rn, u8 reg_count);
void translate_ld2(code_buffer_t *buf, u8 vd, u8 vt, u8 rn);
void translate_st2(code_buffer_t *buf, u8 vd, u8 vt, u8 rn);
void translate_ld3(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 rn);
void translate_st3(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 rn);
void translate_ld4(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 v3, u8 rn);
void translate_st4(code_buffer_t *buf, u8 vd, u8 vt, u8 v2, u8 v3, u8 rn);

/* ============================================================================
 * NEON Vector Arithmetic
 * ============================================================================ */

void translate_add_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, u8 size);
void translate_sub_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, u8 size);
void translate_and_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm);
void translate_orr_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm);
void translate_eor_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm);
void translate_bic_vec(code_buffer_t *buf, u8 vd, u8 vn, u8 vm);

/* ============================================================================
 * Floating Point Instructions
 * ============================================================================ */

void translate_fmov_reg(code_buffer_t *buf, u8 vd, u8 vn, int is_double);
void translate_fadd(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double);
void translate_fsub(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double);
void translate_fmul(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double);
void translate_fdiv(code_buffer_t *buf, u8 vd, u8 vn, u8 vm, int is_double);
void translate_fsqrt(code_buffer_t *buf, u8 vd, u8 vn, int is_double);
void translate_fcmp(code_buffer_t *buf, u8 vn, u8 vm, int is_double);
void translate_fcvtds(code_buffer_t *buf, u8 vd, u8 vn);
void translate_fcvtsd(code_buffer_t *buf, u8 vd, u8 vn);

#endif /* ROSETTA_TRANSLATE_H */
