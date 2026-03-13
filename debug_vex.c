#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_x86_decode.h"

int main() {
    /* VEX.128.F3.0F.WIG 10 /r = VEX.128.F3.0F.WIG 59 /r */
    /* Should be: C5 F4 59 C1 (4 bytes) but test has C4 E2 10 C2 (from test) */
    uint8_t insn_bytes[] = {0xC5, 0xF4, 0x59, 0xC1};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("Bytes: %02X %02X %02X %02X\n", insn_bytes[0], insn_bytes[1], insn_bytes[2], insn_bytes[3]);
    printf("Expected: VEX.128.F3.0F.WIG 59 /r (4 bytes)\n");

    int len = decode_x86_insn(insn_bytes, &insn);

    printf("Decoded length: %d\n", len);
    printf("opcode: 0x%02X\n", insn.opcode);
    printf("opcode2: 0x%02X\n", insn.opcode2);
    printf("rex: 0x%02X\n", insn.rex);

    return 0;
}
