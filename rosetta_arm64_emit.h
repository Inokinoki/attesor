/* ============================================================================
 * Rosetta ARM64 Code Emitter
 * ============================================================================
 *
 * This module handles emission of ARM64 instructions for the JIT compiler.
 * Each function emits a specific ARM64 instruction encoding.
 * ============================================================================ */

#ifndef ROSETTA_ARM64_EMIT_H
#define ROSETTA_ARM64_EMIT_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* Use common code_buffer_t type from rosetta_types.h */

/* ============================================================================
 * Code Buffer Management
 * ============================================================================ */

/**
 * Initialize code buffer for ARM64 emission
 * @param buf Code buffer to initialize
 * @param buffer_ptr Pre-allocated buffer or NULL for internal cache
 * @param size Buffer size
 */
void code_buffer_init_arm64(code_buffer_t *buf, u8 *buffer_ptr, u32 size);

/**
 * Get code size
 * @param buf Code buffer
 * @return Current code size
 */
u32 code_buffer_get_size_arm64(code_buffer_t *buf);

/* ============================================================================
 * Basic Emit Functions
 * ============================================================================ */

void emit_byte_arm64(code_buffer_t *buf, u8 byte);
void emit_word32_arm64(code_buffer_t *buf, u32 word);
void emit_arm64_insn(code_buffer_t *buf, u32 insn);

/* ============================================================================
 * Register Constants
 * ============================================================================ */

#define XZR 31  /* ARM64 zero register */
#define WZR 31  /* ARM64 zero register (32-bit) */

/* x86_64 to ARM64 register mapping */
uint8_t map_x86_to_arm(uint8_t x86_reg);
uint8_t x86_map_gpr(uint8_t arm64_reg);
uint8_t x86_map_xmm(uint8_t arm64_vreg);

/* ============================================================================
 * Data Processing (Register) - ALU Operations
 * ============================================================================ */

void emit_add_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sub_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_and_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_and_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_orr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_eor_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_mov_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* ============================================================================
 * Data Processing (Immediate)
 * ============================================================================ */

void emit_add_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_sub_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint16_t imm);
void emit_movz(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movk(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);
void emit_movn(code_buffer_t *buf, uint8_t dst, uint16_t imm, uint8_t shift);

/* ============================================================================
 * Multiply/Divide
 * ============================================================================ */

void emit_mul_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_sdiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);
void emit_udiv_reg(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2);

/* ============================================================================
 * Compare/Test
 * ============================================================================ */

void emit_cmp_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);
void emit_cmn_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);
void emit_tst_reg(code_buffer_t *buf, uint8_t src1, uint8_t src2);

/* ============================================================================
 * Branch Instructions
 * ============================================================================ */

void emit_b(code_buffer_t *buf, int32_t imm26);
void emit_bl(code_buffer_t *buf, int32_t imm26);
void emit_bcond(code_buffer_t *buf, uint8_t cond, int32_t imm19);
void emit_br(code_buffer_t *buf, uint8_t src);
void emit_ret(code_buffer_t *buf);
void emit_cbnz(code_buffer_t *buf, uint8_t src, int32_t imm19);
void emit_cbz(code_buffer_t *buf, uint8_t src, int32_t imm19);
void emit_tbz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14);
void emit_tbnz(code_buffer_t *buf, uint8_t src, uint8_t bit, int32_t imm14);

/* ============================================================================
 * Load/Store Instructions
 * ============================================================================ */

void emit_ldr_imm(code_buffer_t *buf, uint8_t dst, uint8_t base, uint16_t imm);
void emit_str_imm(code_buffer_t *buf, uint8_t src, uint8_t base, uint16_t imm);
void emit_ldr_reg(code_buffer_t *buf, uint8_t dst, uint8_t base, uint8_t offset);
void emit_str_reg(code_buffer_t *buf, uint8_t src, uint8_t base, uint8_t offset);
void emit_ldp(code_buffer_t *buf, uint8_t dst1, uint8_t dst2, uint8_t base);
void emit_stp(code_buffer_t *buf, uint8_t src1, uint8_t src2, uint8_t base);

/* ============================================================================
 * Address Calculation
 * ============================================================================ */

void emit_adr(code_buffer_t *buf, uint8_t dst, int32_t imm21);

/* ============================================================================
 * System Instructions
 * ============================================================================ */

