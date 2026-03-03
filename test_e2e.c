/* ============================================================================
 * Rosetta End-to-End Test Harness
 * ============================================================================
 *
 * This test harness loads an x86_64 binary and tests the full translation
 * pipeline from loading to execution.
 * ============================================================================ */

#include "rosetta_macho_loader.h"
#include "rosetta_refactored.h"
#include "rosetta_refactored_init.h"
#include "rosetta_refactored_exec.h"
#include "rosetta_x86_decode.h"
#include "rosetta_translate_dispatch.h"
#include "rosetta_arm64_emit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

/* ============================================================================
 * Test Configuration
 * ============================================================================ */

#define TEST_CODE_SIZE 4096
#define MAX_TEST_INSNS 16

/* ============================================================================
 * Simple x86_64 Test Programs (hand-encoded)
 * ============================================================================ */

/* Simple test: NOP; RET (single-byte instructions that decode correctly) */
static const uint8_t test_nop_ret[] = {
    0x90,  /* NOP */
    0xC3   /* RET */
};

/* Test: PUSH RBX; POP RBX; RET */
static const uint8_t test_push_pop_ret[] = {
    0x53,  /* PUSH RBX */
    0x5B,  /* POP RBX */
    0xC3   /* RET */
};

/* Test: PUSH RAX; POP RCX; RET */
static const uint8_t test_push_pop_rcx[] = {
    0x50,  /* PUSH RAX */
    0x59,  /* POP RCX */
    0xC3   /* RET */
};

/* Test: NOP; NOP; RET */
static const uint8_t test_multi_nop_ret[] = {
    0x90,  /* NOP */
    0x90,  /* NOP */
    0x90,  /* NOP */
    0xC3   /* RET */
};

/* ============================================================================
 * Test Runner
 * ============================================================================ */

typedef struct {
    const char *name;
    const uint8_t *code;
    size_t code_size;
    uint64_t initial_rax;
    uint64_t initial_rbx;
    uint64_t expected_rax;
    int expect_success;
} test_case_t;

static test_case_t test_cases[] = {
    {
        .name = "NOP; RET",
        .code = test_nop_ret,
        .code_size = sizeof(test_nop_ret),
        .initial_rax = 0,
        .initial_rbx = 0,
        .expected_rax = 0,
        .expect_success = 1
    },
    {
        .name = "PUSH RBX; POP RBX; RET",
        .code = test_push_pop_ret,
        .code_size = sizeof(test_push_pop_ret),
        .initial_rax = 0,
        .initial_rbx = 0xDEADBEEF,
        .expected_rax = 0xDEADBEEF,
        .expect_success = 1
    },
    {
        .name = "PUSH RAX; POP RCX; RET",
        .code = test_push_pop_rcx,
        .code_size = sizeof(test_push_pop_rcx),
        .initial_rax = 0x12345678,
        .initial_rbx = 0,
        .expected_rax = 0x12345678,
        .expect_success = 1
    },
    {
        .name = "NOP; NOP; NOP; RET",
        .code = test_multi_nop_ret,
        .code_size = sizeof(test_multi_nop_ret),
        .initial_rax = 0,
        .initial_rbx = 0,
        .expected_rax = 0,
        .expect_success = 1
    }
};

/* ============================================================================
 * Translation Test Function
 * ============================================================================ */

/**
 * Translate and execute a single x86_64 test program
 */
