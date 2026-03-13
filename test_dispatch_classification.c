/* Test instruction classification for dispatch */
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
    InsnCategory expected_category;
} dispatch_test_t;

const char* category_name(InsnCategory cat) {
    switch (cat) {
        case INSN_ALU: return "ALU";
        case INSN_MEMORY: return "MEMORY";
        case INSN_BRANCH: return "BRANCH";
        case INSN_BIT: return "BIT";
        case INSN_STRING: return "STRING";
        case INSN_SPECIAL: return "SPECIAL";
        case INSN_SIMD: return "SIMD";
        default: return "UNKNOWN";
    }
}

int main() {
    int passed = 0, failed = 0;

    dispatch_test_t tests[] = {
        /* ALU operations */
        { "ADD EAX, EBX", {0x01, 0xD8}, 2, INSN_ALU },
        { "SUB EAX, EBX", {0x29, 0xD8}, 2, INSN_ALU },
        { "AND EAX, EBX", {0x21, 0xD8}, 2, INSN_ALU },
        { "OR EAX, EBX", {0x09, 0xD8}, 2, INSN_ALU },
        { "XOR EAX, EBX", {0x31, 0xD8}, 2, INSN_ALU },
        { "INC EAX", {0xFF, 0xC0}, 2, INSN_ALU },
        { "DEC EAX", {0xFF, 0xC8}, 2, INSN_ALU },
        { "NEG EAX", {0xF7, 0xD8}, 2, INSN_ALU },
        { "NOT EAX", {0xF7, 0xD0}, 2, INSN_ALU },
        { "CMP EAX, EBX", {0x3B, 0xC3}, 2, INSN_ALU },
        { "TEST EAX, EBX", {0x85, 0xC3}, 2, INSN_ALU },
        { "IMUL EAX, EBX", {0x0F, 0xAF, 0xC3}, 3, INSN_ALU },
        { "SHL EAX, 1", {0xD1, 0xE0}, 2, INSN_ALU },
        { "SHR EAX, 1", {0xD1, 0xE8}, 2, INSN_ALU },
        { "SAR EAX, 1", {0xD1, 0xF8}, 2, INSN_ALU },
        { "ROL EAX, 1", {0xD1, 0xC0}, 2, INSN_ALU },
        { "ROR EAX, 1", {0xD1, 0xC8}, 2, INSN_ALU },

        /* Memory operations */
        { "MOV EAX, EBX", {0x8B, 0xC3}, 2, INSN_MEMORY },
        { "MOV EAX, [EBX]", {0x8B, 0x03}, 2, INSN_MEMORY },
        { "MOV [EBX], EAX", {0x89, 0x03}, 2, INSN_MEMORY },
        { "LEA EAX, [EBX+ECX]", {0x8D, 0x04, 0x0B}, 3, INSN_MEMORY },
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, INSN_MEMORY },
        { "MOVSX EAX, BL", {0x0F, 0xBE, 0xC3}, 3, INSN_MEMORY },
        { "MOVSXD RAX, EAX", {0x48, 0x63, 0xC0}, 3, INSN_MEMORY },
        { "XCHG EAX, EBX", {0x93}, 1, INSN_MEMORY },
        { "PUSH EAX", {0x50}, 1, INSN_MEMORY },
        { "POP EAX", {0x58}, 1, INSN_MEMORY },
        { "CMPXCHG EAX, EBX", {0x0F, 0xB1, 0xC3}, 3, INSN_MEMORY },

        /* Branch operations */
        { "JMP label", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, INSN_BRANCH },
        { "JMP EAX", {0xFF, 0xE0}, 2, INSN_BRANCH },
        { "CALL label", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, INSN_BRANCH },
        { "CALL EAX", {0xFF, 0xD0}, 2, INSN_BRANCH },
        { "RET", {0xC3}, 1, INSN_BRANCH },
        { "JE label", {0x74, 0x00}, 2, INSN_BRANCH },
        { "JNE label", {0x75, 0x00}, 2, INSN_BRANCH },
        { "JL label", {0x7C, 0x00}, 2, INSN_BRANCH },
        { "JG label", {0x7F, 0x00}, 2, INSN_BRANCH },
        { "CMOVZ EAX, EBX", {0x0F, 0x44, 0xC3}, 3, INSN_BRANCH },
        { "SETZ AL", {0x0F, 0x94, 0xC0}, 3, INSN_BRANCH },
        { "LOOP label", {0xE2, 0x00}, 2, INSN_BRANCH },
        { "JCXZ label", {0xE3, 0x00}, 2, INSN_BRANCH },

        /* Bit operations */
        { "BT EAX, 3", {0x0F, 0xA3, 0xC3}, 3, INSN_BIT },
        { "BTS EAX, 5", {0x0F, 0xAB, 0xC3}, 3, INSN_BIT },
        { "BTR EAX, 7", {0x0F, 0xB3, 0xC3}, 3, INSN_BIT },
        { "BTC EAX, 9", {0x0F, 0xBB, 0xC3}, 3, INSN_BIT },
        { "BSF EAX, ECX", {0x0F, 0xBC, 0xC1}, 3, INSN_BIT },
        { "BSR EAX, ECX", {0x0F, 0xBD, 0xC1}, 3, INSN_BIT },
        { "POPCNT EAX, ECX", {0xF3, 0x0F, 0xB8, 0xC1}, 4, INSN_BIT },
        { "LZCNT EAX, ECX", {0xF3, 0x0F, 0xBD, 0xC1}, 4, INSN_BIT },
        { "TZCNT EAX, ECX", {0xF3, 0x0F, 0xBC, 0xC1}, 4, INSN_BIT },

        /* String operations */
        { "MOVSB", {0xA4}, 1, INSN_STRING },
        { "MOVSW", {0x66, 0xA5}, 2, INSN_STRING },
        { "MOVSD", {0xA5}, 1, INSN_STRING },
        { "CMPSB", {0xA6}, 1, INSN_STRING },
        { "SCASB", {0xAE}, 1, INSN_STRING },
        { "LODSB", {0xAC}, 1, INSN_STRING },
        { "STOSB", {0xAA}, 1, INSN_STRING },
        { "REP MOVSB", {0xF3, 0xA4}, 2, INSN_STRING },
        { "REPE CMPSB", {0xF3, 0xA6}, 2, INSN_STRING },
        { "REPNE SCASB", {0xF2, 0xAE}, 2, INSN_STRING },

        /* Special operations */
        { "NOP", {0x90}, 1, INSN_SPECIAL },
        { "CPUID", {0x0F, 0xA2}, 2, INSN_SPECIAL },
        { "RDTSC", {0x0F, 0x31}, 2, INSN_SPECIAL },
        { "RDMSR", {0x0F, 0x32}, 2, INSN_SPECIAL },
        { "WRMSR", {0x0F, 0x30}, 2, INSN_SPECIAL },
        { "SYSCALL", {0x0F, 0x05}, 2, INSN_SPECIAL },
        { "SYSENTER", {0x0F, 0x34}, 2, INSN_SPECIAL },
        { "INT 0x80", {0xCD, 0x80}, 2, INSN_SPECIAL },
        { "IRETQ", {0x48, 0xCF}, 2, INSN_SPECIAL },
        { "CLC", {0xF8}, 1, INSN_SPECIAL },
        { "STC", {0xF9}, 1, INSN_SPECIAL },
        { "CLI", {0xFA}, 1, INSN_SPECIAL },
        { "STI", {0xFB}, 1, INSN_SPECIAL },
        { "CLD", {0xFC}, 1, INSN_SPECIAL },
        { "STD", {0xFD}, 1, INSN_SPECIAL },
        { "SAHF", {0x9E}, 1, INSN_SPECIAL },
        { "LAHF", {0x9F}, 1, INSN_SPECIAL },
        { "PUSHFQ", {0x9C}, 1, INSN_SPECIAL },
        { "POPFQ", {0x9D}, 1, INSN_SPECIAL },
        { "XLAT", {0xD7}, 1, INSN_SPECIAL },
        { "CBW", {0x66, 0x98}, 2, INSN_SPECIAL },
        { "CWDE", {0x98}, 1, INSN_SPECIAL },
        { "CDQE", {0x48, 0x98}, 2, INSN_SPECIAL },
        { "CWD", {0x66, 0x99}, 2, INSN_SPECIAL },
        { "CDQ", {0x99}, 1, INSN_SPECIAL },
        { "CQO", {0x48, 0x99}, 2, INSN_SPECIAL },

        /* SIMD operations */
        { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3, INSN_SIMD },
        { "ADDSS XMM0, XMM1", {0xF3, 0x0F, 0x58, 0xC1}, 4, INSN_SIMD },
        { "SUBPS XMM0, XMM1", {0x0F, 0x5C, 0xC1}, 3, INSN_SIMD },
        { "MULPS XMM0, XMM1", {0x0F, 0x59, 0xC1}, 3, INSN_SIMD },
        { "DIVPS XMM0, XMM1", {0x0F, 0x5E, 0xC1}, 3, INSN_SIMD },
        { "CMPPS XMM0, XMM1, 0", {0x0F, 0xC2, 0xC1, 0x00}, 4, INSN_SIMD },
        { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3, INSN_SIMD },
        { "MOVUPS XMM0, XMM1", {0x0F, 0x10, 0xC1}, 3, INSN_SIMD },
        { "MOVSS XMM0, XMM1", {0xF3, 0x0F, 0x10, 0xC1}, 4, INSN_SIMD },
        { "MOVDQA XMM0, XMM1", {0x66, 0x0F, 0x6F, 0xC1}, 4, INSN_SIMD },
        { "MOVDQU XMM0, XMM1", {0xF3, 0x0F, 0x6F, 0xC1}, 4, INSN_SIMD },
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, INSN_SIMD },
        { "PSUBQ XMM0, XMM1", {0x66, 0x0F, 0xFB, 0xC1}, 4, INSN_SIMD },
        { "PMULUDQ XMM0, XMM1", {0x66, 0x0F, 0xF4, 0xC1}, 4, INSN_SIMD },
        { "PAND XMM0, XMM1", {0x66, 0x0F, 0xDB, 0xC1}, 4, INSN_SIMD },
        { "POR XMM0, XMM1", {0x66, 0x0F, 0xEB, 0xC1}, 4, INSN_SIMD },
        { "PXOR XMM0, XMM1", {0x66, 0x0F, 0xEF, 0xC1}, 4, INSN_SIMD },
        { "PUNPCKLDQ XMM0, XMM1", {0x66, 0x0F, 0x62, 0xC1}, 4, INSN_SIMD },
        { "PUNPCKHDQ XMM0, XMM1", {0x66, 0x0F, 0x6A, 0xC1}, 4, INSN_SIMD },
        { "SHUFPS XMM0, XMM1, 0", {0x0F, 0xC6, 0xC1, 0x00}, 4, INSN_SIMD },
        { "PSHUFD XMM0, XMM1, 0", {0x66, 0x0F, 0x70, 0xC1, 0x00}, 5, INSN_SIMD },
        { "SQRTPS XMM0, XMM1", {0x0F, 0x51, 0xC1}, 3, INSN_SIMD },
        { "RSQRTPS XMM0, XMM1", {0x0F, 0x52, 0xC1}, 3, INSN_SIMD },
        { "RCPPS XMM0, XMM1", {0x0F, 0x53, 0xC1}, 3, INSN_SIMD },
        { "COMISS XMM0, XMM1", {0x0F, 0x2F, 0xC1}, 3, INSN_SIMD },
        { "UCOMISS XMM0, XMM1", {0x0F, 0x2E, 0xC1}, 3, INSN_SIMD },

        /* AVX operations */
        { "VADDPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x58, 0xC2}, 4, INSN_SIMD },
        { "VMULPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x59, 0xC2}, 4, INSN_SIMD },
        { "VSUBPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x5C, 0xC2}, 4, INSN_SIMD },
        { "VDIVPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x5E, 0xC2}, 4, INSN_SIMD },
        { "VCMPPS XMM0, XMM1, XMM2, 0", {0xC5, 0xF0, 0xC2, 0xC2, 0x00}, 5, INSN_SIMD },

        /* Crypto operations (classified as SIMD) */
        { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5, INSN_SIMD },
        { "AESENCLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDD, 0xC1}, 5, INSN_SIMD },
        { "AESDEC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDE, 0xC1}, 5, INSN_SIMD },
        { "AESDECLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDF, 0xC1}, 5, INSN_SIMD },
        { "AESIMC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDB, 0xC1}, 5, INSN_SIMD },
        { "AESKEYGENASSIST XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0xDF, 0xC1, 0x00}, 6, INSN_SIMD },
        { "SHA1RNDS4 XMM0, XMM1, 0", {0x0F, 0x3A, 0xCC, 0xC1, 0x00}, 5, INSN_SIMD },
        { "SHA1NEXTE XMM0, XMM1", {0x0F, 0x38, 0xC8, 0xC1}, 4, INSN_SIMD },
        { "SHA1MSG1 XMM0, XMM1", {0x0F, 0x38, 0xC9, 0xC1}, 4, INSN_SIMD },
        { "SHA1MSG2 XMM0, XMM1", {0x0F, 0x38, 0xCA, 0xC1}, 4, INSN_SIMD },
        { "SHA256RNDS2 XMM0, XMM1", {0x0F, 0x38, 0xCB, 0xC1}, 4, INSN_SIMD },
        { "SHA256MSG1 XMM0, XMM1", {0x0F, 0x38, 0xCC, 0xC1}, 4, INSN_SIMD },
        { "SHA256MSG2 XMM0, XMM1", {0x0F, 0x38, 0xCD, 0xC1}, 4, INSN_SIMD },
        { "PCLMULQDQ XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x44, 0xC1, 0x00}, 6, INSN_SIMD },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d instruction classifications:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        dispatch_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);
        InsnCategory actual = dispatch_classify_insn(&insn);

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
    printf("Dispatch Classification Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
