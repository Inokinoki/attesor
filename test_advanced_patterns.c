/* Advanced instruction pattern test */
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
} advanced_test_t;

int main() {
    int passed = 0, failed = 0;

    advanced_test_t tests[] = {
        /* More SSE instructions */
        { "ADDSD XMM0, XMM1", {0xF2, 0x0F, 0x58, 0xC1}, 4, "SSE" },
        { "ADDSS XMM0, XMM1", {0xF3, 0x0F, 0x58, 0xC1}, 4, "SSE" },
        { "SUBSD XMM0, XMM1", {0xF2, 0x0F, 0x5C, 0xC1}, 4, "SSE" },
        { "SUBSS XMM0, XMM1", {0xF3, 0x0F, 0x5C, 0xC1}, 4, "SSE" },
        { "MULSD XMM0, XMM1", {0xF2, 0x0F, 0x59, 0xC1}, 4, "SSE" },
        { "MULSS XMM0, XMM1", {0xF3, 0x0F, 0x59, 0xC1}, 4, "SSE" },
        { "DIVSD XMM0, XMM1", {0xF2, 0x0F, 0x5E, 0xC1}, 4, "SSE" },
        { "DIVSS XMM0, XMM1", {0xF3, 0x0F, 0x5E, 0xC1}, 4, "SSE" },

        /* SSE compare instructions */
        { "CMPSD XMM0, XMM1, 0", {0xF2, 0x0F, 0xC2, 0xC1, 0x00}, 5, "SSE" },
        { "CMPSS XMM0, XMM1, 0", {0xF3, 0x0F, 0xC2, 0xC1, 0x00}, 5, "SSE" },
        { "CMPPD XMM0, XMM1, 0", {0x66, 0x0F, 0xC2, 0xC1, 0x00}, 5, "SSE" },
        { "CMPPS XMM0, XMM1, 0", {0x0F, 0xC2, 0xC1, 0x00}, 4, "SSE" },

        /* SSE conversion instructions */
        { "CVTPS2PD XMM0, XMM1", {0x0F, 0x5A, 0xC1}, 3, "SSE" },
        { "CVTPD2PS XMM0, XMM1", {0x66, 0x0F, 0x5A, 0xC1}, 4, "SSE" },
        { "CVTSS2SD XMM0, XMM1", {0xF3, 0x0F, 0x5A, 0xC1}, 4, "SSE" },
        { "CVTSD2SS XMM0, XMM1", {0xF2, 0x0F, 0x5A, 0xC1}, 4, "SSE" },
        { "CVTDQ2PS XMM0, XMM1", {0x0F, 0x5B, 0xC1}, 3, "SSE" },
        { "CVTPS2DQ XMM0, XMM1", {0x66, 0x0F, 0x5B, 0xC1}, 4, "SSE" },
        { "CVTTPS2DQ XMM0, XMM1", {0xF3, 0x0F, 0x5B, 0xC1}, 4, "SSE" },

        /* SSE square root */
        { "SQRTPD XMM0, XMM1", {0x66, 0x0F, 0x51, 0xC1}, 4, "SSE" },
        { "SQRTPS XMM0, XMM1", {0x0F, 0x51, 0xC1}, 3, "SSE" },
        { "SQRTSD XMM0, XMM1", {0xF2, 0x0F, 0x51, 0xC1}, 4, "SSE" },
        { "SQRTSS XMM0, XMM1", {0xF3, 0x0F, 0x51, 0xC1}, 4, "SSE" },

        /* SSE min/max */
        { "MINPD XMM0, XMM1", {0x66, 0x0F, 0x5D, 0xC1}, 4, "SSE" },
        { "MINPS XMM0, XMM1", {0x0F, 0x5D, 0xC1}, 3, "SSE" },
        { "MAXPD XMM0, XMM1", {0x66, 0x0F, 0x5F, 0xC1}, 4, "SSE" },
        { "MAXPS XMM0, XMM1", {0x0F, 0x5F, 0xC1}, 3, "SSE" },

        /* SSE shuffle */
        { "SHUFPS XMM0, XMM1, 0", {0x0F, 0xC6, 0xC1, 0x00}, 4, "SSE" },
        { "SHUFPD XMM0, XMM1, 0", {0x66, 0x0F, 0xC6, 0xC1, 0x00}, 5, "SSE" },
        { "PSHUFD XMM0, XMM1, 0", {0x66, 0x0F, 0x70, 0xC1, 0x00}, 5, "SSE" },
        { "PSHUFHW XMM0, XMM1, 0", {0xF3, 0x0F, 0x70, 0xC1, 0x00}, 5, "SSE" },
        { "PSHUFLW XMM0, XMM1, 0", {0xF2, 0x0F, 0x70, 0xC1, 0x00}, 5, "SSE" },

        /* SSE unpack */
        { "PUNPCKLBW XMM0, XMM1", {0x66, 0x0F, 0x60, 0xC1}, 4, "SSE" },
        { "PUNPCKLWD XMM0, XMM1", {0x66, 0x0F, 0x61, 0xC1}, 4, "SSE" },
        { "PUNPCKLDQ XMM0, XMM1", {0x66, 0x0F, 0x62, 0xC1}, 4, "SSE" },
        { "PUNPCKLQDQ XMM0, XMM1", {0x66, 0x0F, 0x6C, 0xC1}, 4, "SSE" },
        { "PUNPCKHBW XMM0, XMM1", {0x66, 0x0F, 0x68, 0xC1}, 4, "SSE" },
        { "PUNPCKHWD XMM0, XMM1", {0x66, 0x0F, 0x69, 0xC1}, 4, "SSE" },
        { "PUNPCKHDQ XMM0, XMM1", {0x66, 0x0F, 0x6A, 0xC1}, 4, "SSE" },
        { "PUNPCKHQDQ XMM0, XMM1", {0x66, 0x0F, 0x6D, 0xC1}, 4, "SSE" },

        /* SSE packed integer arithmetic */
        { "PADDB XMM0, XMM1", {0x66, 0x0F, 0xFC, 0xC1}, 4, "SSE" },
        { "PADDW XMM0, XMM1", {0x66, 0x0F, 0xFD, 0xC1}, 4, "SSE" },
        { "PADDD XMM0, XMM1", {0x66, 0x0F, 0xFE, 0xC1}, 4, "SSE" },
        { "PADDQ XMM0, XMM1", {0x66, 0x0F, 0xD4, 0xC1}, 4, "SSE" },
        { "PSUBB XMM0, XMM1", {0x66, 0x0F, 0xF8, 0xC1}, 4, "SSE" },
        { "PSUBW XMM0, XMM1", {0x66, 0x0F, 0xF9, 0xC1}, 4, "SSE" },
        { "PSUBD XMM0, XMM1", {0x66, 0x0F, 0xFA, 0xC1}, 4, "SSE" },
        { "PSUBQ XMM0, XMM1", {0x66, 0x0F, 0xFB, 0xC1}, 4, "SSE" },

        /* SSE logical */
        { "PAND XMM0, XMM1", {0x66, 0x0F, 0xDB, 0xC1}, 4, "SSE" },
        { "PANDN XMM0, XMM1", {0x66, 0x0F, 0xDF, 0xC1}, 4, "SSE" },
        { "POR XMM0, XMM1", {0x66, 0x0F, 0xEB, 0xC1}, 4, "SSE" },
        { "PXOR XMM0, XMM1", {0x66, 0x0F, 0xEF, 0xC1}, 4, "SSE" },

        /* More AVX instructions */
        { "VADDPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x58, 0xC2}, 4, "AVX" },
        { "VADDSD XMM0, XMM1, XMM2", {0xC5, 0xF1, 0x58, 0xC2}, 4, "AVX" },
        { "VMULPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x59, 0xC2}, 4, "AVX" },
        { "VMULSD XMM0, XMM1, XMM2", {0xC5, 0xF1, 0x59, 0xC2}, 4, "AVX" },
        { "VSUBPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x5C, 0xC2}, 4, "AVX" },
        { "VSUBSD XMM0, XMM1, XMM2", {0xC5, 0xF1, 0x5C, 0xC2}, 4, "AVX" },
        { "VDIVPS XMM0, XMM1, XMM2", {0xC5, 0xF0, 0x5E, 0xC2}, 4, "AVX" },
        { "VDIVSD XMM0, XMM1, XMM2", {0xC5, 0xF1, 0x5E, 0xC2}, 4, "AVX" },

        /* AVX compare */
        { "VCMPPS XMM0, XMM1, XMM2, 0", {0xC5, 0xF0, 0xC2, 0xC2, 0x00}, 5, "AVX" },
        { "VCMPPD XMM0, XMM1, XMM2, 0", {0xC5, 0xF1, 0xC2, 0xC2, 0x00}, 5, "AVX" },

        /* AES instructions */
        { "AESENC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDC, 0xC1}, 5, "AES" },
        { "AESENCLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDD, 0xC1}, 5, "AES" },
        { "AESDEC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDE, 0xC1}, 5, "AES" },
        { "AESDECLAST XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDF, 0xC1}, 5, "AES" },
        { "AESIMC XMM0, XMM1", {0x66, 0x0F, 0x38, 0xDB, 0xC1}, 5, "AES" },
        { "AESKEYGENASSIST XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0xDF, 0xC1, 0x00}, 6, "AES" },

        /* SHA instructions */
        { "SHA1RNDS4 XMM0, XMM1, 0", {0x0F, 0x3A, 0xCC, 0xC1, 0x00}, 5, "SHA" },
        { "SHA1NEXTE XMM0, XMM1", {0x0F, 0x38, 0xC8, 0xC1}, 4, "SHA" },
        { "SHA1MSG1 XMM0, XMM1", {0x0F, 0x38, 0xC9, 0xC1}, 4, "SHA" },
        { "SHA1MSG2 XMM0, XMM1", {0x0F, 0x38, 0xCA, 0xC1}, 4, "SHA" },
        { "SHA256RNDS2 XMM0, XMM1", {0x0F, 0x38, 0xCB, 0xC1}, 4, "SHA" },
        { "SHA256MSG1 XMM0, XMM1", {0x0F, 0x38, 0xCC, 0xC1}, 4, "SHA" },
        { "SHA256MSG2 XMM0, XMM1", {0x0F, 0x38, 0xCD, 0xC1}, 4, "SHA" },

        /* PCLMULQDQ */
        { "PCLMULQDQ XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x44, 0xC1, 0x00}, 6, "PCLMUL" },

        /* SSE4.1 instructions */
        { "BLENDPD XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x0D, 0xC1, 0x00}, 6, "SSE4.1" },
        { "BLENDPS XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x0C, 0xC1, 0x00}, 6, "SSE4.1" },
        { "PBLENDW XMM0, XMM1, 0", {0x66, 0x0F, 0x0D, 0xC1, 0x00}, 5, "SSE4.1" },
        { "PBLENDVB XMM0, XMM1", {0x66, 0x0F, 0x38, 0x10, 0xC1}, 5, "SSE4.1" },
        { "PMINSB XMM0, XMM1", {0x66, 0x0F, 0x38, 0x38, 0xC1}, 5, "SSE4.1" },
        { "PMINSD XMM0, XMM1", {0x66, 0x0F, 0x38, 0x39, 0xC1}, 5, "SSE4.1" },
        { "PMINUW XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3A, 0xC1}, 5, "SSE4.1" },
        { "PMINUD XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3B, 0xC1}, 5, "SSE4.1" },
        { "PMAXSB XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3C, 0xC1}, 5, "SSE4.1" },
        { "PMAXSD XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3D, 0xC1}, 5, "SSE4.1" },
        { "PMAXUW XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3E, 0xC1}, 5, "SSE4.1" },
        { "PMAXUD XMM0, XMM1", {0x66, 0x0F, 0x38, 0x3F, 0xC1}, 5, "SSE4.1" },

        /* SSE4.2 instructions */
        { "PCMPESTRM XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x60, 0xC1, 0x00}, 6, "SSE4.2" },
        { "PCMPESTRI XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x61, 0xC1, 0x00}, 6, "SSE4.2" },
        { "PCMPISTRM XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x62, 0xC1, 0x00}, 6, "SSE4.2" },
        { "PCMPISTRI XMM0, XMM1, 0", {0x66, 0x0F, 0x3A, 0x63, 0xC1, 0x00}, 6, "SSE4.2" },
        { "PCMPGTQ XMM0, XMM1", {0x66, 0x0F, 0x38, 0x37, 0xC1}, 5, "SSE4.2" },

        /* Complex addressing modes */
        { "MOV RAX, [RAX]", {0x48, 0x8B, 0x00}, 3, "MEMORY" },
        { "MOV RAX, [RAX+RBX]", {0x48, 0x8B, 0x04, 0x18}, 4, "MEMORY" },
        { "MOV RAX, [RAX+RBX*2]", {0x48, 0x8B, 0x04, 0x40}, 4, "MEMORY" },
        { "MOV RAX, [RAX+RBX*4]", {0x48, 0x8B, 0x04, 0x98}, 4, "MEMORY" },
        { "MOV RAX, [RAX+RBX*8]", {0x48, 0x8B, 0x04, 0xD8}, 4, "MEMORY" },
        { "MOV RAX, [RAX+RBX+0x10]", {0x48, 0x8B, 0x44, 0x18, 0x10}, 5, "MEMORY" },
        { "MOV RAX, [RAX+RBX*4+0x10]", {0x48, 0x8B, 0x84, 0x98, 0x10, 0x00, 0x00, 0x00}, 8, "MEMORY" },

        /* Boundary cases - single byte instructions */
        { "AAA", {0x37}, 1, "ALU" },
        { "AAD", {0xD5, 0x0A}, 2, "ALU" },
        { "AAM", {0xD4, 0x0A}, 2, "ALU" },
        { "AAS", {0x3F}, 1, "ALU" },
        { "CBW", {0x66, 0x98}, 2, "ALU" },
        { "CWDE", {0x98}, 1, "ALU" },
        { "CDQE", {0x48, 0x98}, 2, "ALU" },
        { "CWD", {0x66, 0x99}, 2, "ALU" },
        { "CDQ", {0x99}, 1, "ALU" },
        { "CQO", {0x48, 0x99}, 2, "ALU" },
        { "DAA", {0x27}, 1, "ALU" },
        { "DAS", {0x2F}, 1, "ALU" },
        { "INTO", {0xCE}, 1, "SPECIAL" },
        { "XLAT", {0xD7}, 1, "SPECIAL" },
        { "NOP", {0x90}, 1, "SPECIAL" },
        { "WAIT", {0x9B}, 1, "SPECIAL" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d advanced instruction patterns:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        advanced_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%3d] %-28s [%-8s] ", i+1, t->name, t->category);
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
    printf("Advanced Pattern Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
