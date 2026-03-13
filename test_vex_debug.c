/* VEX prefix decoder debug test */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
} vex_test_t;

int main() {
    vex_test_t tests[] = {
        { "VEX.C5 VMULPS", {0xC5, 0xF8, 0x59, 0xC1}, 4 },
        { "VEX.C4 VBLEND", {0xC4, 0xE1, 0x10, 0xC1}, 4 },
    };

    for (int i = 0; i < 2; i++) {
        vex_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        printf("Testing: %s\n", t->name);
        printf("Bytes: ");
        for (int j = 0; j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\n");

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("Decoded length: %d (expected %d)\n", decoded, t->length);
        printf("VEX prefix: %d\n", insn.vex_prefix);
        printf("VEX.L: %d\n", insn.vex_L);
        printf("VEX.pp: %d\n", insn.vex_pp);
        printf("VEX.m: %d\n", insn.vex_m);
        printf("VEX.w: %d\n", insn.vex_w);
        printf("VEX.vvvv: %d\n", insn.vex_vvvv);
        printf("SIMD prefix: 0x%02X\n", insn.simd_prefix);
        printf("opcode: 0x%02X\n", insn.opcode);
        printf("opcode2: 0x%02X\n", insn.opcode2);
        printf("has_modrm: %d\n", insn.has_modrm);
        if (insn.has_modrm) {
            printf("modrm: 0x%02X\n", insn.modrm);
            printf("mod: %d, reg: %d, rm: %d\n", insn.mod, insn.reg, insn.rm);
        }
        printf("\n");
    }

    return 0;
}
