/* ============================================================================
 * Test: Decode Generated ARM64 Code
 * ============================================================================
 *
 * Decode the 196 bytes of generated code to verify it's pure ARM64
 * ============================================================================ */

#include <stdio.h>
#include <stdint.h>

/* ARM64 instruction decoder */
void print_arm64_insn(uint32_t insn, uint32_t offset)
{
    printf("  [%2u] %08x  ", offset, insn);

    /* Check common ARM64 instructions */
    if ((insn & 0xFFE0001F) == 0xD65F0000) {
        printf("RET\n");
        return;
    }
    if (insn == 0x1F2003D5) {
        printf("NOP\n");
        return;
    }
    if ((insn & 0xFF800000) == 0xD2800000) {
        /* MOVZ */
        uint8_t rd = insn & 0x1F;
        uint16_t imm = (insn >> 5) & 0xFFFF;
        uint8_t shift = (insn >> 21) & 0x3;
        printf("MOVZ X%d, #0x%x LSL #%d\n", rd, imm, shift * 16);
        return;
    }
    if ((insn & 0x7FE00000) == 0x0A000000) {
        /* ORR (shifted register) */
        uint8_t rd = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint8_t rm = (insn >> 16) & 0x1F;
        if (rn == 31) {
            printf("MOV X%d, X%d  (ORR with ZR)\n", rd, rm);
        } else {
            printf("ORR X%d, X%d, X%d\n", rd, rn, rm);
        }
        return;
    }
    if ((insn & 0x7FC00000) == 0xF9400000) {
        /* LDR 64-bit */
        uint8_t rt = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t offset = ((insn >> 10) & 0xFFF) * 8;
        printf("LDR X%d, [X%d%s%u]\n", rt, rn,
               offset ? ", #" : "", offset);
        return;
    }
    if ((insn & 0x7FC00000) == 0xF9000000) {
        /* STR 64-bit */
        uint8_t rt = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t offset = ((insn >> 10) & 0xFFF) * 8;
        printf("STR X%d, [X%d%s%u]\n", rt, rn,
               offset ? ", #" : "", offset);
        return;
    }
    if ((insn & 0xFFC00000) == 0x91000000) {
        /* ADD immediate */
        uint8_t rd = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t imm = (insn >> 10) & 0xFFF;
        printf("ADD X%d, X%d, #%u\n", rd, rn, imm);
        return;
    }
    if ((insn & 0xFFC00000) == 0x51000000) {
        /* SUB immediate */
        uint8_t rd = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t imm = (insn >> 10) & 0xFFF;
        printf("SUB X%d, X%d, #%u\n", rd, rn, imm);
        return;
    }
    if ((insn & 0x7FC00000) == 0xF8000000) {
        /* STR (immediate) unscaled */
        uint8_t rt = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t imm = ((insn >> 10) & 0x1FF) * 8;
        printf("STR X%d, [X%d, #%u]\n", rt, rn, imm);
        return;
    }
    if ((insn & 0x7F800000) == 0xD1000000) {
        /* SUB immediate */
        uint8_t rd = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t imm = (insn >> 10) & 0xFFF;
        printf("SUB X%d, X%d, #%u\n", rd, rn, imm);
        return;
    }
    if ((insn & 0x7F800000) == 0x91000000) {
        /* ADD immediate */
        uint8_t rd = insn & 0x1F;
        uint8_t rn = (insn >> 5) & 0x1F;
        uint16_t imm = (insn >> 10) & 0xFFF;
        printf("ADD X%d, X%d, #%u\n", rd, rn, imm);
        return;
    }

    /* Check for x86_64 instructions */
    if ((insn & 0xFF) == 0x50) {
        printf("❌ x86_64 PUSH!\n");
        return;
    }
    if ((insn & 0xFF) == 0x58) {
        printf("❌ x86_64 POP!\n");
        return;
    }
    if ((insn & 0xFF) == 0xC3) {
        printf("❌ x86_64 RET!\n");
        return;
    }
    if ((insn & 0xFF) == 0x90) {
        printf("❌ x86_64 NOP!\n");
        return;
    }

    printf("UNKNOWN\n");
}

/* Test the 196 bytes that were generated */
int main(void) {
    /* From the translation output */
    unsigned char code[] = {
        0xd5, 0x03, 0x20, 0x1f,  // 0: NOP
        0x05, 0x00, 0x80, 0xd2,  // 4: MOVZ
        0xe2, 0x03, 0x09, 0xaa,  // 8: ?
        0xd5, 0x03, 0x20, 0x1f,  // 12: NOP
        /* Need to capture more bytes from actual run */
    };

    printf("Decoding first 20 bytes:\n");
    for (size_t i = 0; i < 5; i++) {
        uint32_t insn;
        memcpy(&insn, &code[i*4], 4);
        print_arm64_insn(insn, i);
    }

    return 0;
}
