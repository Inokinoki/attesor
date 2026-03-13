/* Comprehensive decoder validation test */
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
    uint8_t expected_opcode;
    uint8_t expected_opcode2;
    int expected_has_modrm;
    const char *description;
} validation_test_t;

int main() {
    int passed = 0, failed = 0;

    validation_test_t tests[] = {
        /* Edge cases: minimum length instructions */
        { "RET", {0xC3}, 1, 0xC3, 0, 0, "Single-byte return" },
        { "NOP", {0x90}, 1, 0x90, 0, 0, "Single-byte NOP" },
        { "HLT", {0xF4}, 1, 0xF4, 0, 0, "Single-byte halt" },

        /* REX prefix variants */
        { "MOV RAX, RBX", {0x48, 0x8B, 0xC3}, 3, 0x8B, 0, 1, "REX.W + MOV" },
        { "MOV EAX, EBX", {0x8B, 0xC3}, 2, 0x8B, 0, 1, "No REX + MOV" },
        { "MOV R8, R9", {0x4C, 0x8B, 0xC9}, 3, 0x8B, 0, 1, "REX.R + MOV" },

        /* Legacy prefix: 0x66 operand-size override */
        { "MOV AX, BX", {0x66, 0x8B, 0xC3}, 3, 0x8B, 0, 1, "0x66 + MOV" },

        /* 0F escape opcodes without ModR/M */
        { "CPUID", {0x0F, 0xA2}, 2, 0x00, 0xA2, 0, "CPU identification" },
        { "SYSCALL", {0x0F, 0x05}, 2, 0x00, 0x05, 0, "Fast system call" },
        { "SYSRET", {0x0F, 0x07}, 2, 0x00, 0x07, 0, "Fast system return" },
        { "SYSENTER", {0x0F, 0x34}, 2, 0x00, 0x34, 0, "Fast system entry" },
        { "SYSEXIT", {0x0F, 0x35}, 2, 0x00, 0x35, 0, "Fast system exit" },
        { "RDTSC", {0x0F, 0x31}, 2, 0x00, 0x31, 0, "Read time-stamp counter" },

        /* 0F escape with ModR/M */
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, 0x00, 0xB6, 1, "MOV zero-extend" },
        { "BTC EAX, EBX", {0x0F, 0xBB, 0xC3}, 3, 0x00, 0xBB, 1, "Bit test complement" },
        { "BTS EAX, EBX", {0x0F, 0xAB, 0xC3}, 3, 0x00, 0xAB, 1, "Bit test set" },
        { "BSF EAX, ECX", {0x0F, 0xBC, 0xC1}, 3, 0x00, 0xBC, 1, "Bit scan forward" },
        { "BSR EAX, ECX", {0x0F, 0xBD, 0xC1}, 3, 0x00, 0xBD, 1, "Bit scan reverse" },

        /* Instructions with immediate operands */
        { "ADD EAX, 0x12345678", {0x05, 0x78, 0x56, 0x34, 0x12}, 5, 0x05, 0, 0, "ADD with imm32" },
        { "CMP EAX, 0x12345678", {0x3D, 0x78, 0x56, 0x34, 0x12}, 5, 0x3D, 0, 0, "CMP with imm32" },
        { "PUSH 0x42", {0x6A, 0x42}, 2, 0x6A, 0, 0, "PUSH imm8" },
        { "PUSH 0x12345678", {0x68, 0x78, 0x56, 0x34, 0x12}, 5, 0x68, 0, 0, "PUSH imm32" },
        { "IN AL, 0x80", {0xE4, 0x80}, 2, 0xE4, 0, 0, "IN from port 0x80" },

        /* Group 2 shifts with immediate */
        { "SHL EAX, 5", {0xC1, 0xE0, 0x05}, 3, 0xC1, 0, 1, "SHL with imm8" },
        { "ROL AL, 2", {0xC0, 0xC0, 0x02}, 3, 0xC0, 0, 1, "ROL with imm8" },

        /* Group 3 with immediate */
        { "BT EAX, 5", {0x0F, 0xBA, 0xE0, 0x05}, 4, 0x00, 0xBA, 1, "BT with imm8" },

        /* String instructions */
        { "MOVSB", {0xA4}, 1, 0xA4, 0, 0, "Move byte string" },
        { "MOVSD", {0xA5}, 1, 0xA5, 0, 0, "Move dword string" },
        { "REP MOVSB", {0xF3, 0xA4}, 2, 0xA4, 0, 0, "Repeat move byte" },

        /* Complex multi-byte instructions */
        { "IMUL EAX, EBX, 0x12345678",
          {0x69, 0xC3, 0x78, 0x56, 0x34, 0x12}, 6, 0x69, 0, 1, "IMUL with imm32" },

        /* Conditional moves */
        { "CMOVZ EAX, EBX", {0x0F, 0x44, 0xC3}, 3, 0x00, 0x44, 1, "Conditional move" },

        /* Conditional set */
        { "SETZ AL", {0x0F, 0x94, 0xC0}, 3, 0x00, 0x94, 1, "Set if zero" },

        /* Control flow */
        { "CALL label", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, 0xE8, 0, 0, "CALL relative" },
        { "JMP label", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, 0xE9, 0, 0, "JMP relative" },
        { "JE label", {0x74, 0x00}, 2, 0x74, 0, 0, "Jump if equal" },
        { "JO label", {0x70, 0x00}, 2, 0x70, 0, 0, "Jump if overflow" },

        /* Stack operations */
        { "PUSH RAX", {0x50}, 1, 0x50, 0, 0, "PUSH register" },
        { "POP RBX", {0x5B}, 1, 0x5B, 0, 0, "POP register" },
        { "PUSHF", {0x9C}, 1, 0x9C, 0, 0, "Push flags" },
        { "POPF", {0x9D}, 1, 0x9D, 0, 0, "Pop flags" },

        /* XCHG */
        { "XCHG RAX, RBX", {0x48, 0x93}, 2, 0x93, 0, 0, "Exchange registers" },

        /* LEA with complex addressing */
        { "LEA RAX, [RBX+RCX*4+0x100]",
          {0x48, 0x8D, 0x84, 0x9B, 0x00, 0x01, 0x00, 0x00},
          8, 0x8D, 0, 1, "Load effective address" },

        /* Sign extension */
        { "CDQE", {0x48, 0x98}, 2, 0x98, 0, 0, "Convert doubleword to quadword" },
        { "CWDE", {0x98}, 1, 0x98, 0, 0, "Convert word to doubleword" },

        /* Multiply/divide */
        { "MUL EBX", {0xF7, 0xE3}, 2, 0xF7, 0, 1, "Unsigned multiply" },
        { "IDIV EBX", {0xF7, 0xFB}, 2, 0xF7, 0, 1, "Signed divide" },

        /* INC/DEC with 0xFF */
        { "INC EAX", {0xFF, 0xC0}, 2, 0xFF, 0, 1, "Increment with 0xFF" },
        { "DEC EBX", {0xFF, 0xCB}, 2, 0xFF, 0, 1, "Decrement with 0xFF" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Comprehensive Decoder Validation Test\n");
    printf("Testing %d edge cases and complex patterns:\n\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        validation_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %s\n", i+1, t->name);
        printf("     %s\n", t->description);
        printf("     Length: %d (expected %d)", decoded, t->length);

        int all_pass = 1;
        if (decoded != t->length) {
            printf(" ✗ FAIL\n");
            all_pass = 0;
        } else if (insn.opcode != t->expected_opcode) {
            printf(" ✗ FAIL (opcode: 0x%02X, expected 0x%02X)\n",
                   insn.opcode, t->expected_opcode);
            all_pass = 0;
        } else if (insn.opcode2 != t->expected_opcode2) {
            printf(" ✗ FAIL (opcode2: 0x%02X, expected 0x%02X)\n",
                   insn.opcode2, t->expected_opcode2);
            all_pass = 0;
        } else if (insn.has_modrm != t->expected_has_modrm) {
            printf(" ✗ FAIL (has_modrm: %d, expected %d)\n",
                   insn.has_modrm, t->expected_has_modrm);
            all_pass = 0;
        } else {
            printf(" ✓ PASS\n");
        }

        if (all_pass) {
            passed++;
        } else {
            failed++;
        }
        printf("\n");
    }

    printf("==================================================\n");
    printf("Validation Results: %d/%d tests passed (%.1f%%)\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\n", failed);
    printf("==================================================\n");

    return failed > 0 ? 1 : 0;
}
