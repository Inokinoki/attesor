/* Comprehensive instruction coverage test */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rosetta_types.h"
#include "rosetta_x86_decode.h"

typedef struct {
    const char *name;
    uint8_t bytes[16];
    int length;
    const char *category;
} coverage_test_t;

int main() {
    int passed = 0, failed = 0;

    coverage_test_t tests[] = {
        /* System instructions */
        { "CPUID", {0x0F, 0xA2}, 2, "SYSTEM" },
        { "RDTSC", {0x0F, 0x31}, 2, "SYSTEM" },
        { "RDTSCP", {0x0F, 0x01, 0xF9}, 3, "SYSTEM" },
        { "RDMSR", {0x0F, 0x32}, 2, "SYSTEM" },
        { "WRMSR", {0x0F, 0x30}, 2, "SYSTEM" },
        { "SYSCALL", {0x0F, 0x05}, 2, "SYSTEM" },
        { "SYSRET", {0x0F, 0x07}, 2, "SYSTEM" },
        { "SYSENTER", {0x0F, 0x34}, 2, "SYSTEM" },
        { "SYSEXIT", {0x0F, 0x35}, 2, "SYSTEM" },

        /* Control transfer */
        { "JMP near rel32", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, "BRANCH" },
        { "JMP short rel8", {0xEB, 0x00}, 2, "BRANCH" },
        { "JMP rm64", {0xFF, 0xE0}, 2, "BRANCH" },
        { "CALL near rel32", {0xE8, 0x00, 0x00, 0x00, 0x00}, 5, "BRANCH" },
        { "CALL rm64", {0xFF, 0xD0}, 2, "BRANCH" },
        { "RET near", {0xC3}, 1, "BRANCH" },
        { "RET far", {0xCB}, 1, "BRANCH" },
        { "RETN imm16", {0xC2, 0x04, 0x00}, 3, "BRANCH" },

        /* Conditional jumps */
        { "JO rel8", {0x70, 0x00}, 2, "BRANCH" },
        { "JNO rel8", {0x71, 0x00}, 2, "BRANCH" },
        { "JB rel8", {0x72, 0x00}, 2, "BRANCH" },
        { "JNB rel8", {0x73, 0x00}, 2, "BRANCH" },
        { "JZ rel8", {0x74, 0x00}, 2, "BRANCH" },
        { "JNZ rel8", {0x75, 0x00}, 2, "BRANCH" },
        { "JBE rel8", {0x76, 0x00}, 2, "BRANCH" },
        { "JA rel8", {0x77, 0x00}, 2, "BRANCH" },
        { "JS rel8", {0x78, 0x00}, 2, "BRANCH" },
        { "JNS rel8", {0x79, 0x00}, 2, "BRANCH" },
        { "JP rel8", {0x7A, 0x00}, 2, "BRANCH" },
        { "JNP rel8", {0x7B, 0x00}, 2, "BRANCH" },
        { "JL rel8", {0x7C, 0x00}, 2, "BRANCH" },
        { "JGE rel8", {0x7D, 0x00}, 2, "BRANCH" },
        { "JLE rel8", {0x7E, 0x00}, 2, "BRANCH" },
        { "JG rel8", {0x7F, 0x00}, 2, "BRANCH" },

        /* String operations */
        { "MOVSB", {0xA4}, 1, "STRING" },
        { "MOVSW", {0x66, 0xA5}, 2, "STRING" },
        { "MOVSD", {0xA5}, 1, "STRING" },
        { "CMPSB", {0xA6}, 1, "STRING" },
        { "SCASB", {0xAE}, 1, "STRING" },
        { "LODSB", {0xAC}, 1, "STRING" },
        { "STOSB", {0xAA}, 1, "STRING" },

        /* REP prefixes */
        { "REP MOVSB", {0xF3, 0xA4}, 2, "STRING" },
        { "REP MOVSD", {0xF3, 0xA5}, 2, "STRING" },
        { "REPE CMPSB", {0xF3, 0xA6}, 2, "STRING" },
        { "REPNE SCASB", {0xF2, 0xAE}, 2, "STRING" },

        /* Stack operations */
        { "PUSH RAX", {0x50}, 1, "STACK" },
        { "PUSH RCX", {0x51}, 1, "STACK" },
        { "PUSH RDX", {0x52}, 1, "STACK" },
        { "PUSH RBX", {0x53}, 1, "STACK" },
        { "PUSH RSP", {0x54}, 1, "STACK" },
        { "PUSH RBP", {0x55}, 1, "STACK" },
        { "PUSH RSI", {0x56}, 1, "STACK" },
        { "PUSH RDI", {0x57}, 1, "STACK" },
        { "POP RAX", {0x58}, 1, "STACK" },
        { "POP RCX", {0x59}, 1, "STACK" },
        { "POP RDX", {0x5A}, 1, "STACK" },
        { "POP RBX", {0x5B}, 1, "STACK" },
        { "POP RSP", {0x5C}, 1, "STACK" },
        { "POP RBP", {0x5D}, 1, "STACK" },
        { "POP RSI", {0x5E}, 1, "STACK" },
        { "POP RDI", {0x5F}, 1, "STACK" },
        { "PUSH R8", {0x41, 0x50}, 2, "STACK" },
        { "POP R8", {0x41, 0x58}, 2, "STACK" },
        { "PUSH imm32", {0x68, 0x00, 0x00, 0x00, 0x00}, 5, "STACK" },
        { "PUSH imm8", {0x6A, 0x00}, 2, "STACK" },
        { "PUSH R64", {0xFF, 0xF0}, 2, "STACK" },
        { "POP R64", {0x8F, 0xC0}, 2, "STACK" },

        /* Flag operations */
        { "CLC", {0xF8}, 1, "FLAG" },
        { "CLD", {0xFC}, 1, "FLAG" },
        { "CLI", {0xFA}, 1, "FLAG" },
        { "STC", {0xF9}, 1, "FLAG" },
        { "STD", {0xFD}, 1, "FLAG" },
        { "STI", {0xFB}, 1, "FLAG" },
        { "CMC", {0xF5}, 1, "FLAG" },
        { "SAHF", {0x9E}, 1, "FLAG" },
        { "LAHF", {0x9F}, 1, "FLAG" },
        { "PUSHFQ", {0x9C}, 1, "FLAG" },
        { "POPFQ", {0x9D}, 1, "FLAG" },

        /* Bit operations */
        { "BT RAX, 3", {0x48, 0x0F, 0xA3, 0xC0}, 4, "BIT" },
        { "BTS RAX, 5", {0x48, 0x0F, 0xAB, 0xC0}, 4, "BIT" },
        { "BTR RAX, 7", {0x48, 0x0F, 0xB3, 0xC0}, 4, "BIT" },
        { "BTC RAX, 9", {0x48, 0x0F, 0xBB, 0xC0}, 4, "BIT" },
        { "BSF RAX, RCX", {0x48, 0x0F, 0xBC, 0xC1}, 4, "BIT" },
        { "BSR RAX, RCX", {0x48, 0x0F, 0xBD, 0xC1}, 4, "BIT" },
        { "POPCNT RAX, RCX", {0xF3, 0x48, 0x0F, 0xB8, 0xC1}, 5, "BIT" },
        { "LZCNT RAX, RCX", {0xF3, 0x48, 0x0F, 0xBD, 0xC1}, 5, "BIT" },
        { "TZCNT RAX, RCX", {0xF3, 0x48, 0x0F, 0xBC, 0xC1}, 5, "BIT" },

        /* Exchange operations */
        { "XCHG RAX, RBX", {0x48, 0x93}, 2, "ALU" },
        { "XCHG RAX, RCX", {0x48, 0x91}, 2, "ALU" },
        { "XCHG RAX, RDX", {0x48, 0x92}, 2, "ALU" },
        { "XCHG R8, R9", {0x4D, 0x31, 0xC8}, 3, "ALU" },

        /* Conditional moves */
        { "CMOVO RAX, RBX", {0x48, 0x0F, 0x40, 0xC3}, 4, "BRANCH" },
        { "CMOVNO RAX, RBX", {0x48, 0x0F, 0x41, 0xC3}, 4, "BRANCH" },
        { "CMOVB RAX, RBX", {0x48, 0x0F, 0x42, 0xC3}, 4, "BRANCH" },
        { "CMOVNB RAX, RBX", {0x48, 0x0F, 0x43, 0xC3}, 4, "BRANCH" },
        { "CMOVZ RAX, RBX", {0x48, 0x0F, 0x44, 0xC3}, 4, "BRANCH" },
        { "CMOVNZ RAX, RBX", {0x48, 0x0F, 0x45, 0xC3}, 4, "BRANCH" },
        { "CMOVS RAX, RBX", {0x48, 0x0F, 0x48, 0xC3}, 4, "BRANCH" },
        { "CMOVNS RAX, RBX", {0x48, 0x0F, 0x49, 0xC3}, 4, "BRANCH" },

        /* SETcc instructions */
        { "SETZO AL", {0x0F, 0x94, 0xC0}, 3, "BRANCH" },
        { "SETNZ CL", {0x0F, 0x95, 0xC1}, 3, "BRANCH" },
        { "SETS DL", {0x0F, 0x98, 0xC2}, 3, "BRANCH" },
        { "SETNS BL", {0x0F, 0x99, 0xC3}, 3, "BRANCH" },

        /* Move with sign/zero extend */
        { "MOVZX EAX, BL", {0x0F, 0xB6, 0xC3}, 3, "ALU" },
        { "MOVZX EAX, BX", {0x0F, 0xB7, 0xC3}, 3, "ALU" },
        { "MOVSX EAX, BL", {0x0F, 0xBE, 0xC3}, 3, "ALU" },
        { "MOVSX EAX, BX", {0x0F, 0xBF, 0xC3}, 3, "ALU" },
        { "MOVSXD RAX, EAX", {0x48, 0x63, 0xC0}, 3, "ALU" },

        /* NOP variants */
        { "NOP", {0x90}, 1, "SPECIAL" },
        { "XCHG EAX, EAX", {0x90}, 1, "SPECIAL" },
        { "NOP dword ptr [RAX]", {0x0F, 0x1F, 0x00}, 3, "SPECIAL" },
        { "NOP dword ptr [RAX+0]", {0x0F, 0x1F, 0x40, 0x00}, 4, "SPECIAL" },

        /* LOCK prefix */
        { "LOCK ADD RAX, RBX", {0xF0, 0x48, 0x01, 0xC3}, 4, "ALU" },
        { "LOCK INC RAX", {0xF0, 0xFF, 0xC0}, 3, "ALU" },
        { "LOCK DEC RAX", {0xF0, 0xFF, 0xC8}, 3, "ALU" },
        { "LOCK AND RAX, RBX", {0xF0, 0x48, 0x21, 0xC3}, 4, "ALU" },
        { "LOCK OR RAX, RBX", {0xF0, 0x48, 0x09, 0xC3}, 4, "ALU" },
        { "LOCK XOR RAX, RBX", {0xF0, 0x48, 0x31, 0xC3}, 4, "ALU" },

        /* Segment operations */
        { "MOV DS, AX", {0x8E, 0xD8}, 2, "SPECIAL" },
        { "MOV ES, AX", {0x8E, 0xC0}, 2, "SPECIAL" },
        { "MOV FS, AX", {0x8E, 0xE0}, 2, "SPECIAL" },
        { "MOV GS, AX", {0x8E, 0xE8}, 2, "SPECIAL" },
        { "MOV SS, AX", {0x8E, 0xD0}, 2, "SPECIAL" },

        /* Control flow */
        { "INT 3", {0xCC}, 1, "SPECIAL" },
        { "INT 0x80", {0xCD, 0x80}, 2, "SPECIAL" },
        { "INT 0x40", {0xCD, 0x40}, 2, "SPECIAL" },
        { "IRETQ", {0x48, 0xCF}, 2, "SPECIAL" },
        { "SYSRET", {0x0F, 0x07}, 2, "SPECIAL" },

        /* Misc */
        { "LEA RAX, [RIP+0x100]", {0x48, 0x8D, 0x05, 0x00, 0x01, 0x00, 0x00}, 7, "MEMORY" },
        { "LEA RAX, [RAX+RBX]", {0x48, 0x8D, 0x04, 0x18}, 4, "MEMORY" },
        { "LEA RAX, [RAX+RBX*4]", {0x48, 0x8D, 0x04, 0x98}, 4, "MEMORY" },
        { "LEA RAX, [RAX+RBX*4+0x100]", {0x48, 0x8D, 0x84, 0x98, 0x00, 0x01, 0x00, 0x00}, 8, "MEMORY" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d instruction patterns for coverage:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        coverage_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%3d] %-20s [%-8s] ", i+1, t->name, t->category);
        for (int j = 0; j < t->length && j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        printf("\\n");

        if (decoded != t->length) {
            printf("      ✗ FAIL: Length %d (expected %d)\\n", decoded, t->length);
            failed++;
        } else {
            printf("      ✓ PASS (%d bytes)\\n", decoded);
            passed++;
        }
    }

    printf("\\n==================================================\\n");
    printf("Coverage Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
