/* ============================================================================
 * Rosetta - Common Instruction Utilities Implementation
 * ============================================================================
 *
 * This module implements common utilities and helpers shared across
 * instruction translation modules.
 * ============================================================================ */

#include "rosetta_insn_common.h"
#include <string.h>

/* ============================================================================
 * ARM64 Instruction Decoding - Extended Helpers
 * ============================================================================ */

/**
 * decode_arm64_instruction - Decode full ARM64 instruction
 * @encoding: ARM64 instruction encoding
 * @insn_info: Output instruction information structure
 * Returns: 0 on success, -1 on invalid encoding
 */
int decode_arm64_instruction(uint32_t encoding, arm64_insn_info_t *insn_info)
{
    if (!insn_info) {
        return -1;
    }

    memset(insn_info, 0, sizeof(*insn_info));

    /* Extract common fields */
    insn_info->opcode = (encoding >> 26) & 0x3F;
    insn_info->rd = insn_get_rd(encoding);
    insn_info->rn = insn_get_rn(encoding);
    insn_info->rm = insn_get_rm(encoding);
    insn_info->sf = (encoding >> 31) & 1;

    /* Decode by opcode class */
    switch (insn_info->opcode) {
        case 0x0B:  /* Add/subtract (register) */
        case 0x0A:  /* Add/subtract (register) with carry */
        case 0x2B:  /* Add/subtract (extended register) */
        case 0x1B:  /* Add/subtract (shifted register) */
            insn_info->class = INSN_CLASS_ALU;
            insn_info->type = INSN_TYPE_ADDSUB;
            break;

        case 0x00000010:  /* Adr (PC-relative) */
        case 0x00000030:  /* Adrp (PC-relative) */
            insn_info->class = INSN_CLASS_ALU;
            insn_info->type = INSN_TYPE_ADR;
            break;

        case 0x08:  /* Load/store register */
            insn_info->class = INSN_CLASS_MEMORY;
            insn_info->type = INSN_TYPE_LDRSTR;
            break;

        case 0x14:  /* Branch (immediate) */
        case 0x94:  /* Branch with link */
            insn_info->class = INSN_CLASS_BRANCH;
            insn_info->type = INSN_TYPE_B;
            break;

        default:
            insn_info->class = INSN_CLASS_UNKNOWN;
            insn_info->type = INSN_TYPE_UNKNOWN;
            break;
    }

    return 0;
}

/* ============================================================================
 * Condition Code Translation
 * ============================================================================ */

/**
 * arm_to_x86_condition - Translate ARM64 condition to x86 Jcc code
 * @arm_cond: ARM64 condition code (0-15)
 * Returns: x86 condition code for Jcc instructions
 */
uint8_t arm_to_x86_condition(uint8_t arm_cond)
{
    /* ARM64 to x86_64 condition code mapping */
    static const uint8_t cond_map[16] = {
        0x0,  /* EQ: Z=1  -> overflow */
        0x1,  /* NE: Z=0  -> not overflow */
        0x2,  /* CS/HS: C=1 -> below */
        0x3,  /* CC/LO: C=0 -> not below */
        0x8,  /* MI: N=1  -> sign */
        0x9,  /* PL: N=0  -> not sign */
        0x6,  /* VS: V=1  -> overflow */
        0x7,  /* VC: V=0  -> not overflow */
        0x7,  /* HI: C=1 && Z=0 -> above */
        0x6,  /* LS: C=0 || Z=1 -> not above */
        0xC,  /* GE: N=V  -> greater or equal (signed) */
        0xC,  /* LT: N!=V -> less than (signed) */
        0xF,  /* GT: Z=0 && N=V -> greater than (signed) */
        0xE,  /* LE: Z=1 || N!=V -> less or equal (signed) */
        0x0,  /* AL: always */
        0x0   /* NV: always */
    };

    return cond_map[arm_cond & 0x0F];
}

/**
 * get_condition_flags - Get condition flags for ARM64 condition
 * @cond: ARM64 condition code
 * Returns: Flags mask (NZCV bits)
 */
uint8_t get_condition_flags(uint8_t cond)
{
    /* Returns which flags are tested by each condition */
    static const uint8_t flag_map[16] = {
        (1 << 2),           /* EQ: Z */
        (1 << 2),           /* NE: Z */
        (1 << 1),           /* CS: C */
        (1 << 1),           /* CC: C */
        (1 << 3),           /* MI: N */
        (1 << 3),           /* PL: N */
        (1 << 0),           /* VS: V */
        (1 << 0),           /* VC: V */
        (1 << 1) | (1 << 2),/* HI: C && !Z */
        (1 << 1) | (1 << 2),/* LS: !C || Z */
        (1 << 3) | (1 << 0),/* GE: N == V */
        (1 << 3) | (1 << 0),/* LT: N != V */
        (1 << 3) | (1 << 0) | (1 << 2), /* GT: !Z && N == V */
        (1 << 3) | (1 << 0) | (1 << 2), /* LE: Z || N != V */
        0,                  /* AL: no flags */
        0                   /* NV: no flags */
    };

    return flag_map[cond & 0x0F];
}

