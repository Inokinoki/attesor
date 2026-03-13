/* Edge case decoder tests */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    const char *description;
} edge_case_t;

int main() {
    int passed = 0, failed = 0;

    edge_case_t tests[] = {
        /* Edge case: Maximum instruction length */
        { "Max length instruction", {0x48, 0x81, 0x84, 0x93, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}, 12, "Complex addressing with imm32" },

        /* Edge case: Single-byte instructions with special meanings */
        { "NOP as XCHG", {0x90}, 1, "NOP = XCHG EAX, EAX" },
        { "HLT", {0xF4}, 1, "Halt instruction" },
        { "RDMSR", {0x0F, 0x32}, 2, "Read Model-Specific Register" },
        { "WRMSR", {0x0F, 0x30}, 2, "Write Model-Specific Register" },

        /* Edge case: REX prefix effects */
        { "REX.W prefix", {0x48, 0x01, 0xC0}, 3, "ADD RAX, RAX with 64-bit" },
        { "REX.R prefix", {0x44, 0x01, 0xC0}, 3, "ADD RAX, R8" },
        { "REX.X prefix", {0x42, 0x8B, 0x04, 0x10}, 4, "MOV RAX, [RAX+RDX*1]" },
        { "REX.B prefix", {0x41, 0x50}, 2, "PUSH R8" },
        { "REX.WRXB", {0x4F, 0x01, 0x04, 0x10}, 4, "ADD [RAX+R9*8], R15" },

        /* Edge case: Multiple prefixes */
        { "LOCK + REX.W", {0xF0, 0x48, 0x01, 0x07}, 4, "LOCK ADD [RDI], RAX" },
        { "Data16 + REX.W", {0x66, 0x48, 0x01, 0x07}, 4, "ADD [RDI], AX with REX.W" },
        { "REP + REX.W", {0xF3, 0x48, 0xA5}, 3, "REP MOVSQ" },

        /* Edge case: Boundary conditions */
        { "JMP short -128", {0xEB, 0x80}, 2, "Jump with rel8=-128" },
        { "JMP short +127", {0xEB, 0x7F}, 2, "Jump with rel8=127" },
        { "LOOP -1", {0xE2, 0xFF}, 2, "LOOP with rel8=-1" },

        /* Edge case: Special ModR/M encodings */
        { "ModR/M mod=00 rm=101", {0x8B, 0x05, 0x00, 0x00, 0x00, 0x00}, 6, "MOV EAX, [RIP+disp32]" },
        { "ModR/M mod=00 rm=100", {0x8B, 0x04, 0x24}, 3, "MOV EAX, [RSP]" },
        { "ModR/M no displacement", {0x8B, 0x00}, 2, "MOV EAX, [RAX]" },
        { "ModR/M disp8 only", {0x8B, 0x40, 0x10}, 3, "MOV EAX, [RAX+0x10]" },
        { "ModR/M disp32", {0x8B, 0x80, 0x00, 0x01, 0x00, 0x00}, 6, "MOV EAX, [RAX+0x100]" },

        /* Edge case: VEX prefix boundaries */
        { "VEX.128", {0xC5, 0xF0, 0x58, 0xC2}, 4, "VADDPS with VEX.128" },
        { "VEX.256", {0xC5, 0x7D, 0x58, 0xC2}, 4, "VADDPS with VEX.256" },
        { "VEX.WIG", {0xC4, 0xE1, 0x79, 0x60, 0xC0}, 5, "VPUNPCKLBW with VEX.WIG" },

        /* Edge case: Three-byte escape sequences */
        { "0F 38 XX", {0x0F, 0x38, 0xDC, 0xC1}, 4, "AESENC (0F 38 XX)" },
        { "0F 3A XX", {0x0F, 0x3A, 0x0D, 0xC1, 0x00}, 5, "BLENDPD with imm (0F 3A XX)" },

        /* Edge case: Undocumented/special instructions */
        { "UD2", {0x0F, 0x0B}, 2, "Undefined instruction" },
        { "ICEBP", {0xF1}, 1, "ICE breakpoint (undocumented)" },

        /* Edge case: Conditional move boundaries */
        { "CMOVO", {0x48, 0x0F, 0x40, 0xC3}, 4, "CMOV overflow" },
        { "CMOVNO", {0x48, 0x0F, 0x41, 0xC3}, 4, "CMOV not overflow" },
        { "CMOVB", {0x48, 0x0F, 0x42, 0xC3}, 4, "CMOV below" },
        { "CMOVNB", {0x48, 0x0F, 0x43, 0xC3}, 4, "CMOV not below" },

        /* Edge case: String operations with address size */
        { "MOVSD with 0x66", {0x66, 0xA5}, 2, "MOVSW (operand size override)" },
        { "MOVSB with 0x67", {0x67, 0xA4}, 2, "MOVSB with address size override" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d edge cases:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        edge_case_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %-25s ", i+1, t->name);
        for (int j = 0; j < t->length && j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        if (t->length > 8) {
            printf("... ");
        }
        printf("\\n");

        printf("     %s\\n", t->description);
        printf("     Length: %d (expected %d)", decoded, t->length);

        if (decoded != t->length) {
            printf(" ✗ FAIL: Length mismatch\\n\\n");
            failed++;
        } else {
            printf(" ✓ PASS\\n\\n");
            passed++;
        }
    }

    printf("==================================================\\n");
    printf("Edge Case Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
