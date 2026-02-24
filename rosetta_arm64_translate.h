#ifndef ROSETTA_ARM64_DECODE_H
#define ROSETTA_ARM64_DECODE_H

/* ============================================================================
 * Rosetta Binary Translator - ARM64 Instruction Decoding
 * ============================================================================
 *
 * This header contains inline functions for decoding ARM64 instructions.
 * Each function extracts specific fields or checks for specific instruction types.
 *
 * ============================================================================ */

#include "rosetta_types.h"

/* ============================================================================
 * Instruction Field Extraction
 * ============================================================================ */

/**
 * Extract opcode class (bits 28-29, "op" field at bits 30-31)
 */
static inline u32 arm64_get_opclass(u32 encoding)
{
    return (encoding >> 28) & 0x3;
}

/**
 * Extract destination register (Rd, bits 0-4)
 */
static inline u8 arm64_get_rd(u32 encoding)
{
    return encoding & 0x1F;
}

/**
 * Extract first operand register (Rn, bits 5-9)
 */
static inline u8 arm64_get_rn(u32 encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract second operand register (Rm, bits 16-20)
 */
static inline u8 arm64_get_rm(u32 encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * Extract 12-bit immediate (imm12, bits 10-21)
 */
static inline u16 arm64_get_imm12(u32 encoding)
{
    return (encoding >> 10) & 0xFFF;
}

/**
 * Extract 26-bit branch immediate (imm26, bits 0-25)
 */
static inline s32 arm64_get_imm26(u32 encoding)
{
    return (s32)((encoding & 0x03FFFFFF) << 6) >> 4;  /* Sign-extend */
}

/**
 * Extract 16-bit immediate for MOVZ/MOVK/MOVN (bits 5-20)
 */
static inline u16 arm64_get_imm16(u32 encoding)
{
    return (encoding >> 5) & 0xFFFF;
}

/**
 * Extract shift amount for MOVZ/MOVK/MOVN (hw field, bits 21-22)
 */
static inline u8 arm64_get_hw(u32 encoding)
{
    return (encoding >> 21) & 0x3;
}

/**
 * Extract 19-bit branch immediate for B.cond (bits 5-23)
 */
static inline s32 arm64_get_imm19(u32 encoding)
{
    return (s32)((encoding & 0x00FFFFE0) << 3) >> 5;  /* Sign-extend */
}

/**
 * Extract 14-bit immediate for CBZ/CBNZ (bits 5-18)
 */
static inline s32 arm64_get_imm14(u32 encoding)
{
    return (s32)((encoding & 0x00FFFFE0) << 3) >> 5;
}

/**
 * Extract 7-bit immediate for TBZ/TBNZ (bits 6-12, 16-19)
 */
static inline s32 arm64_get_imm7(u32 encoding)
{
    u32 imm = ((encoding >> 31) & 0x10) | ((encoding >> 16) & 0xF);
    return (s32)(imm << 26) >> 26;  /* Sign-extend */
}

/**
 * Extract test bit for TBZ/TBNZ (bits 26-27, 31-32 encoded)
 */
static inline u8 arm64_get_test_bit(u32 encoding)
{
    u8 bit5 = (encoding >> 31) & 0x1;
    u8 bit4_0 = (encoding >> 16) & 0x1F;
    return (bit5 << 5) | bit4_0;
}

/**
 * Extract condition code (cond, bits 0-3)
 */
static inline u8 arm64_get_cond(u32 encoding)
{
    return encoding & 0xF;
}

/**
 * Extract SIMD/FP opcode (bits 10-14)
 */
static inline u8 arm64_get_fp_opcode(u32 encoding)
{
    return (encoding >> 10) & 0x1F;
}

/**
 * Extract SIMD/FP destination register (Vd, bits 0-4)
 */
static inline u8 arm64_get_vd(u32 encoding)
{
    return encoding & 0x1F;
}

/**
 * Extract SIMD/FP first operand register (Vn, bits 5-9)
 */
static inline u8 arm64_get_vn(u32 encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract SIMD/FP second operand register (Vm, bits 16-20)
 */
static inline u8 arm64_get_vm(u32 encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * Extract SIMD/FP register with Q bit (Q:Vd)
 */
static inline u8 arm64_get_vd_q(u32 encoding)
{
    u8 q = (encoding >> 30) & 0x1;
    u8 vd = encoding & 0x1F;
    return (q << 5) | vd;
}

/**
 * Extract size field for SIMD instructions (bits 22-23)
 */
static inline u8 arm64_get_vec_size(u32 encoding)
{
    return (encoding >> 22) & 0x3;
}

/**
 * Extract Q bit (bit 30) - 128-bit vector flag
 */
static inline u8 arm64_get_q_bit(u32 encoding)
{
    return (encoding >> 30) & 0x1;
}

/**
 * Extract shift immediate for vector shifts
 */
static inline u8 arm64_get_shift_imm(u32 encoding)
{
    u8 immh = (encoding >> 16) & 0x7;
    u8 immb = (encoding >> 11) & 0x1F;
    return (immh << 5) | immb;
}

/**
 * Extract FP immediate value (bits 13-18)
 */
static inline u8 arm64_get_fpmem16(u32 encoding)
{
    return (encoding >> 13) & 0x3F;
}

/**
 * Get register count for LD1/LD2/LD3/LD4/ST1/ST2/ST3/ST4
 */
static inline u8 arm64_get_neon_reg_count(u32 encoding)
{
    u8 size = (encoding >> 30) & 0x3;
    return (size & 0x1) + 1;  /* Simplified */
}

/**
 * Get size field for NEON load/store
 */
static inline u8 arm64_get_neon_size(u32 encoding)
{
    return (encoding >> 30) & 0x3;
}

/**
 * Get post-increment index for NEON load/store
 */
inline s8 arm64_get_neon_index(u32 encoding)
{
    if (encoding & (1 << 23)) {
        return (s8)((encoding >> 16) & 0xF);
    }
    return 0;
}

/**
 * Get table register count for TBL/TBX
 */
static inline u8 arm64_get_tbl_reg_count(u32 encoding)
{
    return ((encoding >> 13) & 0x3) + 1;
}

/* ============================================================================
 * Instruction Type Checks - Data Processing (op30 == 0)
 * ============================================================================ */

/**
 * Check if ADD (register) instruction
 * Encoding: 0000 1011 0000 0000 0000 0000 0000 0000 = 0x0B000000
 */
static inline int arm64_is_add(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x0B000000;
}

/**
 * Check if SUB (register) instruction
 * Encoding: 0100 1011 0000 0000 0000 0000 0000 0000 = 0x4B000000
 */
static inline int arm64_is_sub(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x4B000000;
}

/**
 * Check if ADD (immediate) instruction
 * Encoding: 0001 0001 0000 0000 0000 0000 0000 0000 = 0x11000000
 */
static inline int arm64_is_add_imm(u32 encoding)
{
    return (encoding & 0xFF800000) == 0x11000000;
}

/**
 * Check if SUB (immediate) instruction
 * Encoding: 0101 0001 0000 0000 0000 0000 0000 0000 = 0x51000000
 */
static inline int arm64_is_sub_imm(u32 encoding)
{
    return (encoding & 0xFF800000) == 0x51000000;
}

/**
 * Check if AND (immediate) instruction
 */
static inline int arm64_is_and_imm(u32 encoding)
{
    return (encoding & 0xFF800000) == 0x12000000;
}

/**
 * Check if CMP (register) instruction - SUBS with Zd = 31 (SP)
 */
static inline int arm64_is_cmp(u32 encoding)
{
    return (encoding & 0xFFE0FFFF) == 0xEB00001F;
}

/**
 * Check if CMN (register) instruction - ADDS with Zd = 31 (SP)
 */
static inline int arm64_is_cmn(u32 encoding)
{
    return (encoding & 0xFFE0FFFF) == 0x2B00001F;
}

/**
 * Check if TST (register) instruction - ANDS with Zd = 31 (SP)
 */
static inline int arm64_is_tst(u32 encoding)
{
    return (encoding & 0xFFE0E01F) == 0xEA00001F;
}

/**
 * Check if AND (register) instruction
 */
static inline int arm64_is_and(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x0A000000;
}

/**
 * Check if ORR (register) instruction
 */
static inline int arm64_is_orr(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x2A000000;
}

/**
 * Check if EOR (register) instruction
 */
static inline int arm64_is_eor(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A000000;
}

/**
 * Check if MVN (register) instruction
 */
static inline int arm64_is_mvn(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A200000;
}

/**
 * Check if MUL (register) instruction
 */
static inline int arm64_is_mul(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x1B000000;
}

/**
 * Check if SDIV (signed divide) instruction
 */
static inline int arm64_is_div(u32 encoding)
{
    return (encoding & 0xFFE0E000) == 0x9AC00000;
}

/* ============================================================================
 * Instruction Type Checks - Branches (op30 == 1)
 * ============================================================================ */

/**
 * Check if B (unconditional branch) instruction
 */
static inline int arm64_is_b(u32 encoding)
{
    return (encoding & 0xFC000000) == 0x14000000;
}

/**
 * Check if BL (branch with link) instruction
 */
static inline int arm64_is_bl(u32 encoding)
{
    return (encoding & 0xFC000000) == 0x94000000;
}

/**
 * Check if BR (branch to register) instruction
 */
static inline int arm64_is_br(u32 encoding)
{
    return (encoding & 0xFFFFFC1F) == 0xD61F0000;
}

/**
 * Check if RET (return) instruction
 */
static inline int arm64_is_ret(u32 encoding)
{
    return (encoding & 0xFFFFFC1F) == 0xD65F0000;
}

/**
 * Check if B.cond (conditional branch) instruction
 */
static inline int arm64_is_bcond(u32 encoding)
{
    return (encoding & 0xFF000010) == 0x54000000;
}

/**
 * Check if CBZ (compare and branch zero) instruction
 */
static inline int arm64_is_cbz(u32 encoding)
{
    return (encoding & 0x7F000000) == 0x34000000;
}

/**
 * Check if CBNZ (compare and branch non-zero) instruction
 */
static inline int arm64_is_cbnz(u32 encoding)
{
    return (encoding & 0x7F000000) == 0x35000000;
}

/**
 * Check if TBZ (test bit and branch zero) instruction
 */
static inline int arm64_is_tbz(u32 encoding)
{
    return (encoding & 0x7F000000) == 0x36000000;
}

/**
 * Check if TBNZ (test bit and branch non-zero) instruction
 */
static inline int arm64_is_tbnz(u32 encoding)
{
    return (encoding & 0x7F000000) == 0x37000000;
}

/* ============================================================================
 * Instruction Type Checks - Load/Store (op30 == 2)
 * ============================================================================ */

/**
 * Check if LDR (register, immediate) instruction
 */
static inline int arm64_is_ldr(u32 encoding)
{
    return (encoding & 0x3B000000) == 0x39000000;
}

/**
 * Check if STR (register, immediate) instruction
 */
static inline int arm64_is_str(u32 encoding)
{
    return (encoding & 0x3B000000) == 0x38000000;
}

/**
 * Check if LDP (load pair) instruction
 */
static inline int arm64_is_ldp(u32 encoding)
{
    return (encoding & 0x3B800000) == 0x29800000;
}

/**
 * Check if STP (store pair) instruction
 */
static inline int arm64_is_stp(u32 encoding)
{
    return (encoding & 0x3B800000) == 0x29000000;
}

/**
 * Check if LDRB (load register byte) instruction
 */
static inline int arm64_is_ldrb(u32 encoding)
{
    return (encoding & 0x3F000000) == 0x39000000;
}

/**
 * Check if STRB (store register byte) instruction
 */
static inline int arm64_is_strb(u32 encoding)
{
    return (encoding & 0x3F000000) == 0x38000000;
}

/**
 * Check if LDRH (load register halfword) instruction
 */
static inline int arm64_is_ldrh(u32 encoding)
{
    return (encoding & 0x3F000000) == 0x79000000;
}

/**
 * Check if STRH (store register halfword) instruction
 */
static inline int arm64_is_strh(u32 encoding)
{
    return (encoding & 0x3F000000) == 0x78000000;
}

/**
 * Check if LDRSB (load register signed byte) instruction
 */
static inline int arm64_is_ldrsb(u32 encoding)
{
    return (encoding & 0x3FC00000) == 0x39C00000;
}

/**
 * Check if LDRSH (load register signed halfword) instruction
 */
static inline int arm64_is_ldrsh(u32 encoding)
{
    return (encoding & 0x3FC00000) == 0x79C00000;
}

/**
 * Check if LDRSW (load register signed word) instruction
 */
static inline int arm64_is_ldrsw(u32 encoding)
{
    return (encoding & 0x3FC00000) == 0xB9C00000;
}

/**
 * Check if LDUR (load register unscaled) instruction
 */
static inline int arm64_is_ldur(u32 encoding)
{
    return (encoding & 0x3B200000) == 0x38400000;
}

/**
 * Check if STUR (store register unscaled) instruction
 */
static inline int arm64_is_stur(u32 encoding)
{
    return (encoding & 0x3B200000) == 0x38000000;
}

/* ============================================================================
 * Instruction Type Checks - System Instructions
 * ============================================================================ */

/**
 * Check if SVC (supervisor call) instruction
 */
static inline int arm64_is_svc(u32 encoding)
{
    return (encoding & 0xFFE00000) == 0xD4000001;
}

/**
 * Check if BRK (breakpoint) instruction
 */
static inline int arm64_is_brk(u32 encoding)
{
    return (encoding & 0xFFE0001F) == 0xD4200000;
}

/**
 * Check if HLT (halt) instruction
 */
static inline int arm64_is_hlt(u32 encoding)
{
    return (encoding & 0xFFE0001F) == 0xD4000000;
}

/**
 * Check if instruction is a block terminator (branch, return, etc.)
 */
static inline int arm64_is_block_terminator(u32 encoding)
{
    return arm64_is_b(encoding) ||
           arm64_is_bl(encoding) ||
           arm64_is_br(encoding) ||
           arm64_is_ret(encoding) ||
           arm64_is_bcond(encoding) ||
           arm64_is_cbz(encoding) ||
           arm64_is_cbnz(encoding) ||
           arm64_is_tbz(encoding) ||
           arm64_is_tbnz(encoding) ||
           arm64_is_svc(encoding) ||
           arm64_is_brk(encoding);
}

/* ============================================================================
 * Instruction Type Checks - Floating Point (op30 == 3)
 * ============================================================================ */

/**
 * Check if instruction is FP/SIMD (op30 == 3)
 */
static inline int arm64_is_fp_insn(u32 encoding)
{
    return ((encoding >> 28) & 0x3) == 0x3;
}

/**
 * Check if FMov (immediate) instruction
 */
static inline int arm64_is_fmov_imm(u32 encoding)
{
    return (encoding & 0x3BF00000) == 0x1E200000;
}

/**
 * Check if FMov (register) instruction
 */
static inline int arm64_is_fmov_reg(u32 encoding)
{
    return (encoding & 0x3FF00000) == 0x1E204000;
}

/**
 * Check if FAdd (FP add) instruction
 */
static inline int arm64_is_fadd(u32 encoding)
{
    return (encoding & 0x3FF00000) == 0x1E200800;
}

/**
 * Check if FSub (FP subtract) instruction
 */
static inline int arm64_is_fsub(u32 encoding)
{
    return (encoding & 0x3FF00000) == 0x1E200C00;
}

/**
 * Check if FMul (FP multiply) instruction
 */
static inline int arm64_is_fmul(u32 encoding)
{
    return (encoding & 0x3FF00000) == 0x1E200400;
}

/**
 * Check if FDiv (FP divide) instruction
 */
static inline int arm64_is_fdiv(u32 encoding)
{
    return (encoding & 0x3FF00000) == 0x1E200C40;
}

/**
 * Check if FSqrt (FP square root) instruction
 */
static inline int arm64_is_fsqrt(u32 encoding)
{
    return (encoding & 0x3FFFC000) == 0x1E21C000;
}

/**
 * Check if FCmp (FP compare) instruction
 */
static inline int arm64_is_fcmp(u32 encoding)
{
    return (encoding & 0x3FF00040) == 0x1E202000;
}

/**
 * Check if FCSEL (FP conditional select) instruction
 */
static inline int arm64_is_fcsel(u32 encoding)
{
    return (encoding & 0x3FE00C00) == 0x1E200C00;
}

/**
 * Check if FCCMP (FP conditional compare) instruction
 */
static inline int arm64_is_fccmp(u32 encoding)
{
    return (encoding & 0x3FE00040) == 0x1E200040;
}

/**
 * Check if FABS (FP absolute value) instruction
 */
static inline int arm64_is_fabs(u32 encoding)
{
    return (encoding & 0x3FFFC000) == 0x1E20C000;
}

/**
 * Check if FNEG (FP negate) instruction
 */
static inline int arm64_is_fneg(u32 encoding)
{
    return (encoding & 0x3FFFC000) == 0x1E214000;
}

/**
 * Check if FCVTDS (convert double to single) instruction
 */
static inline int arm64_is_fcvtds(u32 encoding)
{
    return (encoding & 0x3FFFC000) == 0x1E224000;
}

/**
 * Check if FCVTSD (convert single to double) instruction
 */
static inline int arm64_is_fcvtsd(u32 encoding)
{
    return (encoding & 0x3FFFC000) == 0x1E624000;
}

/**
 * Extract FP condition code for FCSEL/FCCMP
 */
static inline u8 arm64_get_fp_cond(u32 encoding)
{
    return (encoding >> 4) & 0xF;
}

/* ============================================================================
 * Instruction Type Checks - NEON/Vector Instructions
 * ============================================================================ */

/**
 * Check if ADD (vector) instruction
 */
static inline int arm64_is_add_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200800;
}

/**
 * Check if SUB (vector) instruction
 */
static inline int arm64_is_sub_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E200800;
}

/**
 * Check if AND (vector) instruction
 */
static inline int arm64_is_and_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201800;
}

/**
 * Check if ORR (vector) instruction
 */
static inline int arm64_is_orr_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201C00;
}

