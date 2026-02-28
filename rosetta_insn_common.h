/* ============================================================================
 * Rosetta - Common Instruction Utilities
 * ============================================================================
 *
 * This header provides common utilities and helpers shared across
 * instruction translation modules.
 * ============================================================================ */

#ifndef ROSETTA_INSN_COMMON_H
#define ROSETTA_INSN_COMMON_H

#include "rosetta_types.h"
#include <stdint.h>

/* code_buffer_t is defined in rosetta_types.h */

/* ============================================================================
 * Instruction Classification
 * ============================================================================ */

/* Instruction class */
typedef enum {
    INSN_CLASS_UNKNOWN = 0,
    INSN_CLASS_ALU,
    INSN_CLASS_MEMORY,
    INSN_CLASS_BRANCH,
    INSN_CLASS_SYSTEM,
    INSN_CLASS_NEON,
    INSN_CLASS_FP
} arm64_insn_class_t;

/* Instruction type */
typedef enum {
    INSN_TYPE_UNKNOWN = 0,
    INSN_TYPE_ADDSUB,
    INSN_TYPE_LOGICAL,
    INSN_TYPE_MUL,
    INSN_TYPE_DIV,
    INSN_TYPE_ADR,
    INSN_TYPE_LDRSTR,
    INSN_TYPE_LDPSTP,
    INSN_TYPE_B,
    INSN_TYPE_BL,
    INSN_TYPE_BR,
    INSN_TYPE_COND
} arm64_insn_type_t;

/* Instruction information structure */
typedef struct {
    uint32_t opcode;
    uint8_t rd;
    uint8_t rn;
    uint8_t rm;
    uint8_t ra;
    uint8_t sf;           /* Size flag: 0=32-bit, 1=64-bit */
    uint8_t class;        /* Instruction class */
    uint8_t type;         /* Instruction type */
    uint32_t imm;         /* Immediate value if any */
    uint8_t cond;         /* Condition code if any */
    uint8_t shift_type;   /* Shift type */
    uint8_t shift_amount; /* Shift amount */
} arm64_insn_info_t;

/* ============================================================================
 * ARM64 Instruction Encoding Helpers
 * ============================================================================ */

/**
 * insn_extract_bits - Extract bits from instruction encoding
 * @encoding: ARM64 instruction encoding
 * @lsb: Least significant bit position
 * @width: Number of bits to extract
 * Returns: Extracted bit field value
 */
static inline uint32_t insn_extract_bits(uint32_t encoding, uint8_t lsb, uint8_t width)
{
    return (encoding >> lsb) & ((1u << width) - 1u);
}

/**
 * insn_get_rd - Get destination register (Rd field)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_rd(uint32_t encoding)
{
    return encoding & 0x1F;
}

/**
 * insn_get_rn - Get first source register (Rn field)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_rn(uint32_t encoding)
{
    return (encoding >> 5) & 0x1F;
}

/**
 * insn_get_rm - Get second source register (Rm field)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_rm(uint32_t encoding)
{
    return (encoding >> 16) & 0x1F;
}

/**
 * insn_get_rt - Get transfer register (Rt field for load/store)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_rt(uint32_t encoding)
{
    return encoding & 0x1F;
}

/**
 * insn_get_rt2 - Get second transfer register (Rt2 field for load/store pair)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_rt2(uint32_t encoding)
{
    return (encoding >> 10) & 0x1F;
}

/**
 * insn_get_ra - Get accumulate register (Ra field for fused operations)
 * @encoding: ARM64 instruction encoding
 * Returns: Register number (0-31)
 */
static inline uint8_t insn_get_ra(uint32_t encoding)
{
    return (encoding >> 10) & 0x1F;
}

/**
 * insn_get_size - Get size field from instruction
 * @encoding: ARM64 instruction encoding
 * @lsb: LSB of size field (varies by instruction type)
 * @width: Width of size field (usually 2 bits)
 * Returns: Size encoding (0=8bit, 1=16bit, 2=32bit, 3=64bit typically)
 */
static inline uint8_t insn_get_size(uint32_t encoding, uint8_t lsb, uint8_t width)
{
    return insn_extract_bits(encoding, lsb, width);
}

/**
 * insn_get_imm - Get immediate field from instruction
 * @encoding: ARM64 instruction encoding
 * @type: Immediate type (shifted, logical, etc.)
 * Returns: Immediate value
 */
