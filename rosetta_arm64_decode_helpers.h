/* ============================================================================
 * Rosetta Translator - ARM64 Decode Helpers
 * ============================================================================
 *
 * This module provides helper functions for decoding ARM64 instruction
 * encodings, including field extractors and instruction type predicates.
 * ============================================================================ */

#ifndef ROSETTA_ARM64_DECODE_HELPERS_H
#define ROSETTA_ARM64_DECODE_HELPERS_H

#include <stdint.h>

/* ============================================================================
 * ARM64 Instruction Encoding Masks
 * ============================================================================ */

#define ARM64_MASK_OP30     0xC0000000  /* Top 2 bits */
#define ARM64_MASK_OPCODE   0xFFE0E000  /* Main opcode */
#define ARM64_MASK_RD       0x0000001F  /* Destination register */
#define ARM64_MASK_RN       0x000003E0  /* First operand register */
#define ARM64_MASK_RM       0x001F0000  /* Second operand register */
#define ARM64_MASK_IMM12    0x00001FFC  /* 12-bit immediate */
#define ARM64_MASK_IMM16    0x00FF0000  /* 16-bit immediate (MOVZ/MOVK) */
#define ARM64_MASK_IMM26    0x03FFFFFF  /* 26-bit immediate (branch) */

/* ============================================================================
 * ARM64 Field Extractors
 * ============================================================================ */

/**
 * Extract opcode class from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Opcode class (0-3)
 */
static inline uint32_t arm64_get_opclass(uint32_t encoding)
{
    return (encoding >> 30) & 0x3;
}

/**
 * Extract destination register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rd(uint32_t encoding)
{
    return (encoding >> 0) & 0x1F;
}

/**
 * Extract first operand register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * Extract second operand register from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Register number (0-31)
 */
static inline uint8_t arm64_get_rm(uint32_t encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * Extract 12-bit immediate from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return 12-bit immediate value
 */
static inline uint16_t arm64_get_imm12(uint32_t encoding)
{
    return (encoding >> 10) & 0xFFF;
}

/**
 * Extract 26-bit branch immediate from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended branch offset
 */
static inline int32_t arm64_get_imm26(uint32_t encoding)
{
    return (int32_t)((encoding & 0x03FFFFFF) << 2);
}

/**
 * Extract 19-bit branch immediate from ARM64 instruction
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 19-bit immediate
 */
static inline int32_t arm64_get_imm19(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 13) >> 11;  /* Sign extend */
}

/**
 * Extract 14-bit CBZ/CBNZ immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 14-bit immediate
 */
static inline int32_t arm64_get_imm14(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 18) >> 16;  /* Sign extend */
}

/**
 * Extract 7-bit TBZ/TBNZ immediate
 * @param encoding ARM64 instruction encoding
 * @return Sign-extended 7-bit immediate
 */
static inline int32_t arm64_get_imm7(uint32_t encoding)
{
    return (int32_t)((encoding >> 5) << 25) >> 23;  /* Sign extend */
}

/**
 * Get test bit from TBZ/TBNZ
 * @param encoding ARM64 instruction encoding
 * @return Bit number to test (0-63)
 */
static inline uint8_t arm64_get_test_bit(uint32_t encoding)
{
    uint8_t bit5 = (encoding >> 31) & 0x20;
    uint8_t bit4_0 = (encoding >> 19) & 0x1F;
    return bit5 | bit4_0;
}

/**
 * Extract 16-bit immediate from MOVZ/MOVK/MOVN
 * @param encoding ARM64 instruction encoding
 * @return 16-bit immediate value
 */
static inline uint16_t arm64_get_imm16(uint32_t encoding)
{
    return (encoding >> 5) & 0xFFFF;
}

/**
 * Get shift amount for MOVZ/MOVK (0, 1, 2, 3 for LSL #0, #16, #32, #48)
 * @param encoding ARM64 instruction encoding
 * @return Shift amount
 */