/**
 * Check if EOR (vector) instruction
 */
static inline int arm64_is_eor_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201800;
}

/**
 * Check if BIC (vector bit clear) instruction
 */
static inline int arm64_is_bic_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201C00;
}

/**
 * Check if MUL (vector) instruction
 */
static inline int arm64_is_mul_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E200C00;
}

/**
 * Check if SSHR (signed shift right) instruction
 */
static inline int arm64_is_sshr_vec(u32 encoding)
{
    return (encoding & 0x3F20FC00) == 0x0E200400;
}

/**
 * Check if USHR (unsigned shift right) instruction
 */
static inline int arm64_is_ushr_vec(u32 encoding)
{
    return (encoding & 0x3F20FC00) == 0x0E201000;
}

/**
 * Check if SHL (shift left) instruction
 */
static inline int arm64_is_shl_vec(u32 encoding)
{
    return (encoding & 0x3F20FC00) == 0x0E200800;
}

/**
 * Check if CMGT (compare greater than) instruction
 */
static inline int arm64_is_cmgt_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201000;
}

/**
 * Check if CMEQ (compare equal) instruction
 */
static inline int arm64_is_cmeq_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201000;
}

/**
 * Check if CMGE (compare greater than or equal) instruction
 */
static inline int arm64_is_cmge_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201400;
}

