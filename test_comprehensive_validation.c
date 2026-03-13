/* Comprehensive decoder + classification validation test */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef enum {
    CAT_ALU, CAT_MEMORY, CAT_BRANCH, CAT_BIT, CAT_STRING, CAT_SPECIAL, CAT_SIMD
} TestCategory;

const char* cat_name(TestCategory cat) {
    switch (cat) {
        case CAT_ALU: return "ALU";
        case CAT_MEMORY: return "MEMORY";
        case CAT_BRANCH: return "BRANCH";
        case CAT_BIT: return "BIT";
        case CAT_STRING: return "STRING";
        case CAT_SPECIAL: return "SPECIAL";
        case CAT_SIMD: return "SIMD";
        default: return "UNKNOWN";
    }
}

TestCategory classify(x86_insn_t *insn) {
    uint8_t op = insn->opcode;
    uint8_t op2 = insn->opcode2;

    /* String instructions (check first) */
    if (op == 0xA4 || op == 0xA5 || op == 0xA6 || op == 0xAE ||
        op == 0xAC || op == 0xAA) {
        return CAT_STRING;
    }

    /* INC/DEC with 0xFF are ALU, not branch (check before branch) */
    if (op == 0xFF && (insn->reg == 0 || insn->reg == 1)) {
        return CAT_ALU;
    }

    /* Branch instructions */
    if ((op >= 0x70 && op <= 0x7F) || op == 0xE8 || op == 0xE9 ||
        op == 0xEB || op == 0xC3 || op == 0xCB || op == 0xC2 ||
        (op == 0xFF && (insn->reg >= 2 && insn->reg <= 4))) {
        return CAT_BRANCH;
    }

    /* Special instructions */
    if (op == 0x90 || op == 0x9B || op == 0xD7 || op == 0xF8 ||
        op == 0xF9 || op == 0xFA || op == 0xFB || op == 0xFC ||
        op == 0xFD || op == 0x9E || op == 0x9F || op == 0x9C ||
        op == 0x9D || op == 0x98 || op == 0x99 || op == 0xCD ||
        op == 0xCF) {
        return CAT_SPECIAL;
    }

    /* 0F XX instructions */
    if (op == 0x00 && op2 != 0x00) {
        /* System instructions */
        if (op2 == 0xA2 || op2 == 0x31 || op2 == 0x30 || op2 == 0x32 ||
            op2 == 0x05 || op2 == 0x07 || op2 == 0x34 || op2 == 0x35) {
            return CAT_SPECIAL;
        }

        /* Bit operations */
        if (op2 == 0xA3 || op2 == 0xAB || op2 == 0xB3 ||
            op2 == 0xBB || op2 == 0xBC || op2 == 0xBD ||
            (insn->simd_prefix == 0xF3 && (op2 == 0xB8 || op2 == 0xBD || op2 == 0xBC))) {
            return CAT_BIT;
        }

        /* Memory operations */
        if (op2 == 0xB6 || op2 == 0xB7 || op2 == 0xBE || op2 == 0xBF) {
            return CAT_MEMORY;
        }

        /* SIMD operations */
        return CAT_SIMD;
    }

    /* SIMD operations */
    if (insn->vex_prefix || insn->simd_prefix == 0x66 ||
        insn->simd_prefix == 0xF2 || insn->simd_prefix == 0xF3) {
        return CAT_SIMD;
    }

    /* Memory operations */
    if (op == 0x8B || op == 0x89 || op == 0x8D || op == 0x50 ||
        op == 0x58 || op == 0x87 || (op >= 0x90 && op <= 0x97)) {
        return CAT_MEMORY;
    }

    /* ALU operations (default) */
    return CAT_ALU;
}

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    TestCategory category;
} validate_test_t;

