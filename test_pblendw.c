/* Debug PBLENDW instruction */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

int main() {
    uint8_t pblendw[] = {0x66, 0x0F, 0x0D, 0xC2, 0x00};
    x86_insn_t insn;
    memset(&insn, 0, sizeof(insn));

    printf("PBLENDW instruction bytes: ");
    for (int i = 0; i < 5; i++) {
        printf("%02X ", pblendw[i]);
    }
    printf("\n");

    int decoded = decode_x86_insn(pblendw, &insn);
    printf("Decoded length: %d (expected 5)\n", decoded);
    printf("opcode: 0x%02X, opcode2: 0x%02X, opcode3: 0x%02X\n", insn.opcode, insn.opcode2, insn.opcode3);
    printf("simd_prefix: 0x%02X\n", insn.simd_prefix);
    printf("has_modrm: %d\n", insn.has_modrm);
    printf("modrm: 0x%02X\n", insn.modrm);
    printf("imm: %lld\n", insn.imm);

    return 0;
}