static inline uint32_t insn_get_imm_shifted(uint32_t encoding)
{
    return insn_extract_bits(encoding, 10, 6);
}

/**
 * insn_get_imm_logical - Get 12-bit immediate for logical operations
 * @encoding: ARM64 instruction encoding
 * Returns: 12-bit immediate value
 */
static inline uint32_t insn_get_imm_logical(uint32_t encoding)
{
    return insn_extract_bits(encoding, 10, 12);
}

/**
 * insn_get_imm_addsub - Get 12-bit immediate for add/sub operations
 * @encoding: ARM64 instruction encoding
 * Returns: 12-bit immediate value
 */
static inline uint32_t insn_get_imm_addsub(uint32_t encoding)
{
    return insn_extract_bits(encoding, 10, 12);
}

/**
 * insn_get_shift_type - Get shift type from instruction
 * @encoding: ARM64 instruction encoding
 * @lsb: LSB position of shift type field
 * Returns: Shift type (0=LSL, 1=LSR, 2=ASR, 3=ROR)
 */
static inline uint8_t insn_get_shift_type(uint32_t encoding, uint8_t lsb)
{
    return insn_extract_bits(encoding, lsb, 2);
}

/**
 * insn_get_shift_amount - Get shift amount from instruction
 * @encoding: ARM64 instruction encoding
 * @size: Element size (0=32bit, 1=64bit)
 * Returns: Shift amount
 */
static inline uint8_t insn_get_shift_amount(uint32_t encoding, uint8_t size)
{
    uint8_t shift_enc = insn_extract_bits(encoding, 22, 6);
    /* For 64-bit operations, bit 5 of shift amount is significant */
    if (size == 1 && (shift_enc & 0x20)) {
        return shift_enc;
    }
    return shift_enc & 0x1F;  /* Max 31 for 32-bit */
}

/**
 * insn_is_64bit - Check if instruction operates on 64-bit values
 * @encoding: ARM64 instruction encoding
 * @sf_pos: Position of SF bit (usually 31)
 * Returns: 1 if 64-bit, 0 if 32-bit
 */
static inline int insn_is_64bit(uint32_t encoding, uint8_t sf_pos)
{
    return (encoding >> sf_pos) & 1;
}

/**
 * insn_get_cond - Get condition code from instruction
 * @encoding: ARM64 instruction encoding
 * Returns: Condition code (0-15)
 */
static inline uint8_t insn_get_cond(uint32_t encoding)
{
    return insn_extract_bits(encoding, 12, 4);
}

/**
 * insn_get_option - Get option field from instruction
 * @encoding: ARM64 instruction encoding
 * @lsb: LSB position of option field
 * Returns: Option field value
 */
static inline uint8_t insn_get_option(uint32_t encoding, uint8_t lsb)
{
    return insn_extract_bits(encoding, lsb, 3);
}

/**
 * insn_get_extend - Get extend type from instruction
 * @encoding: ARM64 instruction encoding
 * Returns: Extend type (0=UXTB, 1=UXTH, 2=UXTW, 3=UXTX, 4=SXTB, 5=SXTH, 6=SXTW, 7=SXTX)
 */
static inline uint8_t insn_get_extend_type(uint32_t encoding)
{
    return insn_extract_bits(encoding, 13, 3);
}

/* ============================================================================
 * Vector/NEON Instruction Helpers
 * ============================================================================ */

/**
 * insn_get_vector_size - Get vector size from SIMD instruction
 * @encoding: ARM64 instruction encoding
 * @q_pos: Position of Q bit (usually 30)
 * @size_pos: Position of size field (usually 22-23)
 * Returns: Element size encoding
 */
static inline uint8_t insn_get_vector_size(uint32_t encoding, uint8_t q_pos, uint8_t size_pos)
{
    uint8_t q = (encoding >> q_pos) & 1;
    uint8_t size = insn_extract_bits(encoding, size_pos, 2);
    /* Q bit modifies interpretation of size field */
    return (q << 2) | size;
}

/**
 * insn_get_vector_index - Get vector index from instruction
 * @encoding: ARM64 instruction encoding
 * @index_pos: Position of index field
 * Returns: Vector lane index
 */
