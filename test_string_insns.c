/* Test string instruction support */
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
        /* MOVSB/MOVSW/MOVSD - Move string */
        { "MOVSB", {0xA4}, 1, "Move byte string" },
        { "MOVSW", {0x66, 0xA5}, 2, "Move word string" },
        { "MOVSD", {0xA5}, 1, "Move dword string" },

        /* CMPSB/CMPSW/CMPSD - Compare string */
        { "CMPSB", {0xA6}, 1, "Compare byte string" },
        { "CMPSW", {0x66, 0xA7}, 2, "Compare word string" },
        { "CMPSD", {0xA7}, 1, "Compare dword string" },

        /* SCASB/SCASW/SCASD - Scan string */
        { "SCASB", {0xAE}, 1, "Scan byte string" },
        { "SCASW", {0x66, 0xAF}, 2, "Scan word string" },
        { "SCASD", {0xAF}, 1, "Scan dword string" },

        /* LODSB/LODSW/LODSD - Load string */
        { "LODSB", {0xAC}, 1, "Load byte string" },
        { "LODSW", {0x66, 0xAD}, 2, "Load word string" },
        { "LODSD", {0xAD}, 1, "Load dword string" },

        /* STOSB/STOSW/STOSD - Store string */
        { "STOSB", {0xAA}, 1, "Store byte string" },
        { "STOSW", {0x66, 0xAB}, 2, "Store word string" },
        { "STOSD", {0xAB}, 1, "Store dword string" },

        /* REP prefixes with string ops */
        { "REP MOVSB", {0xF3, 0xA4}, 2, "Repeat move byte string" },
        { "REP MOVSD", {0xF3, 0xA5}, 2, "Repeat move dword string" },
        { "REPE CMPSB", {0xF3, 0xA6}, 2, "Repeat while equal compare byte" },
        { "REPNE SCASB", {0xF2, 0xAE}, 2, "Repeat while not equal scan byte" },

        /* Bit operations - BT, BTS, BTR, BTC */
        { "BTC EAX, 3", {0x0F, 0xBB, 0xD8}, 3, "Bit test and complement" },
        { "BTS EAX, 5", {0x0F, 0xAB, 0xD8}, 3, "Bit test and set" },
        { "BTR EAX, 7", {0x0F, 0xB3, 0xD8}, 3, "Bit test and reset" },
        { "BT EAX, 9", {0x0F, 0xBA, 0xD8, 0x09}, 4, "Bit test with immediate" },

        /* BSF/BSR - Bit scan forward/reverse */
        { "BSF EAX, ECX", {0x0F, 0xBC, 0xC1}, 3, "Bit scan forward" },
        { "BSR EAX, ECX", {0x0F, 0xBD, 0xC1}, 3, "Bit scan reverse" },

        /* POPCNT - Population count */
        { "POPCNT EAX, ECX", {0xF3, 0x0F, 0xB8, 0xC1}, 4, "Population count" },

        /* LZCNT/TZCNT - Leading/trailing zero count */
        { "LZCNT EAX, ECX", {0xF3, 0x0F, 0xBD, 0xC1}, 4, "Leading zero count" },
        { "TZCNT EAX, ECX", {0xF3, 0x0F, 0xBC, 0xC1}, 4, "Trailing zero count" },

        /* I/O instructions */
        { "IN AL, imm8", {0xE4, 0x80}, 2, "Input byte from port" },
        { "IN EAX, imm8", {0xE5, 0x80}, 2, "Input dword from port" },
        { "OUT imm8, AL", {0xE6, 0x80}, 2, "Output byte to port" },
        { "OUT imm8, EAX", {0xE7, 0x80}, 2, "Output dword to port" },

        /* System instructions */
        { "SYSCALL", {0x0F, 0x05}, 2, "Fast system call" },
        { "SYSRET", {0x0F, 0x07}, 2, "Return from fast system call" },
        { "SYSENTER", {0x0F, 0x34}, 2, "Fast system entry" },
        { "SYSEXIT", {0x0F, 0x35}, 2, "Fast system exit" },

        /* Control register access */
        { "MOV CR0, RAX", {0x0F, 0x22, 0xC0}, 3, "Move to control register" },
        { "MOV RAX, CR0", {0x0F, 0x20, 0xC0}, 3, "Move from control register" },

        /* Debug register access */
        { "MOV DR0, RAX", {0x0F, 0x23, 0xC0}, 3, "Move to debug register" },
        { "MOV RAX, DR0", {0x0F, 0x21, 0xC0}, 3, "Move from debug register" },

        /* CPUID */
        { "CPUID", {0x0F, 0xA2}, 2, "CPU identification" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d string, bit, I/O, and system instructions:\n\n", num_tests);

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

            printf("     ✓ PASS\n");
            passed++;
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
