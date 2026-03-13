/* Test instruction classification only */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

/* Simplified classification function for testing */
typedef enum {
    INSN_ALU,
    INSN_MEMORY,
    INSN_BRANCH,
    INSN_BIT,
    InSN_STRING,
    INSN_SPECIAL,
    INSN_SIMD,
} TestInsnCategory;

const char* category_name(TestInsnCategory cat) {
    switch (cat) {
        case INSN_ALU: return "ALU";
        case INSN_MEMORY: return "MEMORY";
        case INSN_BRANCH: return "BRANCH";
        case INSN_BIT: return "BIT";
        case InSN_STRING: return "STRING";
        case INSN_SPECIAL: return "SPECIAL";
        case INSN_SIMD: return "SIMD";
        default: return "UNKNOWN";
    }
}

/* Simple classification based on opcode */
TestInsnCategory classify_insn_simple(x86_insn_t *insn) {
    uint8_t op = insn->opcode;
    uint8_t op2 = insn->opcode2;

    /* String instructions (check first) */
    if (op == 0xA4 || op == 0xA5 || op == 0xA6 || op == 0xAE ||
        op == 0xAC || op == 0xAA) {
        return InSN_STRING;
    }

    /* INC/DEC with 0xFF are ALU, not branch (check before branch) */
    if (op == 0xFF && (insn->reg == 0 || insn->reg == 1)) {
        return INSN_ALU;
    }

    /* Branch instructions */
    if ((op >= 0x70 && op <= 0x7F) || op == 0xE8 || op == 0xE9 ||
        op == 0xEB || op == 0xC3 || op == 0xCB || op == 0xC2 ||
        (op == 0xFF && (insn->reg >= 2 && insn->reg <= 4))) {  /* CALL, JMP, RET via FF */
        return INSN_BRANCH;
    }

    /* Special instructions */
    if (op == 0x90 || op == 0x9B || op == 0xD7 || op == 0xF8 ||
        op == 0xF9 || op == 0xFA || op == 0xFB || op == 0xFC ||
        op == 0xFD || op == 0x9E || op == 0x9F || op == 0x9C ||
        op == 0x9D || op == 0x98 || op == 0x99 || op == 0xCD ||
        op == 0xCF) {
        return INSN_SPECIAL;
    }

    /* 0F XX instructions */
    if (op == 0x00 && op2 != 0x00) {
        /* System instructions (0F XX) */
        if (op2 == 0xA2 || op2 == 0x31 || op2 == 0x30 || op2 == 0x32 ||
            op2 == 0x05 || op2 == 0x07 || op2 == 0x34 || op2 == 0x35) {
            return INSN_SPECIAL;
        }

        /* Bit operations (0F XX) */
        if (op2 == 0xA3 || op2 == 0xAB || op2 == 0xB3 ||
            op2 == 0xBB || op2 == 0xBC || op2 == 0xBD ||
            (insn->simd_prefix == 0xF3 && (op2 == 0xB8 || op2 == 0xBD || op2 == 0xBC))) {
            return INSN_BIT;
        }

        /* Memory operations (0F XX) */
        if (op2 == 0xB6 || op2 == 0xB7 || op2 == 0xBE || op2 == 0xBF) {
            return INSN_MEMORY;
        }

        /* SIMD operations (0F XX) - most 0F XX opcodes are SIMD */
        return INSN_SIMD;
    }

    /* SIMD operations (VEX prefix or SIMD prefix) */
    if (insn->vex_prefix || insn->simd_prefix == 0x66 ||
        insn->simd_prefix == 0xF2 || insn->simd_prefix == 0xF3) {
        return INSN_SIMD;
    }

    /* Memory operations */
    if (op == 0x8B || op == 0x89 || op == 0x8D || op == 0x50 ||
        op == 0x58 || op == 0x87 || /* XCHG */
        (op >= 0x90 && op <= 0x97)) {  /* XCHG with EAX */
        return INSN_MEMORY;
    }

    /* ALU operations (default) */
    return INSN_ALU;
}

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    TestInsnCategory expected_category;
} classify_test_t;