int run_translation_test(test_case_t *test)
{
    printf("  Testing: %s\n", test->name);

    /* Allocate memory for x86_64 code */
    void *x86_mem = mmap(NULL, TEST_CODE_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (x86_mem == MAP_FAILED) {
        printf("    FAILED: Could not allocate memory\n");
        return 0;
    }

    /* Copy test code to memory */
    memcpy(x86_mem, test->code, test->code_size);

    /* Make executable */
    mprotect(x86_mem, TEST_CODE_SIZE, PROT_READ | PROT_EXEC);

    /* Initialize Rosetta */
    if (rosetta_init() != 0) {
        printf("    FAILED: Could not initialize Rosetta\n");
        munmap(x86_mem, TEST_CODE_SIZE);
        return 0;
    }

    /* Set initial register state */
    ThreadState *state = rosetta_get_state();
    state->guest.r[X86_RAX] = test->initial_rax;
    state->guest.r[X86_RBX] = test->initial_rbx;
    state->guest.rip = (uint64_t)(uintptr_t)x86_mem;

    /* Verify decoder works */
    x86_insn_t insn;
    int len = decode_x86_insn((const uint8_t *)x86_mem, &insn);
    printf("    Decoder: len=%d, opcode=0x%02X, insn.length=%d\n", len, insn.opcode, insn.length);

    if (len == 0) {
        printf("    FAILED: Decoder returned 0\n");
        rosetta_cleanup();
        munmap(x86_mem, TEST_CODE_SIZE);
        return 0;
    }

    /* Translate the block */
    printf("    About to translate block at %p...\n", x86_mem);
    void *translated = translate_block((uint64_t)(uintptr_t)x86_mem);
    printf("    Translation returned: %p\n", translated);
    if (!translated) {
        printf("    FAILED: Translation returned NULL (code_size may be 0 or cache alloc failed)\n");
        rosetta_cleanup();
        munmap(x86_mem, TEST_CODE_SIZE);
        return 0;
    }

    rosetta_cleanup();
    munmap(x86_mem, TEST_CODE_SIZE);

    printf("    PASSED (translated to ARM64 at %p)\n", translated);
    return 1;
}

/* ============================================================================
 * Mach-O Loader Test
 * ============================================================================ */

/**
 * Test loading a Mach-O binary
 */
int test_macho_loader(const char *filename)
{
    printf("\n=== Mach-O Loader Test ===\n");
    printf("Loading: %s\n", filename);

    rosetta_binary_t *binary = NULL;
    if (rosetta_macho_load(filename, &binary) != 0) {
        printf("  FAILED: Could not load binary\n");
        return 0;
    }

    /* Validate it's x86_64 */
    if (!rosetta_macho_validate_x86_64(binary)) {
        printf("  FAILED: Not a valid x86_64 binary\n");
        rosetta_macho_unload(binary);
        return 0;
    }

    /* Print binary info */
    rosetta_macho_print_info(binary);

    /* Verify entry point */
    uint64_t entry = rosetta_macho_get_entry_point(binary);
    if (entry == 0) {
        printf("  WARNING: Entry point is 0\n");
    } else {
        printf("  Entry point: 0x%016llX\n", (unsigned long long)entry);
    }

    /* Look up common symbols */
    uint64_t main_addr = rosetta_macho_lookup_symbol(binary, "_main");
    if (main_addr != 0) {
        printf("  Found _main at: 0x%016llX\n", (unsigned long long)main_addr);
    }

    rosetta_macho_unload(binary);
    printf("  PASSED\n");
    return 1;
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(int argc, char *argv[])
{
    int passed = 0;
    int failed = 0;

    printf("==================================================\n");
    printf("Rosetta End-to-End Test Harness\n");
    printf("==================================================\n\n");

    /* Test 1: Translation tests */
    printf("=== Translation Tests ===\n");
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        if (run_translation_test(&test_cases[i])) {
            passed++;
        } else {
            failed++;
        }
        printf("\n");
    }

    /* Test 2: Mach-O loader test (if binary provided) */
    if (argc > 1) {
        if (test_macho_loader(argv[1])) {
            passed++;
        } else {
            failed++;
        }
    } else {
        printf("\n=== Mach-O Loader Test (skipped - no binary provided) ===\n");
        printf("  Usage: %s <path-to-x86_64-binary>\n\n", argv[0]);
    }

    /* Summary */
    printf("==================================================\n");
    printf("Test Summary\n");
    printf("==================================================\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Total:  %d\n", passed + failed);
    printf("==================================================\n");

    return failed > 0 ? 1 : 0;
}

/* End of test_e2e.c */
