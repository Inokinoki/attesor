#ifndef ROSETTA_ARM64_DECODE_H
#define ROSETTA_ARM64_DECODE_H

/* ============================================================================
 * Rosetta Binary Translator - ARM64 Instruction Decoding
 * ============================================================================ */

#include "rosetta_types.h"

/* ============================================================================
 * ARM64 Instruction Encoding Fields
 * ============================================================================ */

/**
 * Extract opcode class (bits 29-30)
 * @param encoding ARM64 instruction encoding
 * @return Opcode class (0-3)
 */
static inline u32 arm64_get_opclass(u32 encoding) {
    return (encoding >> 29) & 0x03;
}

/**
 * Extract destination register Rd (bits 0-4)
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-30)
 */
static inline u8 arm64_get_rd(u32 encoding) {
    return encoding & 0x1F;
}

/**
 * Extract first operand register Rn (bits 5-9)
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-30)
 */
static inline u8 arm64_get_rn(u32 encoding) {
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract second operand register Rm (bits 16-20)
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-30)
 */
static inline u8 arm64_get_rm(u32 encoding) {
    return (encoding >> 16) & 0x1F;
}

/**
 * Extract 12-bit immediate (bits 10-21)
 * @param encoding ARM64 instruction encoding
 * @return 12-bit immediate value
 */
static inline u16 arm64_get_imm12(u32 encoding) {
    return (encoding >> 10) & 0xFFF;
}

/**
 * Extract 16-bit immediate for MOVZ/MOVK/MOVN (bits 5-20)
 * @param encoding ARM64 instruction encoding
 * @return 16-bit immediate value
 */
static inline u16 arm64_get_imm16(u32 encoding) {
    return ((encoding >> 5) & 0xFFFF);
}

/**
 * Extract shift amount for MOVZ/MOVK (bits 21-22)
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (0, 16, 32, or 48)
 */
static inline u8 arm64_get_hw(u32 encoding) {
    return ((encoding >> 21) & 0x03);
}

/**
 * Extract 26-bit branch immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 26-bit immediate
 */
static inline s32 arm64_get_imm26(u32 encoding) {
    s32 imm = encoding & 0x03FFFFFF;
    return SIGN_EXT(imm, 26);
}

/**
 * Extract 19-bit branch immediate (conditional branches)
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 19-bit immediate
 */
static inline s32 arm64_get_imm19(u32 encoding) {
    s32 imm = ((encoding >> 5) & 0x7FFFF);
    return SIGN_EXT(imm, 19);
}

/**
 * Extract 14-bit immediate (CBZ/CBNZ)
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 14-bit immediate
 */
static inline s32 arm64_get_imm14(u32 encoding) {
    s32 imm = ((encoding >> 5) & 0x3FFF);
    return SIGN_EXT(imm, 14);
}

/**
 * Extract test bit (TBZ/TBNZ)
 * @param encoding ARM64 instruction encoding
 * @return Bit number to test
 */
static inline u8 arm64_get_test_bit(u32 encoding) {
    u8 bit5 = (encoding >> 31) & 0x01;
    u8 bit4_0 = (encoding >> 19) & 0x1F;
    return (bit5 << 5) | bit4_0;
}

/**
 * Extract condition code (bits 0-3)
 * @param encoding ARM64 instruction encoding
 * @return Condition code (0-15)
 */
static inline u8 arm64_get_cond(u32 encoding) {
    return encoding & 0x0F;
}

/**
 * Get vector size field (bits 22-23)
 * @param encoding ARM64 instruction encoding
 * @return Size field (0-3)
 */
static inline u8 arm64_get_vec_size(u32 encoding) {
    return (encoding >> 22) & 0x03;
}

/**
 * Get Q bit (128-bit flag, bit 30)
 * @param encoding ARM64 instruction encoding
 * @return Q bit value
 */
static inline u8 arm64_get_q_bit(u32 encoding) {
    return (encoding >> 30) & 0x01;
}

/**
 * Extract SIMD/FP destination register Vd (bits 0-4)
 * @param encoding ARM64 instruction encoding
 * @return Vector register number (0-31)
 */
static inline u8 arm64_get_vd(u32 encoding) {
    return encoding & 0x1F;
}

/**
 * Extract SIMD/FP first operand register Vn (bits 5-9)
 * @param encoding ARM64 instruction encoding
 * @return Vector register number (0-31)
 */