static inline uint8_t insn_get_vector_index(uint32_t encoding, uint8_t index_pos)
{
    return insn_extract_bits(encoding, index_pos, 4);
}

/**
 * insn_get_arrangement - Get SIMD arrangement specifier
 * @encoding: ARM64 instruction encoding
 * Returns: Arrangement encoding (interpretation depends on instruction class)
 */
static inline uint8_t insn_get_arrangement(uint32_t encoding)
{
    uint8_t size = insn_extract_bits(encoding, 22, 2);
    uint8_t q = (encoding >> 30) & 1;
    return (q << 2) | size;
}

/* ============================================================================
 * x86_64 Register Mapping Helpers
 * ============================================================================ */

/**
 * x86_reg_for_arm64 - Map ARM64 register to x86_64 register
 * @arm_reg: ARM64 register number (0-31)
 * Returns: x86_64 register encoding for REX prefix
 */
static inline uint8_t x86_reg_for_arm64(uint8_t arm_reg)
{
    return arm_reg & 0x0F;  /* x86_64 has 16 GPRs */
}

/**
 * x86_xmm_for_arm64_vec - Map ARM64 vector register to x86_64 XMM register
 * @arm_vec: ARM64 vector register number (0-31)
 * Returns: XMM register number
 */
static inline uint8_t x86_xmm_for_arm64_vec(uint8_t arm_vec)
{
    return arm_vec & 0x0F;  /* x86_64 has 16 XMM registers */
}

/* ============================================================================
 * Code Emission Helpers
 *
 * Note: For actual code emission, use the functions from rosetta_emit_x86.h:
 * - code_buf_emit_byte()
 * - code_buf_emit_word16()
 * - code_buf_emit_word32()
 * - emit_x86_rex()
 * - emit_x86_modrm()
 *
 * The helpers below are provided for convenience but may duplicate
 * functionality from rosetta_emit_x86.h.
 * ============================================================================ */

/**
 * emit_x86_byte_inline - Emit single x86 byte to buffer (inline helper)
 * @buf: Code buffer pointer
 * @byte: Byte to emit
 */
static inline void emit_x86_byte_inline(uint8_t **buf, uint8_t byte)
{
    *(*buf)++ = byte;
}

/**
 * emit_x86_word16_inline - Emit 16-bit word (little-endian)
 * @buf: Code buffer pointer
 * @word: 16-bit word to emit
 */
static inline void emit_x86_word16_inline(uint8_t **buf, uint16_t word)
{
    *(*buf)++ = word & 0xFF;
    *(*buf)++ = (word >> 8) & 0xFF;
}

/**
 * emit_x86_dword32_inline - Emit 32-bit dword (little-endian)
 * @buf: Code buffer pointer
 * @dword: 32-bit dword to emit
 */
static inline void emit_x86_dword32_inline(uint8_t **buf, uint32_t dword)
{
    *(*buf)++ = dword & 0xFF;
    *(*buf)++ = (dword >> 8) & 0xFF;
    *(*buf)++ = (dword >> 16) & 0xFF;
    *(*buf)++ = (dword >> 24) & 0xFF;
}

/**
 * emit_x86_modrm_inline - Emit ModR/M byte (inline helper)
 * @buf: Code buffer pointer
 * @mod: Mod field (2 bits)
 * @reg: Reg field (3 bits)
 * @rm: R/M field (3 bits)
 */
static inline void emit_x86_modrm_inline(uint8_t **buf, uint8_t mod, uint8_t reg, uint8_t rm)
{
    emit_x86_byte_inline(buf, ((mod & 0x03) << 6) | ((reg & 0x07) << 3) | (rm & 0x07));
}

/**
 * emit_x86_rex_inline - Emit REX prefix (inline helper)
 * @buf: Code buffer pointer
 * @w: 1 for 64-bit operand size
 * @r: Extension for ModR/M reg field
 * @x: Extension for SIB index field
 * @b: Extension for ModR/M r/m field or opcode reg
 */
static inline void emit_x86_rex_inline(uint8_t **buf, uint8_t w, uint8_t r, uint8_t x, uint8_t b)
{
    emit_x86_byte_inline(buf, 0x40 | ((w & 1) << 3) | ((r & 1) << 2) | ((x & 1) << 1) | (b & 1));
}

/* ============================================================================
 * Condition Code Translation
 * ============================================================================ */

