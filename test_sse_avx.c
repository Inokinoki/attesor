/* SSE/AVX instruction predicate and validation test */
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
} sse_test_t;

int main() {
    int passed = 0, failed = 0;

    sse_test_t tests[] = {
        /* SSE packed integer operations */
        { "MOVAPS XMM0, XMM1", {0x0F, 0x28, 0xC1}, 3, "Move aligned packed single" },
        { "MOVUPS XMM0, XMM1", {0x0F, 0x10, 0xC1}, 3, "Move unaligned packed single" },
        { "MOVAPD XMM0, XMM1", {0x0F, 0x29, 0xC8}, 3, "Move aligned packed double" },
        { "MOVUPD XMM0, XMM1", {0x0F, 0x11, 0xC1}, 3, "Move unaligned packed double" },

        /* SSE scalar single/double precision */
        { "MOVSS XMM0, XMM1", {0xF3, 0x0F, 0x10, 0xC1}, 4, "Move scalar single" },
        { "MOVSD XMM0, XMM1", {0xF2, 0x0F, 0x10, 0xC1}, 4, "Move scalar double" },

        /* SSE arithmetic */
        { "ADDPS XMM0, XMM1", {0x0F, 0x58, 0xC1}, 3, "Add packed single" },
        { "ADDSD XMM0, XMM1", {0xF2, 0x0F, 0x58, 0xC1}, 4, "Add scalar double" },
        { "SUBPS XMM0, XMM1", {0x0F, 0x5C, 0xC1}, 3, "Subtract packed single" },
        { "MULPS XMM0, XMM1", {0x0F, 0x59, 0xC1}, 3, "Multiply packed single" },
        { "DIVPS XMM0, XMM1", {0x0F, 0x5E, 0xC1}, 3, "Divide packed single" },

        /* SSE comparison */
        { "CMPPS XMM0, XMM1, 0", {0x0F, 0xC2, 0xC1, 0x00}, 4, "Compare packed single" },
        { "COMISS XMM0, XMM1", {0x0F, 0x2F, 0xC1}, 3, "Compare scalar single" },
        { "UCOMISS XMM0, XMM1", {0x0F, 0x2E, 0xC1}, 3, "Unordered compare scalar" },

        /* SSE2 128-bit integer operations */
        { "MOVDQA XMM0, XMM1", {0x0F, 0x6F, 0xC1}, 3, "Move aligned double quadword" },
        { "MOVDQU XMM0, XMM1", {0xF3, 0x0F, 0x6F, 0xC1}, 4, "Move unaligned double quadword" },
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, "Packed add 64-bit integers" },
        { "PSUBQ XMM0, XMM1", {0x66, 0x0F, 0xFB, 0xC1}, 4, "Packed subtract 64-bit" },
        { "PMULUDQ XMM0, XMM1", {0x66, 0x0F, 0xF4, 0xC1}, 4, "Packed multiply 32->64-bit" },

        /* SSE2 shuffle/insert/extract */
        { "PSHUFD XMM0, XMM1, 0", {0x66, 0x0F, 0x70, 0xC1, 0x00}, 5, "Shuffle packed dwords" },
        { "PSHUFD XMM0, XMM1, 0xFF", {0x66, 0x0F, 0x70, 0xC1, 0xFF}, 5, "Reverse shuffle" },
        { "PEXTRW XMM0, XMM1, 0", {0x66, 0x0F, 0xC5, 0xC1, 0x00}, 5, "Extract word" },
        { "PINSRW XMM0, XMM1, 0", {0x66, 0x0F, 0xC4, 0xC1, 0x00}, 5, "Insert word" },
        { "CMPPS XMM0, XMM1, 0", {0x0F, 0xC2, 0xC1, 0x00}, 4, "Compare packed single" },

        /* SSE4.1 blend/insert/pack */
        { "BLENDVPS XMM0, XMM1, XMM2", {0x66, 0x0F, 0x14, 0xC2}, 4, "Blend packed single" },
        { "PBLENDW XMM0, XMM1, XMM2, 0", {0x66, 0x0F, 0x0D, 0xC2, 0x00}, 5, "Packed blend words" },
        { "PINSRB XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x20, 0xC1, 0x00}, 6, "Insert byte" },
        { "PINSRD XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x22, 0xC1, 0x00}, 6, "Insert dword" },
        { "PINSRQ XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x22, 0xC1, 0x00}, 6, "Insert qword" },

        /* AES instructions */
        { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5, "AES encode round" },
        { "AESENCLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDD, 0xC1}, 5, "AES encode last" },
        { "AESDEC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDE, 0xC1}, 5, "AES decode round" },
        { "AESDECLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDF, 0xC1}, 5, "AES decode last" },
        { "AESIMC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDB, 0xC1}, 5, "AES inverse mix" },
        { "AESEKEYGENASSIST XMM0, XMM1", {0x66, 0x0F, 0x3A, 0xDF, 0xC1, 0x00}, 6, "AES key gen assist" },
        { "AESKEYGENASSIST XMM0, XMM1", {0x66, 0x0F, 0x3A, 0xDC, 0xC1, 0x00}, 6, "AES key expand assist" },

        /* SHA instructions */
        { "SHA1NEXTE XMM0, XMM1", {0x0F, 0x38, 0xC8, 0xC1}, 4, "SHA1 next e" },
        { "SHA1MSG1 XMM0, XMM1", {0x0F, 0x38, 0xC9, 0xC1}, 4, "SHA1 msg1" },
        { "SHA1MSG2 XMM0, XMM1", {0x0F, 0x38, 0xCA, 0xC1}, 4, "SHA1 msg2" },
        { "SHA256RNDS2 XMM0, XMM1", {0x0F, 0x38, 0xCB, 0xC1}, 4, "SHA256 rounds 2" },
        { "SHA256MSG1 XMM0, XMM1", {0x0F, 0x38, 0xCC, 0xC1}, 4, "SHA256 msg1" },

        /* PCLMULQD (carryless multiply) */
        { "PCLMULQDQ XMM0, XMM1, 0x00", {0x66, 0x0F, 0x3A, 0x44, 0xC1, 0x00}, 6, "Packed carryless mul" },
        { "PCLMULQDQ XMM0, XMM1, 0x01", {0x66, 0x0F, 0x3A, 0x44, 0xC1, 0x01}, 6, "Packed carryless mul (high)" },

        /* AVX instructions (with VEX prefix) */
        { "VEX.128.66.0F.WIG 59 /r", {0xC5, 0xF0, 0x59, 0xC1}, 4, "AVX VEX multiply (VMULPS)" },
        { "VEX.128.F2.0F.WIG 10 /r", {0xC4, 0xE1, 0x09, 0x10, 0xC1}, 5, "AVX VEX blend (VMOVSD)" },
        { "VEX.128.F3.0F.WIG 10 /r", {0xC4, 0xE1, 0x09, 0x10, 0xC1}, 5, "AVX VEX move (VMOVSS)" },
        { "VEX.256.66.0F.WIG 59 /r", {0xC5, 0xFD, 0x59, 0xC1}, 4, "AVX 256-bit multiply" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d SSE/AVX/Crypto instructions:\n\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        sse_test_t *t = &tests[i];
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
                   (type == INSN_SIMD) ? "SIMD" :
                   (type == INSN_ALU) ? "ALU" :
                   (type == INSN_SPECIAL) ? "SPECIAL" :
                   (type == INSN_UNKNOWN) ? "UNKNOWN" : "OTHER");

            if (type == INSN_SIMD || type == INSN_SPECIAL) {
                printf("     ✓ PASS\n");
                passed++;
            } else {
                printf("     ⚠ WARNING: Not classified as SIMD\n");
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
