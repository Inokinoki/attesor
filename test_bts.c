#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_x86_decode.h"

int main() {
    /* BTS EAX, EBX = 0F AB C3 */
    uint8_t insn_bytes[] = {0x0F, 0xAB, 0xC3};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("Bytes: %02X %02X %02X\n", insn_bytes[0], insn_bytes[1], insn_bytes[2]);
    printf("Test name: BTS EAX, EBX (should be 3 bytes)\n");

    int len = decode_x86_insn(insn_bytes, &insn);

    printf("Decoded length: %d\n", len);
    printf("opcode: 0x%02X\n", insn.opcode);
    printf("opcode2: 0x%02X\n", insn.opcode2);
    printf("has_modrm: %d\n", insn.has_modrm);
    printf("modrm: 0x%02X\n", insn.modrm);

    return 0;
}
