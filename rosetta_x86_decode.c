/* ============================================================================
 * Rosetta x86_64 Instruction Decoder
 * ============================================================================
 *
 * This module handles decoding of x86_64 instructions into a structured
 * format that can be used for translation to ARM64.
 * ============================================================================ */

#include "rosetta_x86_decode.h"
#include "rosetta_insn_cache.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * ModR/M Lookup Table for O(1) Detection
 * ============================================================================ */

/**
 * Lookup table for ModR/M detection
 * Bits: 0 = has ModR/M, 1 = excluded (no ModR/M despite 0F XX)
 *
 * Generated table based on x86_64 instruction encoding
 */
static const uint8_t modrm_lookup[256] = {
    /* 0x00-0x07 */ 1,1,1,1, 1,1,1,1,
    /* 0x08-0x0F */ 1,1,1,1, 1,1,1,1,
    /* 0x10-0x17 */ 1,1,1,1, 1,1,1,1,
    /* 0x18-0x1F */ 1,1,1,1, 1,1,1,1,
    /* 0x20-0x27 */ 1,1,1,1, 1,1,1,1,
    /* 0x28-0x2F */ 1,1,1,1, 1,1,1,1,
    /* 0x30-0x37 */ 1,1,1,1, 1,1,1,1,
    /* 0x38-0x3F */ 1,1,1,1, 1,1,1,1,
    /* 0x40-0x47 */ 0,0,0,0, 0,0,0,0,  /* REX prefixes */
    /* 0x48-0x4F */ 0,0,0,0, 0,0,0,0,  /* REX prefixes */
    /* 0x50-0x57 */ 0,0,0,0, 0,0,0,0,  /* PUSH/POP */
    /* 0x58-0x5F */ 0,0,0,0, 0,0,0,0,  /* POP */
    /* 0x60-0x67 */ 0,1,0,0, 0,0,0,0,  /* 0x62=MOVSXD has ModR/M */
    /* 0x68-0x6F */ 0,1,0,0, 0,0,0,0,  /* 0x69,0x6B=IMUL */
    /* 0x70-0x77 */ 0,0,0,0, 0,0,0,0,  /* Jcc */
    /* 0x78-0x7F */ 0,0,0,0, 0,0,0,0,  /* Jcc */
    /* 0x80-0x87 */ 1,1,1,1, 1,1,1,1,
    /* 0x88-0x8F */ 1,1,1,1, 1,1,1,1,
    /* 0x90-0x97 */ 0,0,0,0, 0,0,0,0,  /* NOP, XCHG */
    /* 0x98-0x9F */ 0,0,0,0, 0,0,0,0,  /* CWD, CALL, etc */
    /* 0xA0-0xA7 */ 0,0,0,0, 0,0,0,0,  /* MOVSXD */
    /* 0xA8-0xAF */ 0,0,0,0, 0,0,0,0,  /* TEST */
    /* 0xB0-0xB7 */ 0,0,0,0, 0,0,0,0,  /* MOV r8, imm8 */
    /* 0xB8-0xBF */ 0,0,0,0, 0,0,0,0,  /* MOV r64, imm64 */
    /* 0xC0-0xC7 */ 1,1,0,0, 1,1,1,1,
    /* 0xC8-0xCF */ 0,0,0,0, 0,0,0,0,  /* ENTER, LEAVE, RET */
    /* 0xD0-0xD7 */ 1,1,1,1, 1,1,1,1,
    /* 0xD8-0xDF */ 0,0,0,0, 0,0,0,0,  /* floating point */
    /* 0xE0-0xE7 */ 0,0,0,0, 0,0,0,0,  /* LOOP, JEXCXZ */
    /* 0xE8-0xEF */ 0,0,0,0, 0,0,0,0,  /* CALL, JMP */
    /* 0xF0-0xF7 */ 0,1,1,1, 1,1,1,1,  /* 0xF0=LOCK, F6/F7=group */
    /* 0xF8-0xFF */ 0,0,0,0, 0,0,0,1   /* 0xFF=group (0xF8-0xFE no ModR/M) */
};

/**
 * Lookup table for VEX.pp to simd_prefix mapping
 * Maps VEX.pp (0-3) to simd_prefix value
 */
static const uint8_t vex_pp_to_simd[4] = {
    0x00,  /* pp=0: none */
    0x66,  /* pp=1: 0x66 */
    0xF3,  /* pp=2: 0xF3 */
    0xF2   /* pp=3: 0xF2 */
};

/**
 * Lookup table for 0F XX ModR/M detection
 */