/**
 * Check if CMHS (compare unsigned higher or same) instruction
 */
static inline int arm64_is_cmhs_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201800;
}

/**
 * Check if CMLE (compare less than or equal) instruction
 */
static inline int arm64_is_cmle_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201400;
}

/**
 * Check if CMLT (compare less than) instruction
 */
static inline int arm64_is_cmlt_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201800;
}

/**
 * Check if UMIN (unsigned minimum) instruction
 */
static inline int arm64_is_umin_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201C00;
}

/**
 * Check if UMAX (unsigned maximum) instruction
 */
static inline int arm64_is_umax_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x2E201400;
}

/**
 * Check if SMIN (signed minimum) instruction
 */
static inline int arm64_is_smin_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201C00;
}

/**
 * Check if SMAX (signed maximum) instruction
 */
static inline int arm64_is_smax_vec(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0E201400;
}

/**
 * Check if FRECPE (FP reciprocal estimate) instruction
 */
static inline int arm64_is_frecpe(u32 encoding)
{
    return (encoding & 0x3FFFFC00) == 0x2E20FC00;
}

/**
 * Check if FRSQRTE (FP reciprocal square root estimate) instruction
 */
static inline int arm64_is_frsqrte(u32 encoding)
{
    return (encoding & 0x3FFFFC00) == 0x2E20F800;
}

