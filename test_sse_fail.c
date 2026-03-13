/* Debug failing SSE tests */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
} sse_test_t;

int main() {
    sse_test_t tests[] = {
        { "MOVUPS XMM0, XMM1", {0x0F, 0x10, 0xC1}, 3 },
        { "MOVUPD XMM0, XMM1", {0x0F, 0x11, 0xC1}, 3 },
        { "COMISS XMM0, XMM1", {0x0F, 0x2F, 0xC1}, 3 },
        { "UCOMISS XMM0, XMM1", {0x0F, 0x2E, 0xC1}, 3 },
        { "MOVDQA XMM0, XMM1", {0x0F, 0x6F, 0xC1}, 3 },
        { "PSUBQ XMM0, XMM1", {0x66, 0x0F, 0xFB, 0xC1}, 4 },
    };

    for (int i = 0; i < 6; i++) {
        sse_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        printf("[%d] %s\n", i+1, t->name);
        printf("Bytes: ");
        for (int j = 0; j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\n");

        int decoded = decode_x86_insn(t->bytes, &insn);
        printf("Decoded length: %d (expected %d)\n", decoded, t->length);
        printf("opcode: 0x%02X, opcode2: 0x%02X\n", insn.opcode, insn.opcode2);
        printf("simd_prefix: 0x%02X\n", insn.simd_prefix);
        printf("has_modrm: %d\n", insn.has_modrm);
        printf("\n");
    }

    return 0;
}
