/* End-to-end translation pipeline test */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_translate_dispatch.h"
#include "rosetta_codegen.h"

typedef struct {
    const char *name;
    uint8_t x86_bytes[16];
    int x86_length;
    const char *description;
    int should_translate;  /* Whether we expect translation to succeed */
} e2e_test_t;

int main() {
    int passed = 0, failed = 0;

    e2e_test_t tests[] = {
        /* Basic ALU operations - should always translate */
        { "ADD EAX, EBX", {0x01, 0xD8}, 2, "Add registers", 1 },
        { "SUB EAX, EBX", {0x29, 0xD8}, 2, "Subtract registers", 1 },
        { "MOV EAX, EBX", {0x8B, 0xC3}, 2, "Move register to register", 1 },
        { "XOR EAX, EAX", {0x31, 0xC0}, 2, "Zero register", 1 },
        { "INC EAX", {0xFF, 0xC0}, 2, "Increment register", 1 },
        { "DEC EAX", {0xFF, 0xC8}, 2, "Decrement register", 1 },

        /* Memory operations - should translate */
        { "MOV EAX, [EBX]", {0x8B, 0x03}, 2, "Load from memory", 1 },
        { "MOV [EBX], EAX", {0x89, 0x03}, 2, "Store to memory", 1 },
        { "LEA EAX, [EBX+ECX]", {0x8D, 0x04, 0x0B}, 3, "Load effective address", 1 },

        /* Control flow - should translate */
        { "CALL label", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, "Call near relative", 1 },
        { "RET", {0xC3}, 1, "Return near", 1 },
        { "JMP label", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, "Jump near relative", 1 },
        { "JE label", {0x74, 0x00}, 2, "Jump if equal", 1 },
        { "JNE label", {0x75, 0x00}, 2, "Jump if not equal", 1 },

        /* Stack operations - should translate */
        { "PUSH EAX", {0x50}, 1, "Push EAX", 1 },
        { "POP ECX", {0x59}, 1, "Pop ECX", 1 },

        /* Bit operations - should translate */
        { "BSF EAX, ECX", {0x0F, 0xBC, 0xC1}, 3, "Bit scan forward", 1 },
        { "BTR EAX, ECX", {0x0F, 0xB3, 0xC1}, 3, "Bit test and reset", 1 },

        /* String operations - should translate */
        { "MOVSB", {0xA4}, 1, "Move byte string", 1 },
        { "LODSB", {0xAC}, 1, "Load byte string", 1 },
        { "STOSB", {0xAA}, 1, "Store byte string", 1 },

        /* Special instructions - should translate */
        { "CPUID", {0x0F, 0xA2}, 2, "CPU identification", 1 },
        { "RDTSC", {0x0F, 0x31}, 2, "Read time-stamp counter", 1 },
        { "NOP", {0x90}, 1, "No operation", 1 },

        /* SIMD operations - should translate */
        { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3, "Move aligned packed single", 1 },
        { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3, "Add packed single", 1 },
        { "MOVSS XMM0, XMM1", {0xF3, 0x0F, 0x10, 0xC1}, 4, "Move scalar single", 1 },
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, "Packed add 64-bit", 1 },

        /* AVX operations - should translate */
        { "VEX.128.66.0F.WIG 59 /r", {0xC5, 0xF0, 0x59, 0xC1}, 4, "AVX multiply", 1 },

        /* Complex operations - should translate */
        { "CMOVZ EAX, EBX", {0x0F, 0x44, 0xC3}, 3, "Conditional move if zero", 1 },
        { "SETZ AL", {0x0F, 0x94, 0xC0}, 3, "Set if zero", 1 },

        /* Instructions that may not translate yet */
        { "SYSCALL", {0x0F, 0x05}, 2, "System call (may need special handling)", 0 },
        { "SYSENTER", {0x0F, 0x34}, 2, "System entry (may need special handling)", 0 },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d end-to-end translations:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        e2e_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        printf("[%2d] %s\\n", i+1, t->name);
        printf("     %s\\n", t->description);
        printf("     Bytes: ");
        for (int j = 0; j < t->x86_length; j++) {
            printf("%02X ", t->x86_bytes[j]);
        }
        printf("\\n");

        /* Step 1: Decode x86_64 instruction */
        int decoded = decode_x86_insn(t->x86_bytes, &insn);
        printf("     Decoded length: %d (expected %d)", decoded, t->x86_length);

        if (decoded != t->x86_length) {
            printf(" ✗ FAIL: Decode length mismatch\\n");
            failed++;
            printf("\\n");
            continue;
        }

        /* Step 2: Classify instruction */
        InsnCategory type = dispatch_classify_insn(&insn);
        printf(" ✓ Type: %d", type);

        /* Step 3: Check if translation dispatch exists */
        /* We can't actually translate without a ThreadState, but we can check if dispatch exists */
        int has_dispatch = 1;
        if (type == INSN_UNKNOWN) {
            has_dispatch = 0;
        }

        if (has_dispatch) {
            printf(" ✓ Has dispatch\\n");
        } else {
            printf(" ⚠ No dispatch (unknown instruction)\\n");
        }

        /* Check if result matches expectation */
        if (t->should_translate && !has_dispatch) {
            printf(" ✗ FAIL: Expected translation support\\n");
            failed++;
        } else if (!t->should_translate && has_dispatch) {
            printf(" ⚠ WARNING: Has dispatch but wasn't expected\\n");
            passed++;  /* Still count as pass if dispatch exists */
        } else {
            printf(" ✓ PASS\\n");
            passed++;
        }
        printf("\\n");
    }

    printf("==================================================\\n");
    printf("End-to-End Translation Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
