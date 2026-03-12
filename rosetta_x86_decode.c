/* ============================================================================
 * Rosetta x86_64 Instruction Decoder
 * ============================================================================
 *
 * This module handles decoding of x86_64 instructions into a structured
 * format that can be used for translation to ARM64.
 * ============================================================================ */

#include "rosetta_x86_decode.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * Instruction Decoder Implementation
 * ============================================================================ */

/**
 * Decode x86_64 instruction at given address
 * @param insn_ptr Pointer to instruction bytes
 * @param insn Output: decoded instruction info
 * @return Number of bytes decoded
 */
int decode_x86_insn(const uint8_t *insn_ptr, x86_insn_t *insn)
{
    const uint8_t *p = insn_ptr;
    uint8_t rex = 0;

    memset(insn, 0, sizeof(x86_insn_t));

    /* Parse REX prefix (0x40-0x4F) */
    /* In 64-bit mode: all bytes from 0x40-0x4F are REX prefixes */
    if ((p[0] & 0xF0) == 0x40) {
        rex = p[0];
        p++;
        insn->rex = rex;
        insn->is_64bit = (rex & 0x08) ? 1 : 0;
    } else {
        insn->is_64bit = 1;
    }

    /* Check for legacy prefixes (simplified) */
    int has_rep = 0;
    while (*p == 0x66 || *p == 0x67 || *p == 0x2E || *p == 0x3E ||
           *p == 0x26 || *p == 0x36 || *p == 0x64 || *p == 0x65 ||
           *p == 0xF0 || *p == 0xF2 || *p == 0xF3) {
        if (*p == 0x66) {
            insn->is_64bit = 0;
            insn->simd_prefix = 0x66;  /* Operand-size override / SSE2 */
        }
        if (*p == 0xF2) {
            has_rep = 1;  /* REPNE prefix / SSE2 */
            insn->simd_prefix = 0xF2;
        }
        if (*p == 0xF3) {
            has_rep = 1;  /* REP prefix / SSE2 */
            insn->simd_prefix = 0xF3;
        }
        p++;
    }

    /* Store REP prefix flag in rex field (bit 6) */
    if (has_rep) {
        insn->rex |= 0x40;
    }

    /* Check for 0F escape */
    if (*p == 0x0F) {
        p++;
        insn->opcode2 = *p++;
        insn->opcode = 0;  /* For 0F XX opcodes, primary opcode is 0 */
    } else {
        insn->opcode = *p++;
    }

    /* Determine if has ModR/M */
    uint8_t op = insn->opcode;
    uint8_t op2 = insn->opcode2;
    int has_modrm = 0;

    if (op == 0x00 || op == 0x01 || op == 0x02 || op == 0x03 ||
        op == 0x08 || op == 0x09 || op == 0x0A || op == 0x0B ||
        op == 0x18 || op == 0x19 || op == 0x1A || op == 0x1B ||
        op == 0x20 || op == 0x21 || op == 0x22 || op == 0x23 ||
        op == 0x28 || op == 0x29 || op == 0x2A || op == 0x2B ||
        op == 0x30 || op == 0x31 || op == 0x32 || op == 0x33 ||
        op == 0x38 || op == 0x39 || op == 0x3A || op == 0x3B ||
        op == 0x84 || op == 0x85 || op == 0x86 || op == 0x87 ||
        op == 0x88 || op == 0x89 || op == 0x8A || op == 0x8B ||
        op == 0x8C || op == 0x8D || op == 0x8E || op == 0x8F ||
        op == 0x69 || op == 0x6B || /* IMUL with immediate */
        op == 0xC0 || op == 0xC1 ||
        op == 0xD0 || op == 0xD1 || op == 0xD2 || op == 0xD3 ||
        op == 0xF6 || op == 0xF7 || op == 0xFF ||
        op == 0x80 || op == 0x81 || op == 0x82 || op == 0x83) {
        has_modrm = 1;
    }
    if (op2 != 0 && (
        (op2 >= 0x00 && op2 <= 0x07) ||
        (op2 >= 0x10 && op2 <= 0x17) ||
        (op2 >= 0x20 && op2 <= 0x27) ||
        (op2 >= 0x40 && op2 <= 0x4F) ||
        (op2 >= 0x50 && op2 <= 0x7F) ||
        (op2 >= 0x80 && op2 <= 0x8F) ||
        (op2 >= 0x90 && op2 <= 0x9F) ||
        op2 == 0x1F ||
        op2 == 0xA0 || op2 == 0xA1 || op2 == 0xA2 || op2 == 0xA3 ||
        op2 == 0xB0 || op2 == 0xB1 || op2 == 0xB3 ||
        op2 == 0xB6 || op2 == 0xB7 || op2 == 0xBE || op2 == 0xBF ||
        op2 == 0xC0 || op2 == 0xC1)) {
        has_modrm = 1;
    }

    insn->has_modrm = has_modrm;

    /* Parse ModR/M */
    if (has_modrm) {
        uint8_t modrm = *p++;
        insn->modrm = modrm;
        insn->mod = (modrm >> 6) & 0x03;
        insn->reg = ((modrm >> 3) & 0x07) | ((rex & 0x04) ? 8 : 0);
        insn->rm = ((modrm >> 0) & 0x07) | ((rex & 0x01) ? 8 : 0);

        /* Handle SIB */
        if (insn->mod != 3 && (insn->rm & 7) == 4) {
            p++;
        }

        /* Handle displacement */
        if (insn->mod == 0 && (insn->rm & 7) == 5) {
            insn->disp = *(const int32_t *)p;
            p += 4;
        } else if (insn->mod == 1) {
            insn->disp = *(const int8_t *)p;
            p += 1;
        } else if (insn->mod == 2) {
            insn->disp = *(const int32_t *)p;
            p += 4;
        }
    }

    /* Parse immediate */
    if (op >= 0xB8 && op <= 0xBF) {
        if (rex & 0x08) {
            insn->imm = *(const int64_t *)p;
            p += 8;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if (op == 0x05 || op == 0x0D || op == 0x0F || op == 0x15 ||
               op == 0x1D || op == 0x25 || op == 0x2D || op == 0x2F ||
               op == 0x35 || op == 0x3C || op == 0x3D || op == 0xA9) {
        insn->rm = 0;
        if (op == 0x3C) {
            /* CMP AL, imm8 */
            insn->imm = *(const int8_t *)p;
            p += 1;
        } else {
            insn->imm = *(const int32_t *)p;
            p += 4;
        }
    } else if ((op >= 0x80 && op <= 0x83) ||
               (op2 >= 0x80 && op2 <= 0x8F)) {
        int is_imm8 = (op == 0x83 || op == 0x82 || op2 >= 0x80);
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
    } else if (op == 0xE8 || op == 0xE9) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    } else if (op == 0xEB) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op >= 0x70 && op <= 0x7F) {
        insn->imm = *(const int8_t *)p;
        p += 1;
    } else if (op2 >= 0x80 && op2 <= 0x8F) {
        insn->imm = *(const int32_t *)p;
        p += 4;
    }

    insn->length = (uint8_t)(p - insn_ptr);
    return insn->length;
}

/* End of rosetta_x86_decode.c */
