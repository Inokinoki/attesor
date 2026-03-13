/* Complex instruction pattern tests */
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
    const char *description;
} complex_test_t;

int main() {
    int passed = 0, failed = 0;

    complex_test_t tests[] = {
        /* Complex multi-prefix instructions */
        { "LOCK ADD RAX, RBX", {0xF0, 0x48, 0x01, 0xC3}, 4, "LOCK prefix with REX.W" },
        { "REP MOVSB", {0xF3, 0xA4}, 2, "REP prefix with MOVSB" },
        { "REPE CMPSB", {0xF3, 0xA6}, 2, "REPE prefix with CMPSB" },
        { "REPNE SCASB", {0xF2, 0xAE}, 2, "REPNE prefix with SCASB" },

        /* REX prefix variants */
        { "ADD AL, BL", {0x00, 0xD8}, 2, "ADD 8-bit registers" },
        { "ADD AX, BX", {0x66, 0x01, 0xD8}, 3, "ADD 16-bit with 0x66" },
        { "ADD EAX, EBX", {0x01, 0xD8}, 2, "ADD 32-bit registers" },
        { "ADD RAX, RBX", {0x48, 0x01, 0xD8}, 3, "ADD 64-bit with REX.W" },
        { "ADD R8, R9", {0x4C, 0x01, 0xC1}, 3, "ADD extended registers" },

        /* Complex addressing modes */
        { "MOV EAX, [RBX+RCX*4]", {0x8B, 0x04, 0x8B}, 3, "Scaled indexed addressing" },
        { "MOV EAX, [RBX+RCX*8+0x100]", {0x8B, 0x84, 0xCB, 0x00, 0x01, 0x00, 0x00}, 7, "Scaled indexed with disp32" },
        { "MOV EAX, [RIP+0x100]", {0x8B, 0x05, 0x00, 0x01, 0x00, 0x00}, 6, "RIP-relative addressing" },

        /* Conditional move variants */
        { "CMOVZ EAX, EBX", {0x0F, 0x44, 0xC3}, 3, "Conditional move if zero" },
        { "CMOVNZ EAX, EBX", {0x0F, 0x45, 0xC3}, 3, "Conditional move if not zero" },
        { "CMOVS EAX, EBX", {0x0F, 0x48, 0xC3}, 3, "Conditional move if sign" },
        { "CMOVNS EAX, EBX", {0x0F, 0x49, 0xC3}, 3, "Conditional move if not sign" },

        /* Bit test operations */
        { "BT EAX, 5", {0x0F, 0xBA, 0xE0, 0x05}, 4, "Bit test with immediate" },
        { "BTS EAX, 7", {0x0F, 0xBA, 0xE8, 0x07}, 4, "Bit test and set" },
        { "BTR EAX, 9", {0x0F, 0xBA, 0xF0, 0x09}, 4, "Bit test and reset" },
        { "BTC EAX, 11", {0x0F, 0xBA, 0xF8, 0x0B}, 4, "Bit test and complement" },

        /* Shift/rotate with immediate */
        { "SHL EAX, 5", {0xC1, 0xE0, 0x05}, 3, "Shift left with immediate" },
        { "SHR EAX, 3", {0xC1, 0xE8, 0x03}, 3, "Shift right with immediate" },
        { "SAR EAX, 2", {0xC1, 0xF8, 0x02}, 3, "Shift arithmetic right" },
        { "ROL EAX, 1", {0xC1, 0xC0, 0x01}, 3, "Rotate left" },
        { "ROR EAX, 4", {0xC1, 0xC8, 0x04}, 3, "Rotate right" },

        /* 8-bit variants */
        { "ADD AL, BL", {0x00, 0xD8}, 2, "ADD 8-bit" },
        { "SUB AL, BL", {0x28, 0xD8}, 2, "SUB 8-bit" },
        { "AND AL, BL", {0x20, 0xD8}, 2, "AND 8-bit" },
        { "OR AL, BL", {0x08, 0xD8}, 2, "OR 8-bit" },
        { "XOR AL, BL", {0x30, 0xD8}, 2, "XOR 8-bit" },

        /* Sign extension */
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, "Move zero-extend 8->32" },
        { "MOVSX EAX, BL", {0x0F, 0xBE, 0xC3}, 3, "Move sign-extend 8->32" },
        { "MOVZX EAX, BX", {0x0F, 0xB7, 0xC3}, 3, "Move zero-extend 16->32" },
        { "MOVSX EAX, BX", {0x0F, 0xBF, 0xC3}, 3, "Move sign-extend 16->32" },
        { "MOVSXD RAX, EAX", {0x48, 0x63, 0xC0}, 3, "Move sign-extend 32->64" },

        /* SETcc instructions */
        { "SETZ AL", {0x0F, 0x94, 0xC0}, 3, "Set if zero" },
        { "SETNZ CL", {0x0F, 0x95, 0xC1}, 3, "Set if not zero" },
        { "SETS DL", {0x0F, 0x98, 0xC2}, 3, "Set if sign" },
        { "SETNS BL", {0x0F, 0x99, 0xC3}, 3, "Set if not sign" },

        /* Complex SSE with multiple prefixes */
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, "Packed add 64-bit" },
        { "PSUBQ XMM0, XMM1", {0x66, 0x0F, 0xFB, 0xC1}, 4, "Packed subtract 64-bit" },
        { "PMULUDQ XMM0, XMM1", {0x66, 0x0F, 0xF4, 0xC1}, 4, "Packed multiply 32->64" },

        /* Control flow with immediates */
        { "ADD EAX, 0x12345678", {0x05, 0x78, 0x56, 0x34, 0x12}, 5, "ADD with imm32" },
        { "CMP EAX, 0x12345678", {0x3D, 0x78, 0x56, 0x34, 0x12}, 5, "CMP with imm32" },
        { "SUB EAX, 0x12345678", {0x2D, 0x78, 0x56, 0x34, 0x12}, 5, "SUB with imm32" },

        /* Stack operations */
        { "PUSH RAX", {0x50}, 1, "PUSH 64-bit register" },
        { "PUSH RBX", {0x53}, 1, "PUSH RBX" },
        { "POP RCX", {0x59}, 1, "POP RCX" },
        { "POP RDX", {0x5A}, 1, "POP RDX" },

        /* Exchange operations */
        { "XCHG RAX, RBX", {0x48, 0x93}, 2, "Exchange 64-bit registers" },
        { "XCHG RAX, RCX", {0x48, 0x91}, 2, "Exchange with RAX" },

        /* LEA variants */
        { "LEA EAX, [EBX]", {0x8D, 0x03}, 2, "Load effective address (reg)" },
        { "LEA EAX, [EBX+ECX]", {0x8D, 0x04, 0x0B}, 3, "Load effective address (reg+reg)" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d complex instruction patterns:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        complex_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %s\\n", i+1, t->name);
        printf("     %s\\n", t->description);
        printf("     Bytes: ");
        for (int j = 0; j < t->length; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\\n");
        printf("     Length: %d (expected %d)", decoded, t->length);

        if (decoded != t->length) {
            printf(" ✗ FAIL: Length mismatch\\n");
            failed++;
        } else {
            InsnCategory type = dispatch_classify_insn(&insn);
            printf(" ✓ PASS (Type: %d)\\n", type);
            passed++;
        }
        printf("\\n");
    }

    printf("==================================================\\n");
    printf("Complex Pattern Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
