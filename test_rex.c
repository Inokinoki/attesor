/* Test REX prefix handling */
#include <stdio.h>
#include <stdint.h>
#include "rosetta_x86_decode.h"

int main() {
    uint8_t insn_bytes[] = {0x48, 0x19, 0xD8};  /* SBB RAX, RBX with REX.W */
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    int len = decode_x86_insn(insn_bytes, &insn);

    printf("Instruction: SBB RAX, RBX\n");
    printf("Bytes: %02X %02X %02X\n", insn_bytes[0], insn_bytes[1], insn_bytes[2]);
    printf("Decoded length: %d\n", len);
    printf("REX prefix: 0x%02X\n", insn.rex);
    printf("Opcode: 0x%02X\n", insn.opcode);
    printf("ModRM: 0x%02X\n", insn.modrm);
    printf("is_64bit: %d\n", insn.is_64bit);

    return 0;
}
