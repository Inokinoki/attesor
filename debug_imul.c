#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_x86_decode.h"

int main() {
    /* IMUL EAX, EBX, 5 = 69 C3 05 00 00 00 */
    uint8_t insn_bytes[] = {0x69, 0xC3, 0x05, 0x00, 0x00, 0x00};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("Bytes: %02X %02X %02X %02X %02X %02X\n",
           insn_bytes[0], insn_bytes[1], insn_bytes[2],
           insn_bytes[3], insn_bytes[4], insn_bytes[5]);

    int len = decode_x86_insn(insn_bytes, &insn);

    printf("Decoded length: %d (expected 6)\n", len);
    printf("opcode: 0x%02X\n", insn.opcode);
    printf("has_modrm: %d\n", insn.has_modrm);
    printf("modrm: 0x%02X\n", insn.modrm);
    printf("imm: %lld (0x%llX)\n", (long long)insn.imm, (long long)insn.imm);

    return 0;
}
