/* System register and control instruction tests */
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
} sysreg_test_t;

int main() {
    int passed = 0, failed = 0;

    sysreg_test_t tests[] = {
        /* Control register access */
        { "MOV to CR0", {0x0F, 0x22, 0xC0}, 3, "MOV CR0, RAX" },
        { "MOV from CR0", {0x0F, 0x20, 0xC0}, 3, "MOV RAX, CR0" },
        { "MOV to CR3", {0x0F, 0x22, 0xD8}, 3, "MOV CR3, RAX" },
        { "MOV from CR3", {0x0F, 0x20, 0xD8}, 3, "MOV RAX, CR3" },
        { "MOV to CR4", {0x0F, 0x22, 0xE0}, 3, "MOV CR4, RAX" },
        { "MOV from CR4", {0x0F, 0x20, 0xE0}, 3, "MOV RAX, CR4" },
        { "MOV to CR8", {0x0F, 0x22, 0xC8}, 3, "MOV CR8, RAX" },
        { "MOV from CR8", {0x0F, 0x20, 0xC8}, 3, "MOV RAX, CR8" },

        /* Debug register access */
        { "MOV to DR0", {0x0F, 0x23, 0xC0}, 3, "MOV DR0, RAX" },
        { "MOV from DR0", {0x0F, 0x21, 0xC0}, 3, "MOV RAX, DR0" },
        { "MOV to DR7", {0x0F, 0x23, 0xF8}, 3, "MOV DR7, RAX" },
        { "MOV from DR7", {0x0F, 0x21, 0xF8}, 3, "MOV RAX, DR7" },

        /* Model-Specific Register access */
        { "RDMSR", {0x0F, 0x32}, 2, "Read MSR (EDX:EAX = MSR[ECX])" },
        { "WRMSR", {0x0F, 0x30}, 2, "Write MSR (MSR[ECX] = EDX:EAX)" },
        { "RDMSR with REX.W", {0x48, 0x0F, 0x32}, 3, "RDMSR with 64-bit operand size" },

        /* Control flow system instructions */
        { "SYSCALL", {0x0F, 0x05}, 2, "System call" },
        { "SYSRET", {0x0F, 0x07}, 2, "System return" },
        { "SYSENTER", {0x0F, 0x34}, 2, "System entry (fast)" },
        { "SYSEXIT", {0x0F, 0x35}, 2, "System exit (fast)" },

        /* Cache control */
        { "WBINVD", {0x0F, 0x09}, 2, "Write-back and invalidate cache" },
        { "INVD", {0x0F, 0x08}, 2, "Invalidate cache" },
        { "CLFLUSH", {0x0F, 0xAE, 0x38}, 3, "Cache line flush" },
        { "CLFLUSHOPT", {0x66, 0x0F, 0xAE, 0x38}, 4, "Cache line flush optimized" },

        /* TLB control */
        { "INVLPG", {0x0F, 0x01, 0x38}, 3, "Invalidate TLB entry" },
        { "INVLPG with mem", {0x0F, 0x01, 0x3D, 0x00, 0x00, 0x00, 0x00}, 7, "INVLPG [RIP+disp32]" },

        /* Memory fencing */
        { "SFENCE", {0x0F, 0xAE, 0xF8}, 3, "Store fence" },
        { "LFENCE", {0x0F, 0xAE, 0xE8}, 3, "Load fence" },
        { "MFENCE", {0x0F, 0xAE, 0xF0}, 3, "Memory fence" },

        /* Processor management */
        { "HLT", {0xF4}, 1, "Halt processor" },
        { "RSM", {0x0F, 0xAA}, 2, "Resume from system management mode" },
        { "MONITOR", {0x0F, 0x01, 0xC8}, 3, "Set up monitor address" },
        { "MWAIT", {0x0F, 0x01, 0xC9}, 3, "Monitor wait" },

        /* Interrupt control */
        { "INT3", {0xCC}, 1, "Breakpoint interrupt" },
        { "INT1", {0xF1}, 1, "Debug interrupt (ICEBP)" },
        { "INT n", {0xCD, 0x80}, 2, "Software interrupt" },
        { "IRETQ", {0x48, 0xCF}, 2, "Interrupt return (64-bit)" },
        { "IRET", {0xCF}, 1, "Interrupt return (legacy)" },

        /* Flag control */
        { "CLC", {0xF8}, 1, "Clear carry flag" },
        { "STC", {0xF9}, 1, "Set carry flag" },
        { "CLI", {0xFA}, 1, "Clear interrupt flag" },
        { "STI", {0xFB}, 1, "Set interrupt flag" },
        { "CLD", {0xFC}, 1, "Clear direction flag" },
        { "STD", {0xFD}, 1, "Set direction flag" },
        { "CMC", {0xF5}, 1, "Complement carry flag" },
        { "SAHF", {0x9E}, 1, "Store AH into flags" },
        { "LAHF", {0x9F}, 1, "Load flags into AH" },
        { "PUSHFQ", {0x9C}, 1, "Push flags (64-bit)" },
        { "POPFQ", {0x9D}, 1, "Pop flags (64-bit)" },

        /* No-operation instructions */
        { "NOP", {0x90}, 1, "No operation" },
        { "NOP dword ptr [RAX]", {0x0F, 0x1F, 0x00}, 3, "Multi-byte NOP" },
        { "PAUSE", {0xF3, 0x90}, 2, "Pause (spin loop hint)" },

        /* Segment management */
        { "MOV to DS", {0x8E, 0xD8}, 2, "MOV DS, EAX" },
        { "MOV from DS", {0x8C, 0xC0}, 2, "MOV EAX, DS" },
        { "MOV to ES", {0x8E, 0xC0}, 2, "MOV ES, EAX" },
        { "MOV to FS", {0x8E, 0xE0}, 2, "MOV FS, EAX" },
        { "MOV to GS", {0x8E, 0xE8}, 2, "MOV GS, EAX" },
        { "MOV to SS", {0x8E, 0xD0}, 2, "MOV SS, EAX" },

        /* Miscellaneous system instructions */
        { "CPUID", {0x0F, 0xA2}, 2, "CPU identification" },
        { "RDTSC", {0x0F, 0x31}, 2, "Read time-stamp counter" },
        { "RDTSCP", {0x0F, 0x01, 0xF9}, 3, "Read TSC and processor ID" },
        { "UD2", {0x0F, 0x0B}, 2, "Undefined instruction" },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    printf("Testing %d system register and control instructions:\\n\\n", num_tests);

    for (int i = 0; i < num_tests; i++) {
        sysreg_test_t *t = &tests[i];
        x86_insn_t insn;
        memset(&insn, 0, sizeof(insn));

        int decoded = decode_x86_insn(t->bytes, &insn);

        printf("[%2d] %-25s ", i+1, t->name);
        for (int j = 0; j < t->length && j < 8; j++) {
            printf("%02X ", t->bytes[j]);
        }
        if (t->length > 8) {
            printf("... ");
        }
        printf("\\n");

        printf("     %s\\n", t->description);
        printf("     Length: %d (expected %d)", decoded, t->length);

        if (decoded != t->length) {
            printf(" ✗ FAIL: Length mismatch\\n\\n");
            failed++;
        } else {
            printf(" ✓ PASS\\n\\n");
            passed++;
        }
    }

    printf("==================================================\\n");
    printf("System Register Test Results: %d/%d tests passed (%.1f%%)\\n",
           passed, num_tests, (100.0 * passed) / num_tests);
    printf("Failed: %d\\n", failed);
    printf("==================================================\\n");

    return failed > 0 ? 1 : 0;
}