void emit_nop(code_buffer_t *buf);
void emit_hlt(code_buffer_t *buf);

/* ============================================================================
 * Additional ARM64 Emit Helpers (P0-P4)
 * ============================================================================ */

/* Move with extend */
void emit_mov_extend(code_buffer_t *buf, uint8_t dst, uint8_t src, int is_signed, int is_16bit);
void emit_movz_ext(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_movs_ext(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* Increment/Decrement */
void emit_inc_reg(code_buffer_t *buf, uint8_t dst);
void emit_dec_reg(code_buffer_t *buf, uint8_t dst);

/* Negate/Not */
void emit_neg_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_not_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);

/* Stack operations */
void emit_push_reg(code_buffer_t *buf, uint8_t reg);
void emit_pop_reg(code_buffer_t *buf, uint8_t reg);

/* Shift/Rotate */
void emit_shl_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shr_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_sar_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_rol_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_ror_reg_imm(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);

/* Conditional operations */
void emit_csel_reg_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t src1, uint8_t src2, uint8_t cond);
void emit_setcc_reg_cond(code_buffer_t *buf, uint8_t dst, uint8_t cond);

/* Bit manipulation */
void emit_bsf_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_bsr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_popcnt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src);
void emit_bt_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_bts_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btr_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);
void emit_btc_reg(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t bit);

/* String operations */
void emit_movs(code_buffer_t *buf, int is_64bit);
void emit_stos(code_buffer_t *buf, int size);
void emit_lods(code_buffer_t *buf, int size);
void emit_cmps(code_buffer_t *buf, int size);
void emit_scas(code_buffer_t *buf, int size);

/* Special instructions */
void emit_shld(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_shrd(code_buffer_t *buf, uint8_t dst, uint8_t src, uint8_t shift);
void emit_cqo(code_buffer_t *buf);
void emit_cli(code_buffer_t *buf);
void emit_sti(code_buffer_t *buf);
void emit_cpuid(code_buffer_t *buf);
void emit_rdtsc(code_buffer_t *buf);

/* ============================================================================
 * SIMD/NEON Instructions
 * ============================================================================ */

/* SIMD MOV */
void emit_simd_mov(code_buffer_t *buf, uint8_t vd, uint8_t vn);
void emit_simd_mov_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_movhl(code_buffer_t *buf, uint8_t vd, uint8_t vn);
void emit_simd_movh(code_buffer_t *buf, uint8_t vd, uint8_t vn);
void emit_simd_movl(code_buffer_t *buf, uint8_t vd, uint8_t vn);
void emit_simd_movd(code_buffer_t *buf, uint8_t vd, uint8_t vn, int aligned);
void emit_simd_movq(code_buffer_t *buf, uint8_t vd, uint8_t vn);

/* SIMD Arithmetic */
void emit_simd_fadd(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fadd_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fsub(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fsub_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fmul(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fmul_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fdiv(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);
void emit_simd_fdiv_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int is_double);

/* SIMD Logical */
void emit_simd_and(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm);
void emit_simd_bic(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm);
void emit_simd_orr(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm);
void emit_simd_eor(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm);

/* SIMD Comparison */
void emit_simd_fcmp(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_fcmp_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_ucomi(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_comi(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);

/* SIMD Conversion */
void emit_simd_fcvt(code_buffer_t *buf, uint8_t vd, uint8_t vn, int to_double);
void emit_simd_fcvt_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, int to_double);
void emit_simd_scvtf(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_fcvtzs(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_fcvtxzs(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);

/* SIMD Square Root */
void emit_simd_fsqrt(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);
void emit_simd_fsqrt_scalar(code_buffer_t *buf, uint8_t vd, uint8_t vn, int is_double);

/* SIMD Shuffle/Unpack */
void emit_simd_shuf(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t imm);
void emit_simd_shuf_lw(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t imm);
void emit_simd_shuf_hw(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t imm);
void emit_simd_unpckl(code_buffer_t *buf, uint8_t vd, uint8_t vn, int elem_size);
void emit_simd_unpckh(code_buffer_t *buf, uint8_t vd, uint8_t vn, int elem_size);

/* SIMD Integer */
void emit_simd_add(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int elem_size);
void emit_simd_sub(code_buffer_t *buf, uint8_t vd, uint8_t vn, uint8_t vm, int elem_size);

#endif /* ROSETTA_ARM64_EMIT_H */