/* ============================================================================
 * NEON Load/Store Instructions
 * ============================================================================ */

/**
 * Check if LD1 (load single structure) instruction
 */
static inline int arm64_is_ld1(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C000000;
}

/**
 * Check if ST1 (store single structure) instruction
 */
static inline int arm64_is_st1(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C800000;
}

/**
 * Check if LD2 (load pair of structures) instruction
 */
static inline int arm64_is_ld2(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C000000 && (encoding & (1 << 15));
}

/**
 * Check if ST2 (store pair of structures) instruction
 */
static inline int arm64_is_st2(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C800000 && (encoding & (1 << 15));
}

/**
 * Check if LD3 (load three structures) instruction
 */
static inline int arm64_is_ld3(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C000000 && ((encoding >> 15) & 0x3) == 0x2;
}

/**
 * Check if ST3 (store three structures) instruction
 */
static inline int arm64_is_st3(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C800000 && ((encoding >> 15) & 0x3) == 0x2;
}

/**
 * Check if LD4 (load four structures) instruction
 */
static inline int arm64_is_ld4(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C000000 && ((encoding >> 15) & 0x3) == 0x3;
}

/**
 * Check if ST4 (store four structures) instruction
 */
static inline int arm64_is_st4(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C800000 && ((encoding >> 15) & 0x3) == 0x3;
}

/**
 * Check if TBL (table lookup) instruction
 */
static inline int arm64_is_tbl(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0C000000;
}

/**
 * Check if TBX (table lookup extension) instruction
 */
static inline int arm64_is_tbx(u32 encoding)
{
    return (encoding & 0xFE20FC00) == 0x0C200000;
}

/**
 * Check if DUP (duplicate) instruction
 */
static inline int arm64_is_dup(u32 encoding)
{
    return (encoding & 0x3FE00000) == 0x0E000000;
}

/**
 * Check if EXT (vector extract) instruction
 */
static inline int arm64_is_ext(u32 encoding)
{
    return (encoding & 0x3F200000) == 0x0C400000;
}

#endif /* ROSETTA_ARM64_DECODE_H */