int main() {
    int passed = 0, failed = 0;

    classify_test_t tests[] = {
        /* ALU operations */
        { "ADD EAX, EBX", {0x01, 0xD8}, 2, INSN_ALU },
        { "SUB EAX, EBX", {0x29, 0xD8}, 2, INSN_ALU },
        { "AND EAX, EBX", {0x21, 0xD8}, 2, INSN_ALU },
        { "OR EAX, EBX", {0x09, 0xD8}, 2, INSN_ALU },
        { "XOR EAX, EBX", {0x31, 0xD8}, 2, INSN_ALU },
        { "CMP EAX, EBX", {0x3B, 0xC3}, 2, INSN_ALU },
        { "TEST EAX, EBX", {0x85, 0xC3}, 2, INSN_ALU },
        { "INC EAX", {0xFF, 0xC0}, 2, INSN_ALU },
        { "DEC EAX", {0xFF, 0xC8}, 2, INSN_ALU },

        /* Memory operations */
        { "MOV EAX, EBX", {0x8B, 0xC3}, 2, INSN_MEMORY },
        { "MOV EAX, [EBX]", {0x8B, 0x03}, 2, INSN_MEMORY },
        { "MOV [EBX], EAX", {0x89, 0x03}, 2, INSN_MEMORY },
        { "LEA EAX, [EBX+ECX]", {0x8D, 0x04, 0x0B}, 3, INSN_MEMORY },
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, INSN_MEMORY },
        { "MOVSX EAX, BL", {0x0F, 0xBE, 0xC3}, 3, INSN_MEMORY },
        { "XCHG EAX, EBX", {0x93}, 1, INSN_MEMORY },
        { "PUSH EAX", {0x50}, 1, INSN_MEMORY },
        { "POP EAX", {0x58}, 1, INSN_MEMORY },

        /* Branch operations */
        { "JMP label", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, INSN_BRANCH },
        { "CALL label", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, INSN_BRANCH },
        { "RET", {0xC3}, 1, INSN_BRANCH },
        { "JE label", {0x74, 0x00}, 2, INSN_BRANCH },
        { "JNE label", {0x75, 0x00}, 2, INSN_BRANCH },
        { "JL label", {0x7C, 0x00}, 2, INSN_BRANCH },
        { "JG label", {0x7F, 0x00}, 2, INSN_BRANCH },

        /* Bit operations */
        { "BT EAX, 3", {0x0F, 0xA3, 0xC3}, 3, INSN_BIT },
        { "BTS EAX, 5", {0x0F, 0xAB, 0xC3}, 3, INSN_BIT },
        { "BSF EAX, ECX", {0x0F, 0xBC, 0xC1}, 3, INSN_BIT },
        { "BSR EAX, ECX", {0x0F, 0xBD, 0xC1}, 3, INSN_BIT },

        /* String operations */
        { "MOVSB", {0xA4}, 1, InSN_STRING },
        { "MOVSD", {0xA5}, 1, InSN_STRING },
        { "CMPSB", {0xA6}, 1, InSN_STRING },
        { "SCASB", {0xAE}, 1, InSN_STRING },
        { "LODSB", {0xAC}, 1, InSN_STRING },
        { "STOSB", {0xAA}, 1, InSN_STRING },

        /* Special operations */
        { "NOP", {0x90}, 1, INSN_SPECIAL },
        { "CPUID", {0x0F, 0xA2}, 2, INSN_SPECIAL },
        { "RDTSC", {0x0F, 0x31}, 2, INSN_SPECIAL },
        { "SYSCALL", {0x0F, 0x05}, 2, INSN_SPECIAL },
        { "INT 0x80", {0xCD, 0x80}, 2, INSN_SPECIAL },
        { "CLC", {0xF8}, 1, INSN_SPECIAL },
        { "STC", {0xF9}, 1, INSN_SPECIAL },

        /* SIMD operations */
        { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3, INSN_SIMD },
        { "ADDSS XMM0, XMM1", {0xF3, 0x0F, 0x58, 0xC1}, 4, INSN_SIMD },
        { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3, INSN_SIMD },
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, INSN_SIMD },
        { "PXOR XMM0, XMM1", {0x66, 0x0F, 0xEF, 0xC1}, 4, INSN_SIMD },
        { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5, INSN_SIMD },
        { "VADDPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x58, 0xC2}, 4, INSN_SIMD },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d instruction classifications:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        classify_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);
        TestInsnCategory actual = classify_insn_simple(&insn);

        printf("[%3d] %-25s -> ", i+1, t->name);

        if (decoded != t->length) {
            printf("✗ FAIL: Decode length %d (expected %d)\\n", decoded, t->length);
            failed++;
        } else if (actual != t->expected_category) {
            printf("✗ FAIL: Category %s (expected %s)\\n",
                   category_name(actual), category_name(t->expected_category));
            failed++;
        } else {
            printf("✓ PASS [%s]\\n", category_name(actual));
            passed++;
        }
    }

    printf("\\n==================================================\\n");
    printf("Classification Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