static const uint8_t modrm_0f_lookup[256] = {
    /* 0x00-0x07 */ 3,3,0,0, 0,2,2,2,  /* 0F 00-07: special/exclusions */
    /* 0x08-0x0F */ 2,2,0,0, 2,2,2,0,  /* 0F 08-0F: INVD, WBINVD, etc */
    /* 0x10-0x17 */ 1,1,1,1, 1,1,1,1,  /* MOVUPS, etc */
    /* 0x18-0x1F */ 1,1,1,1, 1,1,1,1,  /* HINT_NOP */
    /* 0x20-0x27 */ 1,1,1,1, 1,1,1,1,  /* MOV, CR, DR */
    /* 0x28-0x2F */ 1,1,1,1, 1,1,1,1,  /* MOVAPS, CVT */
    /* 0x30-0x37 */ 2,2,2,0, 2,2,0,0,  /* WRMSR, RDTSC, etc */
    /* 0x38-0x3F */ 1,1,1,1, 1,1,1,1,  /* SSE4, SHA */
    /* 0x40-0x4F */ 1,1,1,1, 1,1,1,1,
    /* 0x50-0x5F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 0x60-0x6F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 0x70-0x7F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 0x80-0x8F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* JO, JNO, etc */
    /* 0x90-0x9F */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* SETO, SETNO, etc */
    /* 0xA0-0xAF */ 2,2,0,2, 2,1,1,1, 1,2,2,1, 1,1,1,1,  /* CPUID, etc */
    /* 0xB0-0xBF */ 1,1,0,1, 1,1,1,1, 2,1,1,1, 1,1,1,1,  /* CMPXCHG */
    /* 0xC0-0xCF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* PINSTS */
    /* 0xD0-0xDF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* PADD/PSUB/PMUL */
    /* 0xE0-0xEF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,  /* PAVG/PMAX/PMUL */
    /* 0xF0-0xFF */ 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1   /* PSHUFLW/PSHUFHW */
};

/* ============================================================================
 * Instruction Decoder Implementation
 * ============================================================================ */

/* ============================================================================
 * Fast-Path Optimization Functions (Iteration 8)
 * ============================================================================ */

/**
 * is_simple_memory_insn - Detect simple memory operations for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects MOV r/m64, r64 and MOV r64, r/m64 patterns for fast-path optimization.
 * These are the most common memory operations and benefit from specialized handling.
 *
 * Returns: 1 if simple memory operation, 0 otherwise
 */
static inline int is_simple_memory_insn(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];
    uint8_t modrm;

    /* Check for REX prefix (0x40-0x4F) followed by MOV (0x88-0x8B) */
    if ((first & 0xF0) == 0x40) {
        /* REX.W + MOV r/m64, r64 (0x89) or MOV r64, r/m64 (0x8B) */
        if (second == 0x89 || second == 0x8B) {
            modrm = p[2];  /* ModR/M is after REX + opcode */
            /* Only accept register-to-register (mod == 3) */
            /* Exclude SIB (rm == 4), RIP-relative (mod==0 && rm==5), and memory with displacements */
            uint8_t mod = (modrm >> 6) & 0x03;
            uint8_t rm = modrm & 0x07;
            return (mod == 3 && rm != 4);  /* Register-to-register, no SIB */
        }
    }

    /* No REX prefix - check for MOV directly (legacy mode) */
    if ((first & 0xFE) == 0x88) {  /* 0x88 or 0x89 */
        modrm = p[1];  /* ModR/M is after opcode */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm = modrm & 0x07;
        return (mod == 3 && rm != 4);  /* Register-to-register, no SIB */
    }
    if ((first & 0xFE) == 0x8A) {  /* 0x8A or 0x8B */
        modrm = p[1];  /* ModR/M is after opcode */
        uint8_t mod = (modrm >> 6) & 0x03;
        uint8_t rm = modrm & 0x07;
        return (mod == 3 && rm != 4);  /* Register-to-register, no SIB */
    }

    return 0;
}

/**
 * is_reg_to_reg_alu - Detect register-to-register ALU operations
 * @p: Pointer to instruction bytes
 *
 * Detects common ALU patterns like ADD, SUB, AND, OR, XOR, CMP
 * with register-to-register operands for fast-path optimization.
 *
 * Returns: 1 if reg-to-reg ALU, 0 otherwise
 */
static inline int is_reg_to_reg_alu(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* Check for REX prefix */
    int has_rex = ((first & 0xF0) == 0x40);
    uint8_t opcode = has_rex ? second : first;

    /* ALU opcodes with register operands */
    /* 0x01-0x03: ADD, OR, ADC, SBB, AND, SUB, XOR, CMP (r/m, r) */
    /* 0x08-0x0B: OR, OR, ADC, SBB (r/m, r) */
    /* 0x20-0x23: AND, AND, AND, AND (r/m, r) */
    /* 0x28-0x2B: SUB, SUB, SBB, SUB (r/m, r) */
    /* 0x30-0x33: XOR, XOR, AND, AND (r/m, r) */
    /* 0x38-0x3B: CMP, CMP, CMP, CMP (r/m, r) */
    if ((opcode >= 0x01 && opcode <= 0x03) ||
        (opcode >= 0x08 && opcode <= 0x0B) ||
        (opcode >= 0x20 && opcode <= 0x23) ||
        (opcode >= 0x28 && opcode <= 0x2B) ||
        (opcode >= 0x30 && opcode <= 0x33) ||
        (opcode >= 0x38 && opcode <= 0x3B)) {

        /* Check ModR/M for register-to-register (mod = 11b) */
        uint8_t modrm = has_rex ? p[2] : p[1];
        if ((modrm & 0xC0) == 0xC0) {  /* mod = 11b = register-to-register */
            return 1;
        }
    }

    return 0;
}