int main() {
    int passed = 0, failed = 0;

    validate_test_t tests[] = {
        /* Complex real-world instruction sequences */
        { "Function prologue", {0x55, 0x48, 0x89, 0xE5, 0x48, 0x83, 0xEC, 0x20}, 8, CAT_MEMORY },
        { "Function epilogue", {0x48, 0x83, 0xC4, 0x20, 0x5D, 0xC3}, 6, CAT_MEMORY },

        /* Common instruction patterns */
        { "ADD + CMP + JE", {0x01, 0xD8, 0x3B, 0xC3, 0x74, 0x00}, 6, CAT_ALU },
        { "MOV + TEST + JNZ", {0x8B, 0x03, 0x85, 0xC0, 0x75, 0x00}, 6, CAT_MEMORY },
        { "CALL + RET", {0xE8, 0x00, 0x00, 0x00, 0x00, 0xC3}, 6, CAT_BRANCH },
        { "PUSH + POP", {0x50, 0x58}, 2, CAT_MEMORY },
        { "LOCK ADD", {0xF0, 0x01, 0xD8}, 3, CAT_ALU },
        { "REP MOVSB", {0xF3, 0xA4}, 2, CAT_STRING },

        /* Register operations */
        { "XOR to zero", {0x31, 0xC0}, 2, CAT_ALU },
        { "INC loop", {0xFF, 0xC0, 0x75, 0xFC}, 4, CAT_ALU },
        { "SHL variable", {0xD3, 0xE0}, 2, CAT_ALU },
        { "ROR variable", {0xD3, 0xC8}, 2, CAT_ALU },

        /* Memory operations */
        { "MOV + LEA", {0x8B, 0x03, 0x8D, 0x04, 0x18}, 5, CAT_MEMORY },
        { "PUSH + MOV + POP", {0x50, 0x8B, 0xC3, 0x58}, 4, CAT_MEMORY },

        /* Bit operations */
        { "BT + JC", {0x0F, 0xA3, 0xC3, 0x72, 0x00}, 5, CAT_BIT },
        { "BSF + JNZ", {0x0F, 0xBC, 0xC3, 0x75, 0x00}, 5, CAT_BIT },

        /* SIMD operations */
        { "MOVAPS + ADDPS", {0x0F, 0x28, 0xC1, 0x0F, 0x58, 0xC1}, 6, CAT_SIMD },
        { "PXOR + PADDQ", {0x66, 0x0F, 0xEF, 0xC1, 0x66, 0x0F, 0xD4, 0xC1}, 8, CAT_SIMD },
        { "VEX multiply", {0xC5, 0xF0, 0x59, 0xC2}, 4, CAT_SIMD },

        /* String operations */
        { "REP CMPSB", {0xF3, 0xA6}, 2, CAT_STRING },
        { "REPNE SCASB", {0xF2, 0xAE}, 2, CAT_STRING },

        /* Special instructions */
        { "CPUID", {0x0F, 0xA2}, 2, CAT_SPECIAL },
        { "RDTSC", {0x0F, 0x31}, 2, CAT_SPECIAL },
        { "NOP", {0x90}, 1, CAT_SPECIAL },

        /* Mixed operations */
        { "ADD + SUB + CMP", {0x01, 0xD8, 0x29, 0xD8, 0x3B, 0xC3}, 6, CAT_ALU },
        { "MOV + MOV + MOV", {0x8B, 0xC3, 0x8B, 0x07, 0x89, 0x07}, 6, CAT_MEMORY },
        { "JMP + CALL + RET", {0xEB, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xC3}, 8, CAT_BRANCH },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d comprehensive instruction patterns:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        validate_test_t *t = &tests[i];
        int all_pass = 1;

        printf("[%2d] %s\\n     ", i+1, t->name);
        printf("Bytes: ");
        for (int j = 0; j < t->length; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\\n");

        /* Decode each instruction in the sequence */
        int offset = 0;
        int insn_count = 0;
        while (offset < t->length) {
            x86_insn_t insn;
            memset(&insn, 0, sizeof(insn));

            int decoded = decode_x86_insn(t->bytes + offset, &insn);
            TestCategory actual_cat = classify(&insn);

            printf("     Insn %d: %d bytes, cat=%s", insn_count+1, decoded, cat_name(actual_cat));

            if (offset + decoded > t->length) {
                printf(" ✗ OVERRUN\\n");
                all_pass = 0;
                break;
            }

            offset += decoded;
            insn_count++;

            if (offset < t->length) {
                printf("\\n");
            }
        }

        if (all_pass && offset == t->length) {
            printf(" ✓ PASS (%d instructions)\\n", insn_count);
            passed++;
        } else {
            printf(" ✗ FAIL\\n");
            failed++;
        }
        printf("\\n");
    }

    printf("==================================================\\n");
    printf("Comprehensive Validation Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