/* ============================================================================
 * Register Mapping
 * ============================================================================ */

/* ARM64 to x86_64 register mapping */
static const uint8_t g_arm64_to_x86_map[32] = {
    0,   /* X0 -> RAX */
    1,   /* X1 -> RCX */
    2,   /* X2 -> RDX */
    3,   /* X3 -> RBX */
    4,   /* X4 -> RSP (careful!) */
    5,   /* X5 -> RBP */
    6,   /* X6 -> RSI */
    7,   /* X7 -> RDI */
    8,   /* X8 -> R8 */
    9,   /* X9 -> R9 */
    10,  /* X10 -> R10 */
    11,  /* X11 -> R11 */
    12,  /* X12 -> R12 */
    13,  /* X13 -> R13 */
    14,  /* X14 -> R14 */
    15,  /* X15 -> R15 */
    16,  /* X16 -> (spill) */
    17,  /* X17 -> (spill) */
    18,  /* X18 -> (platform) */
    19,  /* X19 -> RBX (callee-saved) */
    20,  /* X20 -> R12 (callee-saved) */
    21,  /* X21 -> R13 (callee-saved) */
    22,  /* X22 -> R14 (callee-saved) */
    23,  /* X23 -> R15 (callee-saved) */
    24,  /* X24 -> (callee-saved) */
    25,  /* X25 -> (callee-saved) */
    26,  /* X26 -> (callee-saved) */
    27,  /* X27 -> (callee-saved) */
    28,  /* X28 -> (callee-saved) */
    29,  /* X29/FP -> RBP */
    30,  /* X30/LR -> (link) */
    31   /* X31/SP -> RSP */
};

/**
 * map_arm64_reg_to_x86 - Map ARM64 register to x86_64 register
 * @arm_reg: ARM64 register number (0-31)
 * Returns: x86_64 register number (0-15)
 */
uint8_t map_arm64_reg_to_x86(uint8_t arm_reg)
{
    if (arm_reg > 31) {
        return 0;  /* Invalid, return RAX */
    }
    return g_arm64_to_x86_map[arm_reg];
}

/* ============================================================================
 * Sign/Zero Extension Helpers
 * ============================================================================ */

/**
 * sign_extend_8 - Sign-extend 8-bit value to 64-bit
 * @val: 8-bit value
 * Returns: Sign-extended 64-bit value
 */
int64_t sign_extend_8(int8_t val)
{
    return (int64_t)val;
}

/**
 * sign_extend_16 - Sign-extend 16-bit value to 64-bit
 * @val: 16-bit value
 * Returns: Sign-extended 64-bit value
 */
int64_t sign_extend_16(int16_t val)
{
    return (int64_t)val;
}

/**
 * sign_extend_32 - Sign-extend 32-bit value to 64-bit
 * @val: 32-bit value
 * Returns: Sign-extended 64-bit value
 */
int64_t sign_extend_32(int32_t val)
{
    return (int64_t)val;
}

/**
 * zero_extend_8 - Zero-extend 8-bit value to 64-bit
 * @val: 8-bit value
 * Returns: Zero-extended 64-bit value
 */
uint64_t zero_extend_8(uint8_t val)
{
    return (uint64_t)val;
}

/**
 * zero_extend_16 - Zero-extend 16-bit value to 64-bit
 * @val: 16-bit value
 * Returns: Zero-extended 64-bit value
 */
uint64_t zero_extend_16(uint16_t val)
{
    return (uint64_t)val;
}

/**
 * zero_extend_32 - Zero-extend 32-bit value to 64-bit
 * @val: 32-bit value
 * Returns: Zero-extended 64-bit value
 */
uint64_t zero_extend_32(uint32_t val)
{
    return (uint64_t)val;
}

/* ============================================================================
 * ARM64 Immediate Decoding
 * ============================================================================ */

/**
 * decode_logical_immediate - Decode logical immediate value
 * @imm: 12-bit or 6-bit immediate field
 * @immr: Rotation amount
 * @imms: Immediate size
 * @sf: Size flag (0=32-bit, 1=64-bit)
 * Returns: Decoded 64-bit immediate value
 */
uint64_t decode_logical_immediate(uint32_t imm, uint8_t immr, uint8_t imms, int sf)
{
    int len = sf ? 6 : 5;
    uint64_t result = 0;
    int i;

    /* Decode the immediate pattern */
    uint64_t pattern = (1ULL << (imms + 1)) - 1;

    /* Rotate the pattern */
    if (sf) {
        result = ((pattern << immr) | (pattern >> (64 - immr))) & 0xFFFFFFFFFFFFFFFFULL;
    } else {
        result = ((pattern << immr) | (pattern >> (32 - immr))) & 0xFFFFFFFFULL;
    }

    return result;
}

/**
 * decode_addsub_immediate - Decode add/sub immediate
 * @imm12: 12-bit immediate
 * @shift: Shift amount (0 or 12)
 * Returns: Decoded immediate value
 */
uint64_t decode_addsub_immediate(uint16_t imm12, uint8_t shift)
{
    return (uint64_t)imm12 << (shift * 12);
}
