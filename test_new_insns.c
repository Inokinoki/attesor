/* Test new x86_64 instruction support */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_translate_dispatch.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    int should_translate;
} test_insn_t;

int main() {
    int passed = 0, failed = 0;

    test_insn_t tests[] = {
        /* 8-bit ALU operations */
        { "ADD AL, BL", {0x00, 0xD8}, 2, 1 },
        { "SUB BL, CL", {0x28, 0xD9}, 2, 1 },
        { "AND CL, DL", {0x20, 0xD1}, 2, 1 },
        { "OR DL, AL", {0x08, 0xD0}, 2, 1 },
        { "XOR AL, BL", {0x30, 0xD8}, 2, 1 },

        /* 0xC0 - Group 2 shifts with imm8 */
        { "ROL AL, 2", {0xC0, 0xC0, 0x02}, 3, 1 },
        { "ROR BL, 3", {0xC0, 0xC1, 0x03}, 3, 1 },
        { "SHL CL, 4", {0xC0, 0xE1, 0x04}, 3, 1 },
        { "SHR DL, 5", {0xC0, 0xEA, 0x05}, 3, 1 },

        /* SBB instructions */
        { "SBB RAX, RBX", {0x48, 0x19, 0xD8}, 3, 1 },
        { "SBB AL, BL", {0x18, 0xD8}, 2, 1 },
        { "SBB RAX, 0x10", {0x48, 0x1D, 0x10, 0x00, 0x00, 0x00}, 6, 1 },

        /* HLT */
        { "HLT", {0xF4}, 1, 1 },

        /* MOV r8, imm8 (0xB0-0xB7 range) */
        { "MOV AL, 0x42", {0xB0, 0x42}, 2, 1 },
        { "MOV CL, 0x43", {0xB1, 0x43}, 2, 1 },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d new x86_64 instructions:\n\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        test_insn_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %s\n", i+1, t->name);
        printf("     Opcode: 0x%02X", insn.opcode);
        if (insn.opcode2) {
            printf(" (0F %02X)", insn.opcode2);
        }
        printf("  Length: %d bytes\n", decoded);

        if (decoded != t->length) {
            printf("     ✗ FAIL: Expected length %d, got %d\n", t->length, decoded);
            failed++;
            continue;
        }

        /* Check if instruction can be classified */
        InsnCategory type = dispatch_classify_insn(&insn);
        printf("     Type: %d\n", type);

        if (t->should_translate) {
            printf("     ✓ PASS\n");
            passed++;
        } else {
            printf("     ✗ FAIL: Expected to translate but type is %d\n", type);
            failed++;
        }
        printf("\n");
    }

    printf("==================================================\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("==================================================\n");

    return failed > 0 ? 1 : 0;
}
