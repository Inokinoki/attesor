/* Debug REX prefix issue */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

void debug_bytes(const char *name, uint8_t *bytes, int length) {
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("%s\\n", name);
    printf("  Bytes: ");
    for (int j = 0; j < length; j++) {
        printf("%02X ", bytes[j]);
    }
    printf("\\n");

    int decoded = decode_x86_insn(bytes, &insn);
    printf("  Decoded: %d bytes\\n", decoded);
    printf("  opcode: 0x%02X, opcode2: 0x%02X, opcode3: 0x%02X\\n", insn.opcode, insn.opcode2, insn.opcode3);
    printf("  rex: 0x%02X, is_64bit: %d\\n", insn.rex, insn.is_64bit);
    printf("  has_modrm: %d, modrm: 0x%02X\\n", insn.has_modrm, insn.modrm);
    printf("  mod: %d, reg: %d, rm: %d\\n\\n", insn.mod, insn.reg, insn.rm);
}

int main() {
    printf("Debugging REX prefix and instruction sequences:\\n\\n");

    /* Single byte instructions */
    debug_bytes("Single byte INC (0x40)", (uint8_t[]){0x40}, 1);
    debug_bytes("Single byte DEC (0x48)", (uint8_t[]){0x48}, 1);
    debug_bytes("INC RAX (REX.W INC)", (uint8_t[]){0x48, 0xFF, 0xC0}, 3);
    debug_bytes("DEC RAX (REX.W DEC)", (uint8_t[]){0x48, 0xFF, 0xC8}, 3);

    /* Actual 64-bit INC/DEC */
    debug_bytes("INC RAX (FF C0)", (uint8_t[]){0xFF, 0xC0}, 2);
    debug_bytes("DEC RAX (FF C8)", (uint8_t[]){0xFF, 0xC8}, 2);
    debug_bytes("INC R8 (49 FF C0)", (uint8_t[]){0x49, 0xFF, 0xC0}, 3);

    /* Instruction sequences */
    debug_bytes("GCC prologue (4 bytes)", (uint8_t[]){0x55, 0x48, 0x89, 0xE5}, 4);
    debug_bytes("GCC epilogue (2 bytes)", (uint8_t[]){0x5D, 0xC3}, 2);

    return 0;
}