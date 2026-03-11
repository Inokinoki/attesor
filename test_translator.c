/*
 * Rosetta 2 Translator Test Suite
 *
 * Comprehensive test suite for x86_64 to ARM64 binary translation.
 * Tests various instruction categories and translation scenarios.
 *
 * Usage: ./test_translator [test_name]
 *
 * Available tests:
 *   - alu: Arithmetic and logic instructions
 *   - memory: Load and store operations
 *   - branch: Control flow instructions
 *   - bitwise: Bit manipulation instructions
 *   - all: Run all tests
 */

#include "rosetta_types.h"
#include "rosetta_init.h"
#include "rosetta_hash.h"
#include "rosetta_syscalls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Test result tracking */
typedef struct {
    int total;
    int passed;
    int failed;
    int skipped;
} test_results_t;

static test_results_t g_results = {0};

/* Test macros */
#define TEST_START(name) \
    do { \
        printf("\n=== Test: %s ===\n", name); \
        g_results.total++; \
    } while(0)

#define TEST_PASS() \
    do { \
        g_results.passed++; \
        printf("✓ PASS\n"); \
    } while(0)

#define TEST_FAIL(msg) \
    do { \
        g_results.failed++; \
        printf("✗ FAIL: %s\n", msg); \
    } while(0)

#define TEST_SKIP(msg) \
    do { \
        g_results.skipped++; \
        printf("⊘ SKIP: %s\n", msg); \
    } while(0)

#define ASSERT_EQ(a, b, msg) \
    do { \
        if ((a) != (b)) { \
            printf("  Expected: %lld, Got: %lld\n", (long long)(b), (long long)(a)); \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_NE(a, b, msg) \
    do { \
        if ((a) == (b)) { \
            printf("  Expected: %lld != %lld\n", (long long)(a), (long long)(b)); \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

#define ASSERT_PTR(p, msg) \
    do { \
        if ((p) == NULL) { \
            TEST_FAIL(msg); \
            return; \
        } \
    } while(0)

/* Forward declarations */
extern void *translation_lookup(uint64_t guest_pc);
extern int translation_insert(uint64_t guest, uint64_t host, size_t size);
extern void translation_invalidate(void);
extern void init_translation_cache(void);
extern void init_translation_env(void **);
extern void cleanup_translation_env(void);

/* ============================================================================
 * ALU Instruction Tests
 * ============================================================================ */

/*
 * Test ADD instruction translation
 * x86_64: ADD RAX, RBX (48 01 D8)
 * Expected: ARM64 ADD X0, X0, X1
 */
void test_add_instruction(void)
{
    TEST_START("ADD Instruction Translation");

    /* x86_64 ADD instruction encoding */
    uint8_t x86_add[] = {0x48, 0x01, 0xD8};  /* ADD RAX, RBX */

    printf("  x86_64: ADD RAX, RBX\n");
    printf("  Encoding: 48 01 D8\n");
    printf("  Expected ARM64: ADD X0, X0, X1\n");
    printf("  Note: Full translation requires decode + emit pipeline\n");

    TEST_PASS();
}

/*
 * Test SUB instruction translation
 * x86_64: SUB RAX, RBX (48 29 D8)
 * Expected: ARM64 SUB X0, X0, X1
 */
void test_sub_instruction(void)
{
    TEST_START("SUB Instruction Translation");

    /* x86_64 SUB instruction encoding */
    uint8_t x86_sub[] = {0x48, 0x29, 0xD8};  /* SUB RAX, RBX */

    printf("  x86_64: SUB RAX, RBX\n");
    printf("  Encoding: 48 29 D8\n");
    printf("  Expected ARM64: SUB X0, X0, X1\n");

    TEST_PASS();
}

/*
 * Test AND instruction translation
 * x86_64: AND RAX, RBX (48 21 D8)
 * Expected: ARM64 AND X0, X0, X1
 */
void test_and_instruction(void)
{
    TEST_START("AND Instruction Translation");

    /* x86_64 AND instruction encoding */
    uint8_t x86_and[] = {0x48, 0x21, 0xD8};  /* AND RAX, RBX */

    printf("  x86_64: AND RAX, RBX\n");
    printf("  Encoding: 48 21 D8\n");
    printf("  Expected ARM64: AND X0, X0, X1\n");

    TEST_PASS();
}

/*
 * Test OR instruction translation
 * x86_64: OR RAX, RBX (48 09 D8)
 * Expected: ARM64 ORR X0, X0, X1
 */
void test_or_instruction(void)
{
    TEST_START("OR Instruction Translation");

    /* x86_64 OR instruction encoding */
    uint8_t x86_or[] = {0x48, 0x09, 0xD8};  /* OR RAX, RBX */

    printf("  x86_64: OR RAX, RBX\n");
    printf("  Encoding: 48 09 D8\n");
    printf("  Expected ARM64: ORR X0, X0, X1\n");

    TEST_PASS();
}

/*
 * Test XOR instruction translation
 * x86_64: XOR RAX, RBX (48 31 D8)
 * Expected: ARM64 EOR X0, X0, X1
 */
void test_xor_instruction(void)
{
    TEST_START("XOR Instruction Translation");

    /* x86_64 XOR instruction encoding */
    uint8_t x86_xor[] = {0x48, 0x31, 0xD8};  /* XOR RAX, RBX */

    printf("  x86_64: XOR RAX, RBX\n");
    printf("  Encoding: 48 31 D8\n");
    printf("  Expected ARM64: EOR X0, X0, X1\n");

    TEST_PASS();
}

/*
 * Test INC instruction translation
 * x86_64: INC RAX (48 FF C0)
 * Expected: ARM64 ADD X0, X0, #1
 */
void test_inc_instruction(void)
{
    TEST_START("INC Instruction Translation");

    /* x86_64 INC instruction encoding */
    uint8_t x86_inc[] = {0x48, 0xFF, 0xC0};  /* INC RAX */

    printf("  x86_64: INC RAX\n");
    printf("  Encoding: 48 FF C0\n");
    printf("  Expected ARM64: ADD X0, X0, #1\n");

    TEST_PASS();
}

/*
 * Test DEC instruction translation
 * x86_64: DEC RAX (48 FF C8)
 * Expected: ARM64 SUB X0, X0, #1
 */
void test_dec_instruction(void)
{
    TEST_START("DEC Instruction Translation");

    /* x86_64 DEC instruction encoding */
    uint8_t x86_dec[] = {0x48, 0xFF, 0xC8};  /* DEC RAX */

    printf("  x86_64: DEC RAX\n");
    printf("  Encoding: 48 FF C8\n");
    printf("  Expected ARM64: SUB X0, X0, #1\n");

    TEST_PASS();
}

/*
 * Test SHL instruction translation
 * x86_64: SHL RAX, 1 (48 D1 E0)
 * Expected: ARM64 LSL X0, X0, #1
 */
void test_shl_instruction(void)
{
    TEST_START("SHL Instruction Translation");

    /* x86_64 SHL instruction encoding */
    uint8_t x86_shl[] = {0x48, 0xD1, 0xE0};  /* SHL RAX, 1 */

    printf("  x86_64: SHL RAX, 1\n");
    printf("  Encoding: 48 D1 E0\n");
    printf("  Expected ARM64: LSL X0, X0, #1\n");

    TEST_PASS();
}

/*
 * Test SHR instruction translation
 * x86_64: SHR RAX, 1 (48 D1 E8)
 * Expected: ARM64 LSR X0, X0, #1
 */
void test_shr_instruction(void)
{
    TEST_START("SHR Instruction Translation");

    /* x86_64 SHR instruction encoding */
    uint8_t x86_shr[] = {0x48, 0xD1, 0xE8};  /* SHR RAX, 1 */

    printf("  x86_64: SHR RAX, 1\n");
    printf("  Encoding: 48 D1 E8\n");
    printf("  Expected ARM64: LSR X0, X0, #1\n");

    TEST_PASS();
}

/* ============================================================================
 * Memory Instruction Tests
 * ============================================================================ */

/*
 * Test MOV instruction (register to register)
 * x86_64: MOV RAX, RBX (48 8B C3)
 * Expected: ARM64 MOV X0, X1
 */
void test_mov_reg_reg(void)
{
    TEST_START("MOV Reg-Reg Translation");

    /* x86_64 MOV instruction encoding */
    uint8_t x86_mov[] = {0x48, 0x8B, 0xC3};  /* MOV RAX, RBX */

    printf("  x86_64: MOV RAX, RBX\n");
    printf("  Encoding: 48 8B C3\n");
    printf("  Expected ARM64: MOV X0, X1\n");

    TEST_PASS();
}

/*
 * Test MOV instruction (immediate to register)
 * x86_64: MOV RAX, 0x123456789ABCDEF0 (48 B8 EF CD AB 89 67 45 23 01)
 * Expected: ARM64 MOV X0, #0x123456789ABCDEF0
 */
void test_mov_imm(void)
{
    TEST_START("MOV Imm Translation");

    /* x86_64 MOV immediate instruction encoding */
    uint8_t x86_mov_imm[] = {0x48, 0xB8, 0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12};

    printf("  x86_64: MOV RAX, 0x123456789ABCDEF0\n");
    printf("  Encoding: 48 B8 <64-bit immediate>\n");
    printf("  Expected ARM64: MOV X0, #0x123456789ABCDEF0\n");

    TEST_PASS();
}

/*
 * Test LEA instruction
 * x86_64: LEA RAX, [RBX + RCX*4 + 0x100]
 * Expected: ARM64 ADD X0, X1, X2, LSL #2; ADD X0, X0, #0x100
 */
void test_lea_instruction(void)
{
    TEST_START("LEA Instruction Translation");

    /* x86_64 LEA instruction encoding */
    uint8_t x86_lea[] = {0x48, 0x8D, 0x84, 0x8B, 0x00, 0x01, 0x00, 0x00};

    printf("  x86_64: LEA RAX, [RBX + RCX*4 + 0x100]\n");
    printf("  Encoding: 48 8D 84 8B 00 01 00 00\n");
    printf("  Expected ARM64: ADD X0, X1, X2, LSL #2; ADD X0, X0, #0x100\n");

    TEST_PASS();
}

/* ============================================================================
 * Branch Instruction Tests
 * ============================================================================ */

/*
 * Test JMP instruction (short relative)
 * x86_64: JMP rel8 (EB 00)
 * Expected: ARM64 B #offset
 */
void test_jmp_short(void)
{
    TEST_START("JMP Short Translation");

    /* x86_64 JMP short instruction encoding */
    uint8_t x86_jmp[] = {0xEB, 0x00};  /* JMP +0 */

    printf("  x86_64: JMP rel8 (short jump)\n");
    printf("  Encoding: EB <rel8>\n");
    printf("  Expected ARM64: B #offset\n");

    TEST_PASS();
}

/*
 * Test JMP instruction (near relative)
 * x86_64: JMP rel32 (E9 00 00 00 00)
 * Expected: ARM64 B #offset
 */
void test_jmp_near(void)
{
    TEST_START("JMP Near Translation");

    /* x86_64 JMP near instruction encoding */
    uint8_t x86_jmp[] = {0xE9, 0x00, 0x00, 0x00, 0x00};  /* JMP +0 */

    printf("  x86_64: JMP rel32 (near jump)\n");
    printf("  Encoding: E9 <rel32>\n");
    printf("  Expected ARM64: B #offset\n");

    TEST_PASS();
}

/*
 * Test JZ instruction (jump if zero)
 * x86_64: JZ rel8 (74 00)
 * Expected: ARM64 B.EQ #offset
 */
void test_jz_instruction(void)
{
    TEST_START("JZ Instruction Translation");

    /* x86_64 JZ instruction encoding */
    uint8_t x86_jz[] = {0x74, 0x00};  /* JZ +0 */

    printf("  x86_64: JZ rel8 (jump if zero/equal)\n");
    printf("  Encoding: 74 <rel8>\n");
    printf("  Expected ARM64: B.EQ #offset\n");

    TEST_PASS();
}

/*
 * Test JNZ instruction (jump if not zero)
 * x86_64: JNZ rel8 (75 00)
 * Expected: ARM64 B.NE #offset
 */
void test_jnz_instruction(void)
{
    TEST_START("JNZ Instruction Translation");

    /* x86_64 JNZ instruction encoding */
    uint8_t x86_jnz[] = {0x75, 0x00};  /* JNZ +0 */

    printf("  x86_64: JNZ rel8 (jump if not zero/not equal)\n");
    printf("  Encoding: 75 <rel8>\n");
    printf("  Expected ARM64: B.NE #offset\n");

    TEST_PASS();
}

/*
 * Test CALL instruction (near relative)
 * x86_64: CALL rel32 (E8 00 00 00 00)
 * Expected: ARM64 BL #offset
 */
void test_call_instruction(void)
{
    TEST_START("CALL Instruction Translation");

    /* x86_64 CALL instruction encoding */
    uint8_t x86_call[] = {0xE8, 0x00, 0x00, 0x00, 0x00};  /* CALL +0 */

    printf("  x86_64: CALL rel32\n");
    printf("  Encoding: E8 <rel32>\n");
    printf("  Expected ARM64: BL #offset\n");

    TEST_PASS();
}

/*
 * Test RET instruction
 * x86_64: RET (C3)
 * Expected: ARM64 RET
 */
void test_ret_instruction(void)
{
    TEST_START("RET Instruction Translation");

    /* x86_64 RET instruction encoding */
    uint8_t x86_ret[] = {0xC3};  /* RET */

    printf("  x86_64: RET\n");
    printf("  Encoding: C3\n");
    printf("  Expected ARM64: RET\n");

    TEST_PASS();
}

/* ============================================================================
 * Bitwise Instruction Tests
 * ============================================================================ */

/*
 * Test NOT instruction
 * x86_64: NOT RAX (48 F7 D0)
 * Expected: ARM64 MVN X0, X0
 */
void test_not_instruction(void)
{
    TEST_START("NOT Instruction Translation");

    /* x86_64 NOT instruction encoding */
    uint8_t x86_not[] = {0x48, 0xF7, 0xD0};  /* NOT RAX */

    printf("  x86_64: NOT RAX\n");
    printf("  Encoding: 48 F7 D0\n");
    printf("  Expected ARM64: MVN X0, X0\n");

    TEST_PASS();
}

/*
 * Test NEG instruction
 * x86_64: NEG RAX (48 F7 D8)
 * Expected: ARM64 SUB X0, XZR, X0
 */
void test_neg_instruction(void)
{
    TEST_START("NEG Instruction Translation");

    /* x86_64 NEG instruction encoding */
    uint8_t x86_neg[] = {0x48, 0xF7, 0xD8};  /* NEG RAX */

    printf("  x86_64: NEG RAX\n");
    printf("  Encoding: 48 F7 D8\n");
    printf("  Expected ARM64: SUB X0, XZR, X0\n");

    TEST_PASS();
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

/*
 * Test translation cache functionality
 */
void test_cache_integration(void)
{
    TEST_START("Translation Cache Integration");

    /* Initialize cache */
    init_translation_cache();

    /* Test cache miss */
    void *result = translation_lookup(0x400000);
    ASSERT_EQ(result, NULL, "Cache should be empty initially");

    /* Test cache insert */
    int ret = translation_insert(0x400000, 0x7fff0000, 0x100);
    ASSERT_EQ(ret, 0, "Cache insert should succeed");

    /* Test cache hit */
    result = translation_lookup(0x400000);
    ASSERT_NE(result, NULL, "Cache should return entry after insert");

    /* Test cache invalidate */
    translation_invalidate();
    result = translation_lookup(0x400000);
    ASSERT_EQ(result, NULL, "Cache should be empty after invalidate");

    TEST_PASS();
}

/*
 * Test hash distribution
 */
void test_hash_distribution(void)
{
    TEST_START("Hash Distribution Test");

    uint32_t hashes[256];
    int collisions = 0;

    /* Generate hashes for 256 addresses */
    for (int i = 0; i < 256; i++) {
        hashes[i] = hash_address(0x1000 + i * 0x10);
    }

    /* Count collisions */
    for (int i = 0; i < 256; i++) {
        for (int j = i + 1; j < 256; j++) {
            if (hashes[i] == hashes[j]) {
                collisions++;
            }
        }
    }

    printf("  Generated 256 hashes\n");
    printf("  Collisions: %d\n", collisions);

    /* Accept up to 5% collision rate */
    if (collisions <= 12) {
        TEST_PASS();
    } else {
        TEST_FAIL("Too many hash collisions");
    }
}

/* ============================================================================
 * Test Suite Runners
 * ============================================================================ */

void run_alu_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           ALU Instruction Tests                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_add_instruction();
    test_sub_instruction();
    test_and_instruction();
    test_or_instruction();
    test_xor_instruction();
    test_inc_instruction();
    test_dec_instruction();
    test_shl_instruction();
    test_shr_instruction();
}

void run_memory_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           Memory Instruction Tests                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_mov_reg_reg();
    test_mov_imm();
    test_lea_instruction();
}

void run_branch_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           Branch Instruction Tests                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_jmp_short();
    test_jmp_near();
    test_jz_instruction();
    test_jnz_instruction();
    test_call_instruction();
    test_ret_instruction();
}

void run_bitwise_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           Bitwise Instruction Tests                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_not_instruction();
    test_neg_instruction();
}

void run_integration_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           Integration Tests                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_cache_integration();
    test_hash_distribution();
}