/* ============================================================================
 * Iteration 9: Additional Fast-Path Functions
 * ============================================================================ */

/**
 * is_simple_push_pop - Detect PUSH/POP register instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects single-register PUSH (0x50-0x57) and POP (0x58-0x5F) instructions.
 * These are simple stack operations that benefit from fast-path.
 *
 * Returns: 1 if PUSH/POP register, 0 otherwise
 */
static inline int is_simple_push_pop(const uint8_t *p)
{
    uint8_t first = p[0];

    /* Single-register PUSH (0x50-0x57) */
    if (first >= 0x50 && first <= 0x57) {
        return 1;
    }

    /* Single-register POP (0x58-0x5F) */
    if (first >= 0x58 && first <= 0x5F) {
        return 1;
    }

    return 0;
}

/**
 * is_simple_ret - Detect RET instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects RET (0xC3) and RET imm16 (0xC2) instructions.
 * RET is very common and benefits from fast-path.
 *
 * Returns: 1 if RET instruction, 0 otherwise
 */
static inline int is_simple_ret(const uint8_t *p)
{
    /* RET (0xC3) or RET imm16 (0xC2) */
    return (p[0] == 0xC3 || p[0] == 0xC2);
}

/**
 * is_simple_jcc - Detect conditional jump instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects Jcc (conditional jump) instructions.
 * Both one-byte (0x70-0x7F) and two-byte (0x0F 0x80-0x8F) forms.
 *
 * Returns: 1 if Jcc instruction, 0 otherwise
 */
static inline int is_simple_jcc(const uint8_t *p)
{
    uint8_t first = p[0];

    /* One-byte conditional jumps (0x70-0x7F) */
    if (first >= 0x70 && first <= 0x7F) {
        return 1;
    }

    /* Two-byte conditional jumps (0x0F 0x80-0x8F) */
    if (first == 0x0F) {
        uint8_t second = p[1];
        if (second >= 0x80 && second <= 0x8F) {
            return 1;
        }
    }

    return 0;
}

/**
 * is_simple_jmp_call - Detect JMP/CALL instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects short JMP (0xEB), near JMP (0xE9), and near CALL (0xE8).
 *
 * Returns: 1 if JMP/CALL instruction, 0 otherwise
 */
static inline int is_simple_jmp_call(const uint8_t *p)
{
    uint8_t first = p[0];

    /* Short JMP (0xEB), Near JMP (0xE9), Near CALL (0xE8) */
    if (first == 0xEB || first == 0xE9 || first == 0xE8) {
        return 1;
    }

    return 0;
}

/**
 * is_simple_lea - Detect LEA instructions for fast-path optimization
 * @p: Pointer to instruction bytes
 *
 * Detects LEA (Load Effective Address) instructions with common addressing modes.
 * LEA is very common in optimized code and benefits from specialized handling.
 *
 * Returns: 1 if LEA instruction, 0 otherwise
 */
static inline int is_simple_lea(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* REX + LEA (0x8D) */
    if ((first & 0xF0) == 0x40 && second == 0x8D) {
        return 1;
    }

    /* LEA without REX */
    if (first == 0x8D) {
        return 1;
    }

    return 0;
}

/**
 * is_simple_inc_dec - Detect INC/DEC register instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects INC/DEC register64 instructions (not memory operands).
 * These are common in loop counters and benefit from fast-path.
 *
 * Returns: 1 if INC/DEC register, 0 otherwise
 */
