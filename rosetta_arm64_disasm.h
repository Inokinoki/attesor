/* ============================================================================
 * ARM64 Instruction Disassembler (Simple)
 * ============================================================================
 *
 * This module provides functions to disassemble ARM64 instructions
 * ============================================================================ */

#ifndef ROSETTA_ARM64_DISASM_H
#define ROSETTA_ARM64_DISASM_H

#include <stdint.h>
#include <stdio.h>

/**
 * Disassemble and print an ARM64 instruction
 * @param insn ARM64 instruction (4 bytes)
 * @param addr Address of instruction (for printing)
 */
static inline void print_arm64_insn(uint32_t insn, uint64_t addr)
{
    printf("  [%04lx] %08x  ", addr & 0xFFFF, insn);

    /* Check common instructions */
    switch (insn >> 24) {
        case 0xD6:
            if ((insn & 0xFFFFFC00) == 0xD65F0000) {
                printf("RET\n");
                return;
            }
            break;
        case 0x1F:
            if (insn == 0x1F2003D5) {
                printf("NOP\n");
                return;
            }
            break;
        case 0xD2:
            if ((insn & 0xFFE00000) == 0xD2800000) {
                /* MOVZ */
                uint8_t rd = insn & 0x1F;
                uint16_t imm = (insn >> 5) & 0xFFFF;
                uint8_t shift = (insn >> 21) & 0x3;
                printf("MOVZ X%d, #0x%x LSL #%d\n", rd, imm, shift * 16);
                return;
            }
            break;
        case 0xF2:
            if ((insn & 0xFF800000) == 0xF2800000) {
                /* MOVK */
                uint8_t rd = insn & 0x1F;
                uint16_t imm = (insn >> 5) & 0xFFFF;
                uint8_t shift = (insn >> 21) & 0x3;
                printf("MOVK X%d, #0x%x LSL #%d\n", rd, imm, shift * 16);
                return;
            }
            break;
        case 0xF9:
            /* LDR/STR register pair */
            if ((insn & 0x3FC00000) == 0xF9400000) {
                /* LDR 64-bit */
                uint8_t rt = insn & 0x1F;
                uint8_t rn = (insn >> 5) & 0x1F;
                uint16_t offset = ((insn >> 10) & 0xFFF) * 8;
                printf("LDR X%d, [X%d%s%u]\n", rt, rn,
                       offset ? ", #" : "", offset);
                return;
            }
            if ((insn & 0x3FC00000) == 0xF9000000) {
                /* STR 64-bit */
                uint8_t rt = insn & 0x1F;
                uint8_t rn = (insn >> 5) & 0x1F;
                uint16_t offset = ((insn >> 10) & 0xFFF) * 8;
                printf("STR X%d, [X%d%s%u]\n", rt, rn,
                       offset ? ", #" : "", offset);
                return;
            }
            break;
        case 0xAA:
            /* ORR (used for MOV) */
            if ((insn & 0x7FE00000) == 0x0A000000) {
                uint8_t rd = insn & 0x1F;
                uint8_t rn = (insn >> 5) & 0x1F;
                uint8_t rm = (insn >> 16) & 0x1F;
                if (rn == 31) {
                    printf("MOV X%d, X%d\n", rd, rm);
                    return;
                }
                printf("ORR X%d, X%d, X%d\n", rd, rn, rm);
                return;
            }
            break;
        case 0x0B:
            /* ADD (shifted register) */
            if ((insn & 0x7FE00000) == 0x0B000000) {
                uint8_t rd = insn & 0x1F;
                uint8_t rn = (insn >> 5) & 0x1F;
                uint8_t rm = (insn >> 16) & 0x1F;
                printf("ADD X%d, X%d, X%d\n", rd, rn, rm);
                return;
            }
            break;
    }

    printf("UNKNOWN\n");
}

#endif /* ROSETTA_ARM64_DISASM_H */
