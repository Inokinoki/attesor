/* ============================================================================
 * Rosetta Translator - ARM64 Instruction Predicates Header
 * ============================================================================
 *
 * ARM64 instruction decoding helpers and predicates.
 * ============================================================================ */

#ifndef ROSETTA_ARM64_INSNS_H
#define ROSETTA_ARM64_INSNS_H

#include <stdint.h>

/* ============================================================================
 * FP/SIMD Instruction Detection (op30 == 3)
 * ============================================================================ */

static inline int arm64_is_fp_insn(uint32_t encoding)
{
    return ((encoding >> 28) & 0x7) == 0x7;  /* op30 == 3 */
}

static inline uint8_t arm64_get_fp_opcode(uint32_t encoding)
{
    return (encoding >> 28) & 0xF;
}

/* ============================================================================
 * FP/SIMD Register Accessors
 * ============================================================================ */

static inline uint8_t arm64_get_vd(uint32_t encoding)
{
    return ((encoding >> 5) & 0x1F);
}

static inline uint8_t arm64_get_vn(uint32_t encoding)
{
    return ((encoding >> 10) & 0x1F);
}

static inline uint8_t arm64_get_vm(uint32_t encoding)
{
    return ((encoding >> 20) & 0x1F);
}

static inline uint8_t arm64_get_vd_q(uint32_t encoding)
{
    uint8_t vd = (encoding >> 5) & 0x1F;
    uint8_t q = (encoding >> 30) & 0x1;
    return vd | (q << 5);
}

static inline uint8_t arm64_get_fpmem16(uint32_t encoding)
{
    return ((encoding >> 13) & 0xF0) | ((encoding >> 5) & 0x0F);
}

static inline uint8_t arm64_get_fp_cond(uint32_t encoding)
{
    return (encoding >> 12) & 0xF;
}

/* ============================================================================
 * Floating Point Instructions
 * ============================================================================ */

static inline int arm64_is_fmov_imm(uint32_t encoding)
{
    return (encoding & 0xFF800400) == 0x1E800000;
}

static inline int arm64_is_fmov_reg(uint32_t encoding)
{
    return (encoding & 0xFFFF0400) == 0x1E800000;
}

static inline int arm64_is_fadd(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E200000;
}

static inline int arm64_is_fsub(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E600000;
}

static inline int arm64_is_fmul(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E300000;
}

static inline int arm64_is_fdiv(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

static inline int arm64_is_fsqrt(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0x1E900000;
}

static inline int arm64_is_fcmp(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

static inline int arm64_is_fcsel(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E800C00;
}

static inline int arm64_is_fccmp(uint32_t encoding)
{
    return (encoding & 0xFFE0FC00) == 0x1E800400;
}

static inline int arm64_is_fabs(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0x1E200800;
}

static inline int arm64_is_fneg(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0x1E201800;
}

static inline int arm64_is_fcvtds(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0x1E60C000;
}

static inline int arm64_is_fcvtsd(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0x1E20C000;
}

/* ============================================================================
 * NEON Vector Instructions
 * ============================================================================ */

static inline int arm64_is_add_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_sub_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_and_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_orr_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_eor_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E800000;
}

static inline int arm64_is_bic_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_mul_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_sshr_vec(uint32_t encoding)
{
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_ushr_vec(uint32_t encoding)
{
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_shl_vec(uint32_t encoding)
{
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_cmgt_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_cmeq_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_cmge_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

static inline int arm64_is_cmhs_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

static inline int arm64_is_cmle_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E100000;
}

static inline int arm64_is_cmlt_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_umin_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_umax_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

static inline int arm64_is_smin_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_smax_vec(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

/* ============================================================================
 * Vector Size/Shift Accessors
 * ============================================================================ */

static inline uint8_t arm64_get_vec_size(uint32_t encoding)
{
    return ((encoding >> 22) & 0x3);
}

static inline uint8_t arm64_get_q_bit(uint32_t encoding)
{
    return ((encoding >> 30) & 0x1);
}

static inline int8_t arm64_get_shift_imm(uint32_t encoding)
{
    int8_t imm = ((encoding >> 16) & 0x3F);
    if (imm & 0x20) imm |= 0xC0;  /* Sign extend */
    return imm;
}

/* ============================================================================
 * NEON Load/Store Instructions
 * ============================================================================ */

static inline int arm64_is_ld1(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x0C000000;
}

static inline int arm64_is_st1(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x08000000;
}

static inline int arm64_is_ld1_multiple(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x0C200000;
}

static inline int arm64_is_st1_multiple(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x08200000;
}

static inline int arm64_is_ld2(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x0C400000;
}

static inline int arm64_is_st2(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x08400000;
}

static inline int arm64_is_ld3(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x0C600000;
}

static inline int arm64_is_st3(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x08600000;
}

static inline int arm64_is_ld4(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x0C700000;
}

static inline int arm64_is_st4(uint32_t encoding)
{
    return (encoding & 0xFF20FC00) == 0x08700000;
}

static inline int arm64_is_tbl(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0C000000;
}

static inline int arm64_is_tbx(uint32_t encoding)
{
    return (encoding & 0xFE20FC00) == 0x0C200000;
}

static inline uint8_t arm64_get_tbl_reg_count(uint32_t encoding)
{
    return ((encoding >> 10) & 0x03) + 1;  /* 1-4 registers */
}

static inline uint8_t arm64_get_neon_reg_count(uint32_t encoding)
{
    return ((encoding >> 10) & 0x03) + 1;  /* 1-4 registers */
}

static inline uint8_t arm64_get_neon_size(uint32_t encoding)
{
    return ((encoding >> 22) & 0x03);
}

static inline int8_t arm64_get_neon_index(uint32_t encoding)
{
    int8_t index = ((encoding >> 10) & 0x0F);
    if (index & 0x08) index |= 0xF0;  /* Sign extend */
    return index;
}

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

static inline uint8_t map_vreg_to_xmm(uint8_t vreg)
{
    return vreg & 0xF;  /* V0-V15 -> XMM0-XMM15 */
}

static inline uint8_t x86_map_xmm(uint8_t vreg)
{
    return vreg & 0x0F;  /* Wrap around */
}

#endif /* ROSETTA_ARM64_INSNS_H */