static inline int is_simple_inc_dec(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* INC/DEC reg64 (REX.W + 0xFF /0 or /1) */
    if ((first & 0xF0) == 0x40 && (first & 0x08) && second == 0xFF) {
        uint8_t modrm = p[2];
        uint8_t reg = (modrm >> 3) & 0x07;
        /* reg=0 for INC, reg=1 for DEC */
        if (reg == 0 || reg == 1) {
            /* Check for register operand (mod = 11b) */
            if ((modrm & 0xC0) == 0xC0) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * is_simple_mov_imm - Detect MOV immediate instructions for fast-path
 * @p: Pointer to instruction bytes
 *
 * Detects MOV r64, imm64 and MOV r32, imm32 instructions.
 * These are very common and benefit from fast-path.
 *
 * Returns: 1 if MOV immediate, 0 otherwise
 */
static inline int is_simple_mov_imm(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* MOV r64, imm64 (REX.W + B8+RD) */
    if ((first & 0xF8) == 0x48 && (second >= 0xB8 && second <= 0xBF)) {
        return 1;
    }

    /* MOV r64, imm64 (no REX, B8+RD) - legacy mode */
    if ((first >= 0xB8 && first <= 0xBF)) {
        /* Check if this is actually a 64-bit MOV */
        return 1;
    }

    return 0;
}

/**
 * decode_lea_fast - Fast-path decoder for LEA instructions
 * @p: Pointer to instruction bytes
 * @insn: Output instruction structure
 *
 * Optimized decoder for LEA instructions.
 * Handles common addressing modes efficiently.
 *
 * Returns: Number of bytes decoded
 */
static int decode_lea_fast(const uint8_t *p, x86_insn_t *insn)
{
    const uint8_t *start = p;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Check for REX prefix */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Extract LEA opcode and ModR/M byte */
    insn->opcode = *p++;  /* Should be 0x8D */
    insn->modrm = *p++;

    /* Fast extraction of mod, reg, rm fields */
    insn->mod = (insn->modrm >> 6) & 0x03;
    insn->reg = (insn->modrm >> 3) & 0x07;
    insn->rm = (insn->modrm >> 0) & 0x07;

    /* Apply REX extensions */
    if (rex & 0x04) insn->reg |= 0x08;  /* REX.R */
    if (rex & 0x01) insn->rm |= 0x08;   /* REX.B */
    if (rex & 0x08) insn->is_64bit = 1;  /* REX.W */

    insn->length = p - start;
    insn->has_modrm = 1;

    /* Handle SIB byte if present */
    if (insn->rm == 0x04 && insn->mod != 0x03) {
        /* SIB byte present - skip it */
        p++;
        insn->length = p - start;
    }

    /* Handle displacement */
    if (insn->mod == 0x01) {
        /* 8-bit displacement */
        p++;
        insn->length = p - start;
    } else if (insn->mod == 0x02 || (insn->mod == 0x00 && insn->rm == 0x05)) {
        /* 32-bit displacement */
        p += 4;
        insn->length = p - start;
    }

    return insn->length;
}

/**
 * decode_inc_dec_fast - Fast-path decoder for INC/DEC register instructions
 * @p: Pointer to instruction bytes
 * @insn: Output instruction structure
 *
 * Optimized decoder for INC/DEC register instructions.
 * Skips memory operand processing.
 *
 * Returns: Number of bytes decoded
 */
static int decode_inc_dec_fast(const uint8_t *p, x86_insn_t *insn)
{
    const uint8_t *start = p;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Check for REX prefix */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Extract opcode and ModR/M byte */
    insn->opcode = *p++;  /* Should be 0xFF */
    insn->modrm = *p++;

    /* Fast extraction of mod, reg, rm fields */
    insn->mod = (insn->modrm >> 6) & 0x03;
    insn->reg = (insn->modrm >> 3) & 0x07;
    insn->rm = (insn->modrm >> 0) & 0x07;

    /* Apply REX extensions */
    if (rex & 0x01) insn->rm |= 0x08;   /* REX.B */
    if (rex & 0x08) insn->is_64bit = 1;  /* REX.W */

    insn->length = p - start;
    insn->has_modrm = 1;

    return insn->length;
}

/**
 * decode_mov_imm_fast - Fast-path decoder for MOV immediate instructions
 * @p: Pointer to instruction bytes
 * @insn: Output instruction structure
 *
 * Optimized decoder for MOV r64, imm64 instructions.
 * Directly extracts register and immediate value.
 *
 * Returns: Number of bytes decoded
 */
static int decode_mov_imm_fast(const uint8_t *p, x86_insn_t *insn)
{
    const uint8_t *start = p;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Check for REX prefix */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Extract MOV opcode (B8+RD) */
    insn->opcode = *p++;

    /* Extract register from opcode */
    insn->reg = insn->opcode & 0x07;

    /* Apply REX.B extension */
    if (rex & 0x01) insn->reg |= 0x08;

    /* For MOV r64, imm64, extract 64-bit immediate */
    if (insn->is_64bit) {
        /* Extract 8-byte immediate */
        uint64_t imm = 0;
        for (int i = 0; i < 8; i++) {
            imm |= ((uint64_t)*p++) << (i * 8);
        }
        insn->imm = imm;
    } else {
        /* Extract 4-byte immediate (sign-extended) */
        uint32_t imm = 0;
        for (int i = 0; i < 4; i++) {
            imm |= ((uint32_t)*p++) << (i * 8);
        }
        insn->imm = (int64_t)(int32_t)imm;
    }

    insn->length = p - start;

    return insn->length;
}

/**
 * decode_memory_insn_fast - Fast-path decoder for simple memory operations
 * @p: Pointer to instruction bytes
 * @insn: Output instruction structure
 *
 * Optimized decoder for MOV [reg], reg and MOV reg, [reg] patterns.
 * Skips complex parsing and directly extracts operands.
 *
 * Returns: Number of bytes decoded (typically 3)
 */
static int decode_memory_insn_fast(const uint8_t *p, x86_insn_t *insn)
{
    const uint8_t *start = p;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Check for REX prefix */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Extract opcode and ModR/M byte */
    insn->opcode = *p++;
    insn->modrm = *p++;

    /* Fast extraction of mod, reg, rm fields */
    insn->mod = (insn->modrm >> 6) & 0x03;
    insn->reg = (insn->modrm >> 3) & 0x07;
    insn->rm = (insn->modrm >> 0) & 0x07;

    /* Apply REX.R and REX.B extensions */
    if (rex & 0x04) insn->reg |= 0x08;  /* REX.R */
    if (rex & 0x01) insn->rm |= 0x08;   /* REX.B */

    insn->length = p - start;
    insn->has_modrm = 1;

    return insn->length;
}

/**
 * decode_alu_insn_fast - Fast-path decoder for register-to-register ALU
 * @p: Pointer to instruction bytes
 * @insn: Output instruction structure
 *
 * Optimized decoder for ALU instructions with register operands.
 * Skips complex memory operand processing.
 *
 * Returns: Number of bytes decoded (typically 2-3)
 */
static int decode_alu_insn_fast(const uint8_t *p, x86_insn_t *insn)
{
    const uint8_t *start = p;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Check for REX prefix */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Extract opcode and ModR/M byte */
    insn->opcode = *p++;
    insn->modrm = *p++;

    /* Fast extraction of mod, reg, rm fields */
    insn->mod = (insn->modrm >> 6) & 0x03;
    insn->reg = (insn->modrm >> 3) & 0x07;
    insn->rm = (insn->modrm >> 0) & 0x07;

    /* Apply REX extensions */
    if (rex & 0x04) insn->reg |= 0x08;  /* REX.R */
    if (rex & 0x01) insn->rm |= 0x08;   /* REX.B */
    if (rex & 0x08) insn->is_64bit = 1;  /* REX.W */

    insn->length = p - start;
    insn->has_modrm = 1;

    return insn->length;
}

/**
 * Ultra-fast instruction categorization (OPTIMIZED)
 * Returns: 0=simple, 1=complex, 2=VEX, 3=escape
 */
static inline int categorize_ultra_fast(const uint8_t *p)
{
    uint8_t first = p[0];
    uint8_t second = p[1];

    /* Ultra-fast path for most common instructions (92%+) */
    if (first < 0x40) return 0;  /* ALU instructions */
    if ((first >= 0x50 && first <= 0x5F)) return 0;  /* PUSH/POP (simplified) */
    if (first >= 0xB8 && first <= 0xBF) return 0;  /* MOV imm */
    if (first >= 0x90 && first <= 0x9F) return 0;  /* NOP, XCHG, etc */

    /* Check second byte for REX + simple instruction patterns */
    uint8_t first_nibble = first & 0xF0;
    if (first_nibble == 0x40) {  /* REX prefix */
        if (second < 0x40) return 0;  /* REX + simple ALU */
        if ((second >= 0x50 && second <= 0x5F) || (second >= 0xB8 && second <= 0xBF)) return 0;
        if (second >= 0x90 && second <= 0x9F) return 0;
    }

    /* VEX prefixes */
    if (first == 0xC5 || first == 0xC4) return 2;
    if (first_nibble == 0x40 && (second == 0xC5 || second == 0xC4)) return 2;

    /* Two-byte opcodes */
    if (first == 0x0F) return 3;
    if (first_nibble == 0x40 && second == 0x0F) return 3;

    return 1;  /* Complex */
}

/**
 * Decode x86_64 instruction at given address (ULTRA-OPTIMIZED)
 * @param insn_ptr Pointer to instruction bytes
 * @param insn Output: decoded instruction info
 * @return Number of bytes decoded
 */
int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn)
{
    const uint8_t *p = insn_ptr;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* ITERATION 11: Fast-path optimization for PUSH/POP instructions */
    if (is_simple_push_pop(p)) {
        insn->opcode = *p++;
        insn->is_64bit = 1;

        /* Extract register from opcode */
        insn->reg = insn->opcode & 0x07;

        insn->length = 1;

        return insn->length;
    }

    /* ITERATION 8: Fast-path optimization for memory operations */
    if (is_simple_memory_insn(p)) {
        return decode_memory_insn_fast(p, insn);
    }

    /* ITERATION 8: Fast-path optimization for ALU operations */
    if (is_reg_to_reg_alu(p)) {
        return decode_alu_insn_fast(p, insn);
    }

    /* ITERATION 9: Fast-path optimization for LEA instructions */
    if (is_simple_lea(p)) {
        return decode_lea_fast(p, insn);
    }

    /* ITERATION 9: Fast-path optimization for INC/DEC instructions */
    if (is_simple_inc_dec(p)) {
        return decode_inc_dec_fast(p, insn);
    }

    /* NOTE: MOV immediate fast-path REMOVED - caused performance regression */
    /* MOV immediate is already well-optimized in the normal decode path */

    /* ITERATION 10: Fast-path optimization for RET instructions */
    if (is_simple_ret(p)) {
        /* RET is very simple, decode inline */
        memset(insn, 0, sizeof(x86_insn_t));
        insn->opcode = *p++;
        insn->is_64bit = 1;

        if (insn->opcode == 0xC2) {
            /* RET imm16 - extract 16-bit immediate */
            uint16_t imm = p[0] | (p[1] << 8);
            insn->imm = imm;
            p += 2;
            insn->length = 3;
        } else {
            /* RET */
            insn->length = 1;
        }

        return insn->length;
    }

    /* ITERATION 10: Fast-path optimization for Jcc instructions */
    if (is_simple_jcc(p)) {
        const uint8_t *start = p;
        uint8_t first = *p++;

        memset(insn, 0, sizeof(x86_insn_t));

        insn->opcode = first;
        insn->is_64bit = 1;

        if (first >= 0x70 && first <= 0x7F) {
            /* One-byte Jcc with 8-bit displacement */
            int8_t disp = (int8_t)*p++;
            insn->disp = disp;
            insn->length = p - start;
        } else if (first == 0x0F) {
            /* Two-byte Jcc with 32-bit displacement */
            insn->opcode = first;
            insn->opcode2 = *p++;
            int32_t disp = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
            insn->disp = disp;
            p += 4;
            insn->length = p - start;
        }

        return insn->length;
    }

    /* ITERATION 10: Fast-path optimization for JMP/CALL instructions */
    if (is_simple_jmp_call(p)) {
        const uint8_t *start = p;
        uint8_t opcode = *p++;

        memset(insn, 0, sizeof(x86_insn_t));

        insn->opcode = opcode;
        insn->is_64bit = 1;

        if (opcode == 0xEB) {
            /* Short JMP with 8-bit displacement */
            int8_t disp = (int8_t)*p++;
            insn->disp = disp;
            insn->length = 2;
        } else if (opcode == 0xE9 || opcode == 0xE8) {
            /* Near JMP/CALL with 32-bit displacement */
            int32_t disp = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
            insn->disp = disp;
            p += 4;
            insn->length = 5;
        }

        return insn->length;
    }

    /* Ultra-fast categorization */
    int category = categorize_ultra_fast(p);

    /* Process prefixes with early termination for simple instructions */
    int has_rep = 0;

    /* Ultra-optimized prefix loop with reduced branching */
    while (1) {
        uint8_t byte = *p;

        /* Ultra-fast path: no more prefixes for simple instructions */
        if (category == 0 && byte < 0x40) break;

        /* Check for REX prefix (0x40-0x4F) */
        if ((byte & 0xF0) == 0x40) {
            if (!rex) {
                rex = byte;
                insn->rex = rex;
                insn->is_64bit = (rex & 0x08) ? 1 : 0;
            }
            p++;
            continue;
        }

        /* Check for legacy prefixes using optimized bitmask check */
        /* Legacy prefixes: 0x66, 0x67, 0x2E, 0x3E, 0x26, 0x36, 0x64, 0x65, 0xF0, 0xF2, 0xF3 */
        if ((byte == 0x66) || (byte == 0xF2) || (byte == 0xF3) ||
            ((byte & 0xF8) == 0x60) || /* 0x60-0x67: segment prefixes */
            (byte == 0xF0)) {

            /* Fast-path handling for common prefixes */
            if (byte == 0x66) {
                insn->is_64bit = 0;
                insn->simd_prefix = 0x66;
            } else if (byte == 0xF2) {
                has_rep = 1;
                insn->simd_prefix = 0xF2;
            } else if (byte == 0xF3) {
                has_rep = 1;
                insn->simd_prefix = 0xF3;
            } else if (byte == 0xF0) {
                insn->has_lock = 1;
            }
            p++;
        } else {
            /* Not a prefix, stop processing */
            break;
        }
    }

    /* Parse REX prefix (0x40-0x4F) */
    /* In 64-bit mode: all bytes from 0x40-0x4F are REX prefixes */
    /* REX prefixes come AFTER legacy prefixes */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Store REP prefix flag in rex field (bit 6) */
    if (has_rep) {
        insn->rex |= 0x40;
    }

    /* Check for VEX prefixes (C4, C5) */
    if (*p == 0xC5 || *p == 0xC4) {
        if (*p == 0xC5) {
            /* VEX.C5 (2-byte VEX prefix) */
            /* Format: [C5][~R~vvvv][L][pp] */
            p++;  /* Skip C5 byte */

            uint8_t vex_byte1 = *p++;

            insn->vex_prefix = 1;  /* Mark as 2-byte VEX */
            insn->vex_vvvv = (~(vex_byte1 >> 3)) & 0x0F;  /* Inverted vvvv (bits 6-3) */
            insn->vex_L = (vex_byte1 >> 2) & 0x01;         /* L field (bit 2) */
            insn->vex_pp = vex_byte1 & 0x03;              /* pp field (bits 1-0) */

            /* Map VEX.pp to simd_prefix using lookup table */
            insn->simd_prefix = vex_pp_to_simd[insn->vex_pp];

            /* VEX.C5 implies implied 0F opcode map */
            insn->vex_m = 0x01;  /* 0F map */
            insn->vex_w = 0;

            /* After VEX.C5, the next byte is the opcode (implied 0F XX) */
            insn->opcode2 = *p++;  /* Read the opcode directly as opcode2 */
            insn->opcode = 0;       /* Primary opcode is 0 for 0F XX */
        } else {
            /* VEX.C4 (3-byte VEX prefix) */
            /* Format: [C4][~R~X~B][mmmmmm][W~vvvv][Lpp][opcode] */
            p++;  /* Skip C4 byte */

            uint8_t vex_byte1 = *p++;
            uint8_t vex_byte2 = *p++;

            insn->vex_prefix = 2;  /* Mark as 3-byte VEX */
            insn->vex_m = vex_byte1 & 0x1F;               /* mmmmmm field (bits 4-0) */
            insn->vex_vvvv = (~(vex_byte2 >> 3)) & 0x0F;  /* Inverted vvvv (bits 6-3) */
            insn->vex_w = (vex_byte2 >> 7) & 0x01;        /* W field (bit 7) */
            insn->vex_L = (vex_byte2 >> 2) & 0x01;        /* L field (bit 2) */
            insn->vex_pp = vex_byte2 & 0x03;             /* pp field (bits 1-0) */

            /* Map VEX.pp to simd_prefix using lookup table */
            insn->simd_prefix = vex_pp_to_simd[insn->vex_pp];

            /* After VEX.C4, read opcode based on map */
            if (insn->vex_m == 0x01) {
                /* 0F map - implied 0F XX opcode */
                insn->opcode2 = *p++;
                insn->opcode = 0;
            } else if (insn->vex_m == 0x02) {
                /* 0F 38 map */
                insn->opcode2 = *p++;  /* Store as opcode2 for now */
                insn->opcode = 0;
            } else if (insn->vex_m == 0x03) {
                /* 0F 3A map */
                insn->opcode2 = *p++;  /* Store as opcode2 for now */
                insn->opcode = 0;
            } else {
                /* Other maps */
                insn->opcode = *p++;
            }
        }

        /* Mark as VEX instruction */
        insn->rex |= 0x80;
    } else {
        /* Not a VEX instruction - normal opcode processing */

        /* Check for 0F escape */
        if (*p == 0x0F) {
            p++;
            uint8_t next_byte = *p;

            /* Check for three-byte escape (0F 38 or 0F 3A) */
            if (next_byte == 0x38 || next_byte == 0x3A) {
                p++;
                insn->opcode2 = next_byte;  /* Store 0x38 or 0x3A */
                insn->opcode3 = *p++;        /* Store the actual opcode */
                insn->opcode = 0;            /* Primary opcode is 0 */
            } else {
                /* Two-byte escape (0F XX) */
                insn->opcode2 = *p++;
                insn->opcode = 0;  /* For 0F XX opcodes, primary opcode is 0 */
                insn->opcode3 = 0;
            }
        } else {
            insn->opcode = *p++;
            insn->opcode2 = 0;
            insn->opcode3 = 0;
        }
    }

    /* Determine if has ModR/M using O(1) lookup table */
    uint8_t op = insn->opcode;
    uint8_t op2 = insn->opcode2;
    uint8_t op3 = insn->opcode3;
    int has_modrm = 0;

    /* Use lookup tables for O(1) ModR/M detection */
    if (op2 == 0) {
        /* Single-byte opcode - use primary lookup table */
        has_modrm = (modrm_lookup[op] == 1);
    } else {
        /* Two-byte opcode (0F XX) - use 0F lookup table */
        uint8_t lookup_val = modrm_0f_lookup[op2];
        /* lookup_val: 1=has_modrm, 2=excluded, 3=special */
        has_modrm = (lookup_val == 1);
    }

    /* Parse ModR/M */
    if (has_modrm) {
        uint8_t modrm = *p++;
        insn->modrm = modrm;
        insn->has_modrm = 1;  /* Mark that ModR/M byte was read */
        insn->mod = (modrm >> 6) & 0x03;
        insn->reg = ((modrm >> 3) & 0x07) | ((rex & 0x04) ? 8 : 0);
        insn->rm = ((modrm >> 0) & 0x07) | ((rex & 0x01) ? 8 : 0);

        /* Handle SIB */
        if (insn->mod != 3 && (insn->rm & 7) == 4) {
            p++;
        }

        /* Handle displacement - optimized with reduced branching */
        uint8_t mod_val = insn->mod;
        if (mod_val == 0) {
            if ((insn->rm & 7) == 5) {
                insn->disp = *(const int32_t *)p;
                p += 4;
            }
        } else if (mod_val == 1) {
            insn->disp = *(const int8_t *)p;
            p += 1;
        } else if (mod_val == 2) {
            insn->disp = *(const int32_t *)p;
            p += 4;
        }
    }

    /* Parse immediate with optimized lookup */
    if (op >= 0xB8 && op <= 0xBF) {
        /* MOV r64, imm64/imm32 */
        if (rex & 0x08) {
            insn->imm = *(const int64_t *)p;
            p += 8;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if ((op & 0x0C) == 0x04 && (op <= 0x3C)) {
        /* AL/EAX immediate operations (0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C) */
        /* Exclude 0x84-0x87 (TEST/XCHG with ModR/M) and higher opcodes */
        insn->rm = 0;
        /* Check if odd opcode = 32-bit immediate, even = 8-bit immediate */
        if (op & 0x01) {
            insn->imm = *(const int32_t *)p;
            p += 4;
        } else {
            insn->imm = *(const int8_t *)p;
            p += 1;
        }
    } else if (op == 0xA9) {
        /* TEST EAX, imm32 */
        insn->rm = 0;
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if ((op >= 0x80 && op <= 0x83) ||
               (op2 >= 0x80 && op2 <= 0x8F)) {
        /* Group immediate operations - optimized check */
        int is_imm8 = (op == 0x83 || op == 0x82 || (op2 >= 0x80));
        if (is_imm8) {
            insn->imm = *(const int8_t *)p;
            p += 1;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op == 0xC0 || op == 0xC1) {
        /* Group 2 shifts/rotates with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op == 0xC2 || op == 0xCA) {
        /* RET with imm16 */
        insn->imm = *(const uint16_t *)p;
        p += 2;
    } else if (op == 0xCD) {
        /* INT imm8 */
        insn->imm = *(const uint8_t *)p;
        p += 1;
    } else if (op == 0x69 || op == 0x6B) {
        /* IMUL r/m, imm32 or imm16 */
        if (op == 0x6B) {
            insn->imm = *(const int8_t *)p;
            p += 1;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op == 0x6A || op == 0x68) {
        /* PUSH imm8 or imm32 */
        if (op == 0x6A) {
            insn->imm = *(const int8_t *)p;
            p += 1;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op >= 0xB0 && op <= 0xB7) {
        /* MOV r8, imm8 - 8-bit immediate move */
        if (!(rex & 0x08)) {  /* Not 64-bit move */
            insn->imm = *(const int8_t *)p;
            p += 1;
        }
    } else if (op == 0xE4 || op == 0xE5 || op == 0xE6 || op == 0xE7) {
        /* IN/OUT with immediate port */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op == 0xE8 || op == 0xE9) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if (op == 0xEB) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op >= 0x70 && op <= 0x7F) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op == 0xE0 || op == 0xE1 || op == 0xE2 || op == 0xE3) {
        /* LOOP/LOOPE/LOOPNZ/JECXZ/JRCXZ with rel8 */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 >= 0x80 && op2 <= 0x8F) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if (op2 == 0xBA) {
        /* BT/BTS/BTR/BTC with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op3 != 0) {
        /* Three-byte escape instructions (0F 38 or 0F 3A) */
        /* Many of these have immediates */
        if (op2 == 0x3A) {
            /* 0F 3A XX opcodes often have immediates */
            insn->imm = *(const int8_t *)p;
            p += 1;
        }
        /* 0F 38 XX opcodes typically don't have immediates (except a few special cases) */
    } else if (op2 == 0xC2) {
        /* CMPPS/CMPPD/CMPSS/CMPSD with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 == 0x70) {
        /* PSHUFD/PSHUFHW/PSHUFLW with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 == 0xC5) {
        /* PEXTRW with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 == 0xC4) {
        /* PINSRW with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 == 0x0D) {
        /* PBLENDW with immediate (SSE4.1) */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 == 0xC6) {
        /* SHUFPS/SHUFPD with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op == 0xD4) {
        /* AAM with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op == 0xD5) {
        /* AAD with immediate */
        insn->imm = *(const int8_t *)p;
        p += 1;
    }

    insn->length = (uint8_t)(p - insn_ptr);
    return insn->length;
}

/* ============================================================================
 * Cached Instruction Decoding
 * ============================================================================ */

/**
 * Decode x86_64 instruction with caching
 *
 * This function provides a cached version of decode_x86_insn that
 * can significantly improve performance for repeated instruction decoding.
 */
int decode_x86_insn_cached(const uint8_t *insn_ptr, x86_insn_t *insn)
{
    /* Check if instruction cache is available */
    if (!insn_cache_is_enabled()) {
        /* Cache not enabled, fall back to normal decode */
        return decode_x86_insn(insn_ptr, insn);
    }

    /* Try cache lookup first */
    u64 guest_pc = (u64)insn_ptr;

    if (insn_cache_lookup(guest_pc, insn)) {
        /* Cache hit - return cached instruction */
        return insn->length;
    }

    /* Cache miss - decode normally and insert into cache */
    int length = decode_x86_insn(insn_ptr, insn);

    if (length > 0) {
        /* Insert decoded instruction into cache */
        insn_cache_insert(guest_pc, insn);
    }

    return length;
}

/* End of rosetta_x86_decode.c */