/* ARM64 condition codes */
#define ARM_COND_EQ  0x0  /* Z == 1 : Equal */
#define ARM_COND_NE  0x1  /* Z == 0 : Not equal */
#define ARM_COND_CS  0x2  /* C == 1 : Carry set / Unsigned lower or same */
#define ARM_COND_CC  0x3  /* C == 0 : Carry clear / Unsigned higher */
#define ARM_COND_MI  0x4  /* N == 1 : Minus / Negative */
#define ARM_COND_PL  0x5  /* N == 0 : Plus / Positive or zero */
#define ARM_COND_VS  0x6  /* V == 1 : Overflow */
#define ARM_COND_VC  0x7  /* V == 0 : No overflow */
#define ARM_COND_HI  0x8  /* C == 1 && Z == 0 : Unsigned higher */
#define ARM_COND_LS  0x9  /* C == 0 || Z == 1 : Unsigned lower or same */
#define ARM_COND_GE  0xA  /* N == V : Signed greater than or equal */
#define ARM_COND_LT  0xB  /* N != V : Signed less than */
#define ARM_COND_GT  0xC  /* Z == 0 && N == V : Signed greater than */
#define ARM_COND_LE  0xD  /* Z == 1 || N != V : Signed less than or equal */
#define ARM_COND_AL  0xE  /* Always */
#define ARM_COND_NV  0xF  /* Always (deprecated) */

/**
 * arm_cond_to_x86_opcode - Translate ARM condition to x86 opcode suffix
 * @cond: ARM64 condition code
 * Returns: x86 condition opcode (0x84-0x8F range for Jcc/CMOVcc)
 */
static inline uint8_t arm_cond_to_x86_opcode(uint8_t cond)
{
    /* ARM and x86 condition codes mostly align with simple translation */
    static const uint8_t cond_table[16] = {
        0x84, /* EQ -> Z (equal) */
        0x85, /* NE -> NZ (not equal) */
        0x82, /* CS/LS -> B (below or same) */
        0x83, /* CC/HI -> NB/A (above) */
        0x88, /* MI -> S (sign) */
        0x89, /* PL -> NS (not sign) */
        0x80, /* VS -> O (overflow) */
        0x81, /* VC -> NO (no overflow) */
        0x87, /* HI -> A (above) */
        0x86, /* LS -> BE (below or equal) */
        0x8D, /* GE -> GE (greater or equal) */
        0x8C, /* LT -> L (less than) */
        0x8F, /* GT -> G (greater than) */
        0x8E, /* LE -> LE (less or equal) */
        0,    /* AL - no condition */
        0     /* NV - no condition */
    };
    return cond_table[cond & 0x0F];
}

/* ============================================================================
 * Function Declarations (non-inline functions in rosetta_insn_common.c)
 * ============================================================================ */

/* Instruction decoding */
int decode_arm64_instruction(uint32_t encoding, arm64_insn_info_t *insn_info);

/* Condition code translation */
uint8_t arm_to_x86_condition(uint8_t arm_cond);
uint8_t get_condition_flags(uint8_t cond);

/* Register mapping */
uint8_t map_arm64_reg_to_x86(uint8_t arm_reg);

/* Sign/Zero extension */
int64_t sign_extend_8(int8_t val);
int64_t sign_extend_16(int16_t val);
int64_t sign_extend_32(int32_t val);
uint64_t zero_extend_8(uint8_t val);
uint64_t zero_extend_16(uint16_t val);
uint64_t zero_extend_32(uint32_t val);

/* Immediate decoding */
uint64_t decode_logical_immediate(uint32_t imm, uint8_t immr, uint8_t imms, int sf);
uint64_t decode_addsub_immediate(uint16_t imm12, uint8_t shift);

/* Note: Code emission functions are provided by rosetta_emit_x86.h:
 * - emit_x86_push_reg, emit_x86_pop_reg
 * - emit_x86_mov_reg_reg, emit_x86_mov_reg_imm64, emit_x86_mov_reg_imm32
 * - emit_x86_add_reg_reg, emit_x86_sub_reg_reg
 * - emit_x86_test_reg_reg, emit_x86_cmp_reg_reg
 * - emit_x86_jmp_rel32, emit_x86_call_rel32, emit_x86_ret, emit_x86_nop
 */

#endif /* ROSETTA_INSN_COMMON_H */