static inline u8 arm64_get_vn(u32 encoding) {
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract SIMD/FP second operand register Vm (bits 16-20)
 * @param encoding ARM64 instruction encoding
 * @return Vector register number (0-31)
 */
static inline u8 arm64_get_vm(u32 encoding) {
    return (encoding >> 16) & 0x1F;
}

/**
 * Get shift immediate from SIMD shift instruction
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (signed)
 */
static inline s8 arm64_get_shift_imm(u32 encoding) {
    s8 imm = ((encoding >> 16) & 0x3F);
    if (imm & 0x20) imm |= 0xC0;  /* Sign extend from 6 bits */
    return imm;
}

/* ============================================================================
 * ALU Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_add(u32 encoding) {
    return (encoding & 0xFF000000) == 0x0B000000;
}

static inline int arm64_is_sub(u32 encoding) {
    return (encoding & 0xFF000000) == 0x4B000000;
}

static inline int arm64_is_and(u32 encoding) {
    return (encoding & 0xFF000000) == 0x0A000000;
}

static inline int arm64_is_orr(u32 encoding) {
    return (encoding & 0xFF000000) == 0x2A000000;
}

static inline int arm64_is_eor(u32 encoding) {
    return (encoding & 0xFF000000) == 0x4A000000;
}

static inline int arm64_is_mvn(u32 encoding) {
    return (encoding & 0xFFE00000) == 0x2A200000;
}

static inline int arm64_is_mul(u32 encoding) {
    return (encoding & 0xFFE0FC00) == 0x1B000000;
}

static inline int arm64_is_div(u32 encoding) {
    return (encoding & 0xFFE0FC00) == 0x1AC00800;
}

/* ============================================================================
 * Compare Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_cmp(u32 encoding) {
    return (encoding & 0xFF000000) == 0xEB000000;
}

static inline int arm64_is_cmn(u32 encoding) {
    return (encoding & 0xFF000000) == 0xAB000000;
}

static inline int arm64_is_tst(u32 encoding) {
    return (encoding & 0xFF200000) == 0xEA000000;
}

/* ============================================================================
 * Branch Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_b(u32 encoding) {
    return (encoding & 0xFC000000) == 0x14000000;
}

static inline int arm64_is_bl(u32 encoding) {
    return (encoding & 0xFC000000) == 0x94000000;
}

static inline int arm64_is_br(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0xD61F0000;
}

static inline int arm64_is_ret(u32 encoding) {
    return (encoding & 0xFFFFFBFF) == 0xD65F0000;
}

static inline int arm64_is_bcond(u32 encoding) {
    return (encoding & 0xFF000000) == 0x54000000;
}

static inline int arm64_is_cbz(u32 encoding) {
    return (encoding & 0x7F800000) == 0x34000000;
}

static inline int arm64_is_cbnz(u32 encoding) {
    return (encoding & 0x7F800000) == 0x35000000;
}

static inline int arm64_is_tbz(u32 encoding) {
    return (encoding & 0x7F000000) == 0x36000000;
}

static inline int arm64_is_tbnz(u32 encoding) {
    return (encoding & 0x7F000000) == 0x37000000;
}

/* ============================================================================
 * Load/Store Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_ldr(u32 encoding) {
    return (encoding & 0xFF800000) == 0xF9400000;
}

static inline int arm64_is_str(u32 encoding) {
    return (encoding & 0xFF800000) == 0xF9000000;
}

static inline int arm64_is_ldrb(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x38C00000;
}

static inline int arm64_is_strb(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x38800000;
}

static inline int arm64_is_ldrh(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x78C00000;
}

static inline int arm64_is_strh(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x78800000;
}

static inline int arm64_is_ldrsb(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x38C00000;
}

static inline int arm64_is_ldrsh(u32 encoding) {
    return (encoding & 0xFFC00000) == 0x78C00000;
}

static inline int arm64_is_ldrsw(u32 encoding) {
    return (encoding & 0xFFC00000) == 0xB8C00000;
}

static inline int arm64_is_ldp(u32 encoding) {
    return (encoding & 0xFF800000) == 0xA9400000;
}

static inline int arm64_is_stp(u32 encoding) {
    return (encoding & 0xFF800000) == 0xA9000000;
}

/* ============================================================================
 * MOV (wide) Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_movz(u32 encoding) {
    return (encoding & 0xFF800000) == 0xD2800000;
}

static inline int arm64_is_movk(u32 encoding) {
    return (encoding & 0xFF800000) == 0xF2800000;
}

static inline int arm64_is_movn(u32 encoding) {
    return (encoding & 0xFF800000) == 0x12800000;
}

/* ============================================================================
 * System Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_svc(u32 encoding) {
    return (encoding & 0xFFE00000) == 0xD4000000;
}

static inline int arm64_is_brk(u32 encoding) {
    return (encoding & 0xFFE00000) == 0xD4200000;
}

static inline int arm64_is_hlt(u32 encoding) {
    return (encoding & 0xFFE00000) == 0xD4000000;
}

static inline int arm64_is_mrs(u32 encoding) {
    return (encoding & 0xFFE00000) == 0xD5300000;
}

static inline int arm64_is_msr(u32 encoding) {
    return (encoding & 0xFFE00000) == 0xD5100000;
}

/* ============================================================================
 * FP/NEON Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_fp_insn(u32 encoding) {
    return arm64_get_opclass(encoding) == 3;
}

static inline int arm64_is_fmov_reg(u32 encoding) {
    return (encoding & 0xFFF0FC00) == 0x1E604000;
}

static inline int arm64_is_fmov_imm(u32 encoding) {
    return (encoding & 0xFF800000) == 0x1E600000;
}

static inline int arm64_is_fadd(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E200400;
}

static inline int arm64_is_fsub(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E200C00;
}

static inline int arm64_is_fmul(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E200000;
}

static inline int arm64_is_fdiv(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E201800;
}

static inline int arm64_is_fsqrt(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0x1E21C000;
}

static inline int arm64_is_fcmp(u32 encoding) {
    return (encoding & 0xFFE0F800) == 0x1E202000;
}

static inline int arm64_is_fcsel(u32 encoding) {
    return (encoding & 0xFFE0FC00) == 0x1E20C000;
}

static inline int arm64_is_fabs(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0x1E20C000;
}

static inline int arm64_is_fneg(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0x1E214000;
}

static inline int arm64_is_fcvtds(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0x1E60C000;
}

static inline int arm64_is_fcvtsd(u32 encoding) {
    return (encoding & 0xFFFFFC00) == 0x1E21C000;
}

static inline int arm64_is_frecpe(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E200000;
}

static inline int arm64_is_frsqrte(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x1E200000;
}

/* ============================================================================
 * NEON Vector Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_add_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0E200400;
}

static inline int arm64_is_sub_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x2E200400;
}

static inline int arm64_is_and_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0E000000;
}

static inline int arm64_is_orr_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0E200000;
}

static inline int arm64_is_eor_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x2E200000;
}

static inline int arm64_is_bic_vec(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x2E000000;
}

static inline int arm64_is_mul_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_sshr_vec(u32 encoding) {
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_ushr_vec(u32 encoding) {
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_shl_vec(u32 encoding) {
    return (encoding & 0xFE200400) == 0x0E000400;
}

static inline int arm64_is_cmgt_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_cmeq_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_cmge_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

static inline int arm64_is_cmhs_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E300000;
}

static inline int arm64_is_cmlt_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_umin_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_umax_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

static inline int arm64_is_smin_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_smax_vec(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E700000;
}

/* ============================================================================
 * NEON Load/Store Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_ld1(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0C000000;
}

static inline int arm64_is_st1(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x08000000;
}

static inline int arm64_is_ld2(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0C400000;
}

static inline int arm64_is_st2(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x08400000;
}

static inline int arm64_is_ld1_multiple(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0C200000;
}

static inline int arm64_is_st1_multiple(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x08200000;
}

/**
 * Get NEON load/store register count
 * @param encoding ARM64 instruction encoding
 * @return Number of registers (1-4)
 */
static inline u8 arm64_get_neon_reg_count(u32 encoding) {
    return ((encoding >> 10) & 0x03) + 1;
}

/**
 * Get NEON load/store size field
 * @param encoding ARM64 instruction encoding
 * @return Size field (0-3)
 */
static inline u8 arm64_get_neon_size(u32 encoding) {
    return (encoding >> 22) & 0x03;
}

/**
 * Get NEON load/store index offset
 * @param encoding ARM64 instruction encoding
 * @return Index offset
 */
static inline u8 arm64_get_neon_index(u32 encoding) {
    return (encoding >> 10) & 0x0F;
}

/* ============================================================================
 * NEON Load/Store Multiple Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_ld3(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0C600000;
}

static inline int arm64_is_st3(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x08600000;
}

static inline int arm64_is_ld4(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x0C700000;
}

static inline int arm64_is_st4(u32 encoding) {
    return (encoding & 0xFF20FC00) == 0x08700000;
}

/* ============================================================================
 * NEON Permutation Instruction Decoders
 * ============================================================================ */

static inline int arm64_is_tbl(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0C000000;
}

static inline int arm64_is_tbx(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0C200000;
}

static inline int arm64_is_dup(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_ext(u32 encoding) {
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

/**
 * Get table lookup register count
 * @param encoding ARM64 instruction encoding
 * @return Number of table registers (1-4)
 */
static inline u8 arm64_get_tbl_reg_count(u32 encoding) {
    return ((encoding >> 10) & 0x03) + 1;
}

/**
 * Get EXT extract index
 * @param encoding ARM64 instruction encoding
 * @return Extract index (byte offset)
 */
static inline u8 arm64_get_ext_index(u32 encoding) {
    return (encoding >> 10) & 0x0F;
}

/* ============================================================================
 * Block Terminator Detection
 * ============================================================================ */

/**
 * Check if instruction is a basic block terminator
 * @param encoding ARM64 instruction encoding
 * @return 1 if terminator, 0 otherwise
 */
static inline int arm64_is_block_terminator(u32 encoding) {
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

/**
 * Get instruction length (all ARM64 instructions are 4 bytes)
 * @param encoding ARM64 instruction encoding
 * @return 4
 */
static inline int arm64_instruction_length(u32 encoding) {
    (void)encoding;  /* Unused */
    return 4;
}

#endif /* ROSETTA_ARM64_DECODE_H */