static inline uint8_t arm64_get_hw(uint32_t encoding)
{
    return (encoding >> 21) & 0x3;
}

/**
 * Get shift amount from instruction
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (0, 1, 2, or 3 for LSL #0, #1, #2, #3)
 */
static inline uint8_t arm64_get_shift(uint32_t encoding)
{
    return (encoding >> 22) & 0x3;
}

/**
 * Get condition code from conditional branch
 * @param encoding ARM64 instruction encoding
 * @return Condition code (0-15)
 */
static inline uint8_t arm64_get_cond(uint32_t encoding)
{
    return (encoding >> 0) & 0x0F;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Data Processing
 * ============================================================================ */

static inline int arm64_is_add(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x0B000000;
}

static inline int arm64_is_sub(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0xCB000000;
}

static inline int arm64_is_add_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x91000000;
}

static inline int arm64_is_sub_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xD1000000;
}

static inline int arm64_is_and_imm(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x12000000;
}

static inline int arm64_is_and(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x0A000000;
}

static inline int arm64_is_orr(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x2A000000;
}

static inline int arm64_is_eor(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A000000;
}

static inline int arm64_is_mvn(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x4A200000;
}

static inline int arm64_is_mul(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x1B000000;
}

static inline int arm64_is_div(uint32_t encoding)
{
    return (encoding & 0xFFE0E000) == 0x9AC00000;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Compare/Test
 * ============================================================================ */

static inline int arm64_is_cmp(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xEB000000;
}

static inline int arm64_is_cmn(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xAB000000;
}

static inline int arm64_is_tst(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xEA000000;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Load/Store
 * ============================================================================ */

static inline int arm64_is_ldr(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8400000;
}

static inline int arm64_is_str(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8000000;
}

static inline int arm64_is_ldrb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38400000;
}

static inline int arm64_is_strb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38000000;
}

static inline int arm64_is_ldrh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78400000;
}

static inline int arm64_is_strh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78000000;
}

static inline int arm64_is_ldrsb(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x38C00000;
}

static inline int arm64_is_ldrsh(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0x78C00000;
}

static inline int arm64_is_ldrsw(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xB8400000;
}

static inline int arm64_is_ldur(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8400000;
}

static inline int arm64_is_stur(uint32_t encoding)
{
    return (encoding & 0xFFC00000) == 0xF8000000;
}

static inline int arm64_is_ldp(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x29400000;
}

