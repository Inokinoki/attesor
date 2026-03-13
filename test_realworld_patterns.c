/* Real-world instruction pattern test */
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
} realworld_test_t;

int main() {
    int passed = 0, failed = 0;

    realworld_test_t tests[] = {
        /* Common compiler patterns (individual instructions) */
        { "PUSH RBP", {0x55}, 1, "Stack frame setup" },
        { "MOV RBP, RSP", {0x48, 0x89, 0xE5}, 3, "Save stack pointer" },
        { "POP RBP", {0x5D}, 1, "Stack frame restore" },
        { "RET", {0xC3}, 1, "Return" },
        { "SUB RSP, imm8", {0x48, 0x83, 0xEC, 0x10}, 4, "Allocate stack space" },

        /* Stack frame operations (individual instructions) */
        { "Stack allocation", {0x48, 0x81, 0xEC, 0x00, 0x01, 0x00, 0x00}, 7, "Large stack allocation" },
        { "Stack deallocation", {0x48, 0x81, 0xC4, 0x00, 0x01, 0x00, 0x00}, 7, "Large stack deallocation" },
        { "PUSH R12", {0x41, 0x54}, 2, "Save R12" },
        { "PUSH R13", {0x41, 0x55}, 2, "Save R13" },
        { "PUSH R14", {0x41, 0x56}, 2, "Save R14" },
        { "PUSH R15", {0x41, 0x57}, 2, "Save R15" },
        { "POP R15", {0x41, 0x5F}, 2, "Restore R15" },
        { "POP R14", {0x41, 0x5E}, 2, "Restore R14" },
        { "POP R13", {0x41, 0x5D}, 2, "Restore R13" },
        { "POP R12", {0x41, 0x5C}, 2, "Restore R12" },

        /* Common arithmetic sequences (individual instructions) */
        { "Integer compare", {0x48, 0x39, 0xC3}, 3, "Compare RAX, RBX" },
        { "TEST RAX,RAX", {0x48, 0x85, 0xC0}, 3, "Test RAX against itself" },
        { "JE disp8", {0x74, 0x10}, 2, "Jump if equal" },
        { "DEC RAX", {0x48, 0xFF, 0xC8}, 3, "Decrement RAX" },
        { "JNE disp8", {0x75, 0xFC}, 2, "Jump if not equal (loop)" },

        /* Memory access patterns */
        { "Array access", {0x48, 0x8B, 0x04, 0xC3}, 4, "MOV RAX, [RBX+RCX*8]" },
        { "Struct access", {0x48, 0x8B, 0x83, 0x20, 0x01, 0x00, 0x00}, 7, "MOV RAX, [RBX+0x120]" },
        { "RIP-relative load", {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00}, 7, "MOV RAX, [RIP+disp32]" },

        /* SIMD common patterns */
        { "Zero XMM", {0x0F, 0x57, 0xC0}, 3, "XORPS XMM0, XMM0" },
        { "SSE load", {0x0F, 0x28, 0x07}, 3, "MOVAPS XMM0, [RDI]" },
        { "SSE store", {0x0F, 0x29, 0x07}, 3, "MOVAPS [RDI], XMM0" },
        { "AVX load", {0xC5, 0xF8, 0x7F, 0x07}, 4, "VMOVDQU XMM0, [RDI]" },
        { "AVX store", {0xC5, 0xF8, 0x7F, 0x07}, 4, "VMOVDQU [RDI], XMM0" },

        /* Branch prediction hints */
        { "Static prediction", {0x2E, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00}, 7, "Branch taken hint" },
        { "Segment override", {0x3E, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00}, 7, "DS segment override" },

        /* Locked operations */
        { "Locked add", {0xF0, 0x48, 0x01, 0x07}, 4, "LOCK ADD [RDI], RAX" },
        { "Locked xchg", {0xF0, 0x48, 0x87, 0x07}, 4, "LOCK XCHG [RDI], RAX" },
        { "Locked cmpxchg", {0xF0, 0x48, 0x0F, 0xB1, 0x07}, 5, "LOCK CMPXCHG [RDI], RAX" },

        /* String operations */
        { "Memset pattern", {0x48, 0x89, 0xD9, 0xB9, 0x10, 0x00, 0x00, 0x00, 0xF3, 0xAA}, 10, "MEMCL (stosq loop)" },
        { "Memcpy prologue", {0x48, 0x89, 0xF8, 0x48, 0x89, 0xD7}, 6, "Save RCX, save RDI" },
        { "Memcpy loop", {0xF3, 0x48, 0xA5}, 3, "REP MOVSQ" },

        /* Bit manipulation patterns (individual instructions) */
        { "BT RAX, RCX", {0x48, 0x0F, 0xA3, 0xC1}, 4, "Bit test" },
        { "JNC disp8", {0x73, 0x02}, 2, "Jump if no carry" },
        { "BSF RAX, RCX", {0x48, 0x0F, 0xBC, 0xC1}, 4, "Bit scan forward" },
        { "JNZ disp8", {0x75, 0x02}, 2, "Jump if not zero" },

        /* 64-bit INC/DEC (use ModR/M, not legacy single-byte) */
        { "INC RAX", {0xFF, 0xC0}, 2, "Increment RAX" },
        { "DEC RAX", {0xFF, 0xC8}, 2, "Decrement RAX" },
        { "INC R8", {0x49, 0xFF, 0xC0}, 3, "Increment R8 (REX.B + FF /0)" },
        { "DEC R8", {0x49, 0xFF, 0xC8}, 3, "Decrement R8 (REX.B + FF /1)" },

        /* Immediate variants */
        { "ADD imm8", {0x83, 0xC0, 0x01}, 3, "ADD RAX, 1" },
        { "ADD imm32", {0x81, 0xC0, 0x01, 0x00, 0x00, 0x00}, 6, "ADD RAX, 1" },
        { "PUSH imm64", {0x48, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50}, 10, "MOV imm64 + PUSH" },
        { "MOV imm64", {0x48, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 10, "MOV RAX, imm64" },

        /* Conditional variants */
        { "CMOV overflow", {0x48, 0x0F, 0x40, 0xC3}, 4, "CMOVO RAX, RBX" },
        { "SET overflow", {0x48, 0x0F, 0x90, 0xC3}, 4, "SETO RAX" },
        { "JECXZ", {0x67, 0xE3, 0x00}, 3, "Jump if ECX zero (32-bit)" },
        { "JRCXZ", {0xE3, 0x00}, 2, "Jump if RCX zero (64-bit)" },

        /* Multi-prefix instructions */
        { "LOCK REP ADD", {0xF0, 0xF3, 0x01, 0x07}, 4, "Multiple prefixes" },
        { "Data16 LOCK", {0x66, 0xF0, 0x01, 0x07}, 4, "Operand size + LOCK" },

        /* SSE/AVX with immediates */
        { "PSHUFD", {0x66, 0x0F, 0x70, 0xC1, 0x00}, 5, "PSHUFD XMM0, XMM1, 0" },
        { "PSHUFHW", {0xF3, 0x0F, 0x70, 0xC1, 0x00}, 5, "PSHUFHW XMM0, XMM1, 0" },
        { "PSHUFLW", {0xF2, 0x0F, 0x70, 0xC1, 0x00}, 5, "PSHUFLW XMM0, XMM1, 0" },
        { "BLENDPD", {0x66, 0x0F, 0x3A, 0x0D, 0xC1, 0x00}, 6, "BLENDPD with immediate" },

        /* Crypto patterns */
        { "AES round", {0x66, 0x0F, 0x38, 0xDC, 0xC1, 0x66, 0x0F, 0x38, 0xDC, 0xC2}, 10, "AESENC x2" },
        { "SHA-256 block", {0x0F, 0x38, 0xCC, 0xC1, 0x0F, 0x38, 0xCB, 0xC1}, 8, "SHA256RNDS2 + SHA256MSG1" },

        /* Extended registers */
        { "R8-R15 operations", {0x41, 0x50, 0x41, 0x58}, 4, "PUSH R8 + POP R8" },
        { "Extended INC", {0x49, 0xFF, 0xC0}, 3, "INC R8" },
        { "Extended MOV", {0x49, 0x89, 0xC1}, 3, "MOV R8, RAX" },
        { "Extended XCHG", {0x49, 0x87, 0xC1}, 3, "XCHG R8, RAX" },

        /* Complex addressing */
        { "Scaled index", {0x48, 0x8B, 0x04, 0x93}, 4, "MOV RAX, [RBX+RDX*4]" },
        { "Complex SIB", {0x48, 0x8B, 0x84, 0x93, 0x10, 0x00, 0x00, 0x00}, 8, "MOV RAX, [RBX+RDX*4+0x10]" },
        { "RIP-relative", {0x48, 0x8B, 0x05, 0x12, 0x34, 0x56, 0x78}, 7, "MOV RAX, [RIP+0x78563412]" },

        /* Control transfer */
        { "Tail call", {0xE9, 0x00, 0x00, 0x00, 0x00}, 5, "JMP to function (tail call)" },
        { "Indirect call", {0xFF, 0xD0}, 2, "CALL RAX" },
        { "Indirect jump", {0xFF, 0xE0}, 2, "JMP RAX" },
        { "Return with imm", {0xC2, 0x08, 0x00}, 3, "RET 0x8" },

        /* Flag operations */
        { "Clear flag", {0x9C, 0x9D}, 2, "PUSHF + POPF" },
        { "Set flag", {0xF9, 0x9E}, 2, "STC + SAHF" },
        { "Complement flag", {0xF5}, 1, "CMC" },

        /* Boundary/edge cases */
        { "Maximum length", {0x48, 0x81, 0xC4, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11}, 11, "Long ADD instruction" },
        { "All prefixes", {0x66, 0xF2, 0xF3, 0xF0, 0x48, 0x01, 0x07}, 7, "Multiple prefixes" },

        /* Segment operations */
        { "Segment load", {0x8E, 0xD8}, 2, "MOV DS, EAX" },
        { "Segment store", {0x8C, 0xC0}, 2, "MOV EAX, DS" },
        { "Far jump", {0xFF, 0x2D, 0x00, 0x00, 0x00, 0x00}, 6, "JMP ptr32:32" },
        { "Far call", {0xFF, 0x15, 0x00, 0x00, 0x00, 0x00}, 6, "CALL ptr32:32" },

        /* I/O operations */
        { "Port I/O", {0xE4, 0x80}, 2, "IN AL, 0x80" },
        { "Port out", {0xE6, 0x80}, 2, "OUT 0x80, AL" },
        { "String I/O", {0x6C, 0x6E}, 2, "INSB + OUTSB" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d real-world instruction patterns:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        realworld_test_t *t = &tests[i];

        printf("[%2d] %-25s ", i+1, t->name);
        for (int j = 0; j < t->length && j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        if (t->length > 8) {
            printf("... ");
        }
        printf("\\n");

        printf("     %s\\n", t->description);

        /* Decode all instructions in the sequence */
        int offset = 0;
        int insn_count = 0;
        int all_pass = 1;

        while (offset < t->length) {
            x86_insn_t insn;
            memset(&insn, 0, sizeof(insn));

            int decoded = decode_x86_insn(t->bytes + offset, &insn);

            if (decoded == 0 || offset + decoded > t->length) {
                printf("     ✗ FAIL: Cannot decode at offset %d\\n", offset);
                all_pass = 0;
                break;
            }

            offset += decoded;
            insn_count++;
        }

        printf("     Instructions: %d, Total bytes: %d (expected %d)", insn_count, offset, t->length);

        if (all_pass && offset == t->length) {
            printf(" ✓ PASS\\n\\n");
            passed++;
        } else {
            printf(" ✗ FAIL\\n\\n");
            failed++;
        }
    }

    printf("==================================================\\n");
    printf("Real-World Pattern Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