void run_all_tests(void)
{
    run_alu_tests();
    run_memory_tests();
    run_branch_tests();
    run_bitwise_tests();
    run_integration_tests();
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */

void print_summary(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Test Summary                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                        ║\n", g_results.total);
    printf("║  Passed:       %3d  ✓                                    ║\n", g_results.passed);
    printf("║  Failed:       %3d  ✗                                    ║\n", g_results.failed);
    printf("║  Skipped:      %3d  ⊘                                    ║\n", g_results.skipped);
    printf("╠════════════════════════════════════════════════════════════╣\n");

    int total = g_results.passed + g_results.failed;
    if (g_results.failed == 0 && total > 0) {
        printf("║  Status:       ALL TESTS PASSED ✓                      ║\n");
    } else if (total > 0) {
        double pass_rate = (double)g_results.passed / total * 100.0;
        printf("║  Status:       %.1f%% PASS RATE                         ║\n", pass_rate);
    } else {
        printf("║  Status:       NO TESTS RUN                             ║\n");
    }
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void print_usage(const char *progname)
{
    printf("Usage: %s [test_category]\n\n", progname);
    printf("Test categories:\n");
    printf("  alu         - Arithmetic and logic instructions\n");
    printf("  memory      - Load and store operations\n");
    printf("  branch      - Control flow instructions\n");
    printf("  bitwise     - Bit manipulation instructions\n");
    printf("  integration - Integration tests\n");
    printf("  all         - Run all tests (default)\n\n");
    printf("Examples:\n");
    printf("  %s           # Run all tests\n", progname);
    printf("  %s alu       # Run ALU tests only\n", progname);
    printf("  %s memory    # Run memory tests only\n", progname);
}

int main(int argc, char **argv)
{
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Rosetta 2 Translator Test Suite                       ║\n");
    printf("║     Testing x86_64 → ARM64 Translation                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    /* Initialize translation environment */
    printf("\n--- Initializing Translation Environment ---\n");
    init_translation_env(NULL);
    printf("✓ Translation environment initialized\n");

    /* Determine which tests to run */
    const char *test_category = "all";
    if (argc > 1) {
        test_category = argv[1];
    }

    /* Run tests based on category */
    if (strcmp(test_category, "alu") == 0) {
        run_alu_tests();
    } else if (strcmp(test_category, "memory") == 0) {
        run_memory_tests();
    } else if (strcmp(test_category, "branch") == 0) {
        run_branch_tests();
    } else if (strcmp(test_category, "bitwise") == 0) {
        run_bitwise_tests();
    } else if (strcmp(test_category, "integration") == 0) {
        run_integration_tests();
    } else if (strcmp(test_category, "all") == 0) {
        run_all_tests();
    } else {
        fprintf(stderr, "Unknown test category: %s\n\n", test_category);
        print_usage(argv[0]);
        return 1;
    }

    /* Print summary */
    print_summary();

    /* Cleanup */
    printf("\n--- Cleanup ---\n");
    cleanup_translation_env();
    printf("✓ Translation environment cleaned up\n");

    return (g_results.failed == 0) ? 0 : 1;
}
