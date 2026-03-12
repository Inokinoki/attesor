#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_x86_decode.h"

int main() {
    /* MOVZX EAX, BL = 0F B6 C3 */
    uint8_t insn_bytes[] = {0x0F, 0xB6, 0xC3};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("Bytes: %02X %02X %02X\n", insn_bytes[0], insn_bytes[1], insn_bytes[2]);

    int len = decode_x86_insn(insn_bytes, &insn);

    printf("Decoded length: %d (expected 3)\n", len);
    printf("opcode: 0x%02X\n", insn.opcode);
    printf("opcode2: 0x%02X\n", insn.opcode2);
    printf("has_modrm: %d\n", insn.has_modrm);
    printf("mod: 0x%02X\n", insn.mod);
    printf("reg: 0x%02X\n", insn.reg);
    printf("rm: 0x%02X\n", insn.rm);
    printf("modrm: 0x%02X\n", insn.modrm);

    return 0;
}
