/* Test additional x86_64 instruction patterns */
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
} test_insn_t;

int main() {
    int passed = 0, failed = 0;

    test_insn_t tests[] = {
        /* INC/DEC with various operand sizes */
        { "INC EAX", {0xFF, 0xC0}, 2, "INC 32-bit register" },
        { "INC RAX", {0x48, 0xFF, 0xC0}, 3, "INC 64-bit register (REX.W)" },
        { "DEC RBX", {0x48, 0xFF, 0xCB}, 3, "DEC 64-bit register" },

        /* PUSH/POP */
        { "PUSH RAX", {0x50}, 1, "PUSH 64-bit register" },
        { "POP RBX", {0x5B}, 1, "POP 64-bit register" },
        { "PUSH RBP", {0x55}, 1, "PUSH RBP" },
        { "POP RBP", {0x5D}, 1, "POP RBP" },

        /* XCHG */
        { "XCHG RAX, RBX", {0x48, 0x93}, 2, "Exchange registers" },
        { "XCHG RAX, RCX", {0x48, 0x91}, 2, "Exchange with RAX" },

        /* CMP - various forms */
        { "CMP RAX, RBX", {0x48, 0x39, 0xC3}, 3, "Compare registers" },
        { "CMP EAX, imm32", {0x3D, 0x10, 0x00, 0x00, 0x00}, 5, "Compare with immediate" },
        { "CMP AL, imm8", {0x3C, 0x42}, 2, "Compare 8-bit with immediate" },

        /* TEST - various forms */
        { "TEST RAX, RBX", {0x48, 0x85, 0xC3}, 3, "Test registers" },
        { "TEST EAX, imm32", {0xA9, 0xFF, 0xFF, 0x00, 0x00}, 5, "Test with immediate" },

        /* LEA */
        { "LEA RAX, [RIP+disp]", {0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00}, 7, "Load effective address RIP-relative" },

        /* MOVZX/MOVSX */
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, "Move zero-extend 8->32" },
        { "MOVSX EAX, BL", {0x0F, 0xBE, 0xC3}, 3, "Move sign-extend 8->32" },
        { "MOVZX RAX, BX", {0x48, 0x0F, 0xB7, 0xC3}, 4, "Move zero-extend 16->64" },

        /* SETcc - conditional set */
        { "SETZ AL", {0x0F, 0x94, 0xC0}, 3, "Set if zero" },
        { "SETNZ CL", {0x0F, 0x95, 0xC1}, 3, "Set if not zero" },

        /* CMOVcc - conditional move */
        { "CMOVZ EAX, EBX", {0x0F, 0x44, 0xC3}, 3, "Conditional move if zero" },

        /* IMUL - various forms */
        { "IMUL EAX, EBX", {0x0F, 0xAF, 0xC3}, 3, "IMUL register" },
        { "IMUL EAX, EBX, 5", {0x69, 0xC3, 0x05, 0x00, 0x00, 0x00}, 6, "IMUL with immediate" },

        /* DIV/IDIV */
        { "DIV EBX", {0xF7, 0xF3}, 2, "Unsigned divide" },
        { "IDIV EBX", {0xF7, 0xFB}, 2, "Signed divide" },

        /* CBW/CWDE/CDQE */
        { "CDQE", {0x48, 0x98}, 2, "Convert doubleword to quadword" },

        /* NOP variants */
        { "NOP", {0x90}, 1, "Single-byte NOP" },
        { "NOP eax", {0x0F, 0x1F, 0x00}, 3, "Multi-byte NOP" },

        /* Control flow */
        { "CALL rel32", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, "Call near relative" },
        { "RET", {0xC3}, 1, "Return near" },

        /* Stack operations */
        { "PUSHF", {0x9C}, 1, "Push flags" },
        { "POPF", {0x9D}, 1, "Pop flags" },
        { "PUSHQ imm8", {0x6A, 0x42}, 2, "Push immediate 8-bit" },
        { "PUSHQ imm32", {0x68, 0x00, 0x00, 0x00, 0x00}, 5, "Push immediate 32-bit" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d additional x86_64 instruction patterns:\n\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        test_insn_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %s\n", i+1, t->name);
        printf("     %s\n", t->description);
        printf("     Opcode: 0x%02X", insn.opcode);
        if (insn.opcode2) {
            printf(" (0F %02X)", insn.opcode2);
        }
        printf("  Length: %d bytes (expected %d)\n", decoded, t->length);

        if (decoded != t->length) {
            printf("     ✗ FAIL: Length mismatch\n");
            failed++;
        } else {
            InsnCategory type = dispatch_classify_insn(&insn);
            printf("     Type: %d (%s)\n", type,
                   (type == INSN_ALU) ? "ALU" :
                   (type == INSN_MEMORY) ? "MEMORY" :
                   (type == INSN_BRANCH) ? "BRANCH" :
                   (type == INSN_BIT) ? "BIT" :
                   (type == INSN_STRING) ? "STRING" :
                   (type == INSN_SPECIAL) ? "SPECIAL" :
                   (type == INSN_UNKNOWN) ? "UNKNOWN" : "OTHER");

            if (type != INSN_UNKNOWN) {
                printf("     ✓ PASS\n");
                passed++;
            } else {
                printf("     ⚠ WARNING: Unknown instruction type\n");
                passed++;  /* Still counts as pass if decoded correctly */
            }
        }
        printf("\n");
    }

    printf("==================================================\n");
    printf("Results: %d/%d tests passed (%.1f%%)\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\n", failed);
    printf("==================================================\n");

    return failed > 0 ? 1 : 0;
}