static inline int arm64_is_stp(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x29000000;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Branch/Control Flow
 * ============================================================================ */

static inline int arm64_is_b(uint32_t encoding)
{
    return (encoding & 0xFC000000) == 0x14000000;
}

static inline int arm64_is_bl(uint32_t encoding)
{
    return (encoding & 0xFC000000) == 0x94000000;
}

static inline int arm64_is_br(uint32_t encoding)
{
    return (encoding & 0xFFFFFC00) == 0xD61F0000;
}

static inline int arm64_is_bcond(uint32_t encoding)
{
    return (encoding & 0xFF000000) == 0x54000000;
}

static inline int arm64_is_ret(uint32_t encoding)
{
    return (encoding & 0xFFFFFC1F) == 0xD65F0000;
}

static inline int arm64_is_cbz(uint32_t encoding)
{
    return (encoding & 0x7F000000) == 0x34000000;
}

static inline int arm64_is_cbnz(uint32_t encoding)
{
    return (encoding & 0x7F000000) == 0x35000000;
}

static inline int arm64_is_tbz(uint32_t encoding)
{
    return (encoding & 0x7E000000) == 0x36000000;
}

static inline int arm64_is_tbnz(uint32_t encoding)
{
    return (encoding & 0x7E000000) == 0x37000000;
}

static inline int arm64_is_block_terminator(uint32_t encoding)
{
    /* Branch instructions */
    if (arm64_is_b(encoding)) return 1;
    if (arm64_is_bl(encoding)) return 1;
    if (arm64_is_br(encoding)) return 1;
    if (arm64_is_bcond(encoding)) return 1;

    /* Conditional branches */
    if (arm64_is_cbz(encoding)) return 1;
    if (arm64_is_cbnz(encoding)) return 1;
    if (arm64_is_tbz(encoding)) return 1;
    if (arm64_is_tbnz(encoding)) return 1;

    /* Return instructions */
    if (arm64_is_ret(encoding)) return 1;

    /* System instructions (SVC/HLT/BRK) */
    if ((encoding & 0xFF000000) == 0xD4000000) return 1;

    return 0;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - System
 * ============================================================================ */

static inline int arm64_is_svc(uint32_t encoding)
{
    return (encoding & 0xFF000000) == 0xD4000000 && (encoding & 0xFFFF) != 0;
}

static inline int arm64_is_brk(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xD4200000;
}

static inline int arm64_is_hlt(uint32_t encoding)
{
    return (encoding & 0xFFE00000) == 0xD4000000;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Move Wide
 * ============================================================================ */

static inline int arm64_is_movz(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xD2800000;
}

static inline int arm64_is_movk(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0xF2800000;
}

static inline int arm64_is_movn(uint32_t encoding)
{
    return (encoding & 0xFF800000) == 0x12800000;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - SIMD/FP
 * ============================================================================ */

static inline int arm64_is_fp_insn(uint32_t encoding)
{
    return ((encoding >> 28) & 0x7) == 0x7;  /* op30 == 3 */
}

static inline uint8_t arm64_get_fp_opcode(uint32_t encoding)
{
    return (encoding >> 28) & 0xF;
}

static inline uint8_t arm64_get_vd(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

static inline uint8_t arm64_get_vn(uint32_t encoding)
{
    return (encoding >> 10) & 0x1F;
}

static inline uint8_t arm64_get_vm(uint32_t encoding)
{
    return (encoding >> 20) & 0x1F;
}

static inline uint8_t arm64_get_vd_q(uint32_t encoding)
{
    uint8_t vd = (encoding >> 5) & 0x1F;
    uint8_t q = (encoding >> 30) & 0x1;
    return vd | (q << 5);
}

static inline int arm64_is_fmov_imm(uint32_t encoding)
{
    /* FMov (immediate): 00011110001mmmmmmmmm1dddfffff */
    return (encoding & 0xFF800400) == 0x1E800000;
}

static inline int arm64_is_fmov_reg(uint32_t encoding)
{
    /* FMov (register): 0001111000000000000001dddfffff */
    return (encoding & 0xFFFF0400) == 0x1E800000;
}

static inline int arm64_is_fadd(uint32_t encoding)
{
    /* FADD Vd, Vn, Vm */
    return (encoding & 0xFFE0FC00) == 0x1E200000;
}

static inline int arm64_is_fsub(uint32_t encoding)
{
    /* FSUB Vd, Vn, Vm */
    return (encoding & 0xFFE0FC00) == 0x1E600000;
}

static inline int arm64_is_fmul(uint32_t encoding)
{
    /* FMUL Vd, Vn, Vm */
    return (encoding & 0xFFE0FC00) == 0x1E300000;
}

static inline int arm64_is_fdiv(uint32_t encoding)
{
    /* FDIV Vd, Vn, Vm */
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

static inline int arm64_is_fsqrt(uint32_t encoding)
{
    /* FSQRT Vd, Vn */
    return (encoding & 0xFFFFFC00) == 0x1E900000;
}

static inline int arm64_is_fcmp(uint32_t encoding)
{
    /* FCMP Vn, Vm */
    return (encoding & 0xFFE0FC00) == 0x1E800000;
}

static inline int arm64_is_fcvtds(uint32_t encoding)
{
    /* FCVTDS: 0001111000100001011000dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E60C000;
}

static inline int arm64_is_fcvtsd(uint32_t encoding)
{
    /* FCVTSD: 0001111000100001011001dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E20C000;
}

static inline int arm64_is_fcsel(uint32_t encoding)
{
    /* FCSEL: 00011110010mmmmm00110000dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E800C00;
}

static inline int arm64_is_fccmp(uint32_t encoding)
{
    /* FCCMP: 00011110001mmmmm00100000dddfffff */
    return (encoding & 0xFFE0FC00) == 0x1E800400;
}

static inline int arm64_is_fabs(uint32_t encoding)
{
    /* FABS: 0001111000100000010000dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E200800;
}

static inline int arm64_is_fneg(uint32_t encoding)
{
    /* FNEG: 0001111000100000010001dddfffff */
    return (encoding & 0xFFFFFC00) == 0x1E201800;
}

/* ============================================================================
 * ARM64 Instruction Type Predicates - Vector (NEON)
 * ============================================================================ */

static inline int arm64_is_add_vec(uint32_t encoding)
{
    /* ADD (vector): 0101ssss000mmmmm000001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_sub_vec(uint32_t encoding)
{
    /* SUB (vector): 0101ssss000mmmmm000011dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_and_vec(uint32_t encoding)
{
    /* AND (vector): 0101ssss000mmmmm000111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_orr_vec(uint32_t encoding)
{
    /* ORR (vector): 0101ssss000mmmmm000101dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_eor_vec(uint32_t encoding)
{
    /* EOR (vector): 0110ssss000mmmmm000111dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E800000;
}

static inline int arm64_is_bic_vec(uint32_t encoding)
{
    /* BIC (vector): 0110ssss000mmmmm000001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E600000;
}

static inline int arm64_is_mul_vec(uint32_t encoding)
{
    /* MUL (vector): 0101ssss000mmmmm100001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E000000;
}

static inline int arm64_is_cmgt_vec(uint32_t encoding)
{
    /* CMGT (vector): 0101ssss000mmmmm001011dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

static inline int arm64_is_cmeq_vec(uint32_t encoding)
{
    /* CMEQ (vector): 0101ssss000mmmmm001001dddddddd */
    return (encoding & 0xFE20FC00) == 0x0E200000;
}

/**
 * Get vector size field from instruction
 * @param encoding ARM64 instruction encoding
 * @return Size field (0-3)
 */
static inline uint8_t arm64_get_vec_size(uint32_t encoding)
{
    return ((encoding >> 22) & 0x3);
}

/**
 * Get Q bit (128-bit flag) from instruction
 * @param encoding ARM64 instruction encoding
 * @return 1 if 128-bit operation, 0 if 64-bit
 */
static inline uint8_t arm64_get_q_bit(uint32_t encoding)
{
    return ((encoding >> 30) & 0x1);
}

/**
 * Get shift immediate from SIMD shift instruction
 * @param encoding ARM64 instruction encoding
 * @return Shift amount (signed, can be negative for right shifts)
 */
static inline int8_t arm64_get_shift_imm(uint32_t encoding)
{
    /* Shift immediate is in bits 16-21 (6 bits, signed) */
    int8_t imm = ((encoding >> 16) & 0x3F);
    /* Sign extend from 6 bits */
    if (imm & 0x20) imm |= 0xC0;  /* Negative */
    return imm;
}

/**
 * Map ARM64 vector register to x86_64 XMM register
 * @param vreg ARM64 vector register number (0-31)
 * @return x86_64 XMM register index (0-15)
 */
static inline uint8_t map_vreg_to_xmm(uint8_t vreg)
{
    return vreg & 0xF;
}

/* ============================================================================
 * ARM64 Instruction Length
 * ============================================================================ */

/**
 * Get the length of an ARM64 instruction (all are 4 bytes)
 * @param encoding ARM64 instruction encoding
 * @return Instruction length in bytes
 */
static inline int arm64_instruction_length(uint32_t encoding)
{
    (void)encoding;  /* All ARM64 instructions are 4 bytes */
    return 4;
}

#endif /* ROSETTA_ARM64_DECODE_HELPERS_H */
