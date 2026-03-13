/* Test decoder fix for CPUID, RDTSC, SYSENTER */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
} decoder_test_t;

int main() {
    int passed = 0, failed = 0;

    decoder_test_t tests[] = {
        { "CPUID", {0x0F, 0xA2}, 2 },
        { "RDTSC", {0x0F, 0x31}, 2 },
        { "SYSENTER", {0x0F, 0x34}, 2 },
        { "SYSCALL", {0x0F, 0x05}, 2 },
        { "MOV EAX, EBX", {0x8B, 0xC3}, 2 },  /* Fixed test - no extra byte */
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing decoder fixes:\\n\\n");

    for (int i = 0; i < num_tests; i++) {
        decoder_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %s\\n", i+1, t->name);
        printf("     Bytes: ");
        for (int j = 0; j < t->length; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\\n");
        printf("     Length: %d (expected %d)\\n", decoded, t->length);
        printf("     has_modrm: %d\\n", insn.has_modrm);

        if (decoded != t->length) {
            printf("     ✗ FAIL: Length mismatch\\n\\n");
            failed++;
        } else {
            printf("     ✓ PASS\\n\\n");
            passed++;
        }
    }

    printf("==================================================\\n");
    printf("Decoder Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
