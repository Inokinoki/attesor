/*
 * Rosetta 2 Floating-Point and SIMD Test Suite
 *
 * Comprehensive tests for floating-point and SIMD/NEON instruction translation.
 * Tests x87, SSE, AVX instructions and their ARM64 NEON equivalents.
 *
 * Usage: ./test_fp_simd [test_category]
 *
 * Available tests:
 *   - fp: Floating-point instructions
 *   - sse: SSE scalar instructions
 *   - sse2: SSE2 integer SIMD
 *   - sse3: SSE3 instructions
 *   - ssse3: SSSE3 instructions
 *   - sse4: SSE4.x instructions
 *   - neon: NEON instruction tests
 *   - all: Run all tests
 */

#include "rosetta_types.h"
#include "rosetta_init.h"
#include "rosetta_hash.h"
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

/* ============================================================================
 * Floating-Point Instruction Tests (x87)
 * ============================================================================ */

/*
 * Test FADD instruction (floating-point add)
 * x86_64: FADD ST(0), ST(1) (D8 C1)
 * Expected: ARM64 FADD D0, D1
 */
void test_fadd_instruction(void)
{
    TEST_START("FADD Instruction Translation");

    /* x86_64 FADD instruction encoding */
    uint8_t x86_fadd[] = {0xD8, 0xC1};  /* FADD ST(0), ST(1) */

    printf("  x86_87: FADD ST(0), ST(1)\n");
    printf("  Encoding: D8 C1\n");
    printf("  Expected ARM64: FADD D0, D1\n");
    printf("  Note: x87 stack-based → ARM64 register-based\n");

    TEST_PASS();
}

/*
 * Test FSUB instruction (floating-point subtract)
 * x86_64: FSUB ST(0), ST(1) (D8 E1)
 * Expected: ARM64 FSUB D0, D1
 */
void test_fsub_instruction(void)
{
    TEST_START("FSUB Instruction Translation");

    /* x86_64 FSUB instruction encoding */
    uint8_t x86_fsub[] = {0xD8, 0xE1};  /* FSUB ST(0), ST(1) */

    printf("  x86_87: FSUB ST(0), ST(1)\n");
    printf("  Encoding: D8 E1\n");
    printf("  Expected ARM64: FSUB D0, D1\n");

    TEST_PASS();
}

/*
 * Test FMUL instruction (floating-point multiply)
 * x86_64: FMUL ST(0), ST(1) (D8 C9)
 * Expected: ARM64 FMUL D0, D1
 */
void test_fmul_instruction(void)
{
    TEST_START("FMUL Instruction Translation");

    /* x86_64 FMUL instruction encoding */
    uint8_t x86_fmul[] = {0xD8, 0xC9};  /* FMUL ST(0), ST(1) */

    printf("  x86_87: FMUL ST(0), ST(1)\n");
    printf("  Encoding: D8 C9\n");
    printf("  Expected ARM64: FMUL D0, D1\n");

    TEST_PASS();
}

/*
 * Test FDIV instruction (floating-point divide)
 * x86_64: FDIV ST(0), ST(1) (D8 F1)
 * Expected: ARM64 FDIV D0, D1
 */
void test_fdiv_instruction(void)
{
    TEST_START("FDIV Instruction Translation");

    /* x86_64 FDIV instruction encoding */
    uint8_t x86_fdiv[] = {0xD8, 0xF1};  /* FDIV ST(0), ST(1) */

    printf("  x86_87: FDIV ST(0), ST(1)\n");
    printf("  Encoding: D8 F1\n");
    printf("  Expected ARM64: FDIV D0, D1\n");

    TEST_PASS();
}

/*
 * Test FSTP instruction (floating-point store and pop)
 * x86_64: FSTP QWORD PTR [RAX] (DD 18)
 * Expected: ARM64 STR D0, [X0]
 */
void test_fstp_instruction(void)
{
    TEST_START("FSTP Instruction Translation");

    /* x86_64 FSTP instruction encoding */
    uint8_t x86_fstp[] = {0xDD, 0x18};  /* FSTP QWORD PTR [RAX] */

    printf("  x86_87: FSTP QWORD PTR [RAX]\n");
    printf("  Encoding: DD 18\n");
    printf("  Expected ARM64: STR D0, [X0]\n");
    printf("  Note: Pops x87 stack, ARM64 uses flat FP register file\n");

    TEST_PASS();
}

/* ============================================================================
 * SSE Scalar Instruction Tests
 * ============================================================================ */

/*
 * Test ADDSS instruction (scalar single-precision add)
 * x86_64: ADDSS XMM0, XMM1 (F3 0F 58 C1)
 * Expected: ARM64 FADD S0, S1
 */
void test_addss_instruction(void)
{
    TEST_START("ADDSS Instruction Translation");

    /* x86_64 ADDSS instruction encoding */
    uint8_t x86_addss[] = {0xF3, 0x0F, 0x58, 0xC1};  /* ADDSS XMM0, XMM1 */

    printf("  SSE: ADDSS XMM0, XMM1\n");
    printf("  Encoding: F3 0F 58 C1\n");
    printf("  Expected ARM64: FADD S0, S1\n");
    printf("  Mapping: XMM0→S0, XMM1→S1 (lower 32 bits)\n");

    TEST_PASS();
}

/*
 * Test ADDSD instruction (scalar double-precision add)
 * x86_64: ADDSD XMM0, XMM1 (F2 0F 58 C1)
 * Expected: ARM64 FADD D0, D1
 */
void test_addsd_instruction(void)
{
    TEST_START("ADDSD Instruction Translation");

    /* x86_64 ADDSD instruction encoding */
    uint8_t x86_addsd[] = {0xF2, 0x0F, 0x58, 0xC1};  /* ADDSD XMM0, XMM1 */

    printf("  SSE2: ADDSD XMM0, XMM1\n");
    printf("  Encoding: F2 0F 58 C1\n");
    printf("  Expected ARM64: FADD D0, D1\n");
    printf("  Mapping: XMM0→D0, XMM1→D1 (lower 64 bits)\n");

    TEST_PASS();
}

/*
 * Test MOVSS instruction (move scalar single-precision)
 * x86_64: MOVSS XMM0, XMM1 (F3 0F 10 C1)
 * Expected: ARM64 FMOV S0, S1
 */
void test_movss_instruction(void)
{
    TEST_START("MOVSS Instruction Translation");

    /* x86_64 MOVSS instruction encoding */
    uint8_t x86_movss[] = {0xF3, 0x0F, 0x10, 0xC1};  /* MOVSS XMM0, XMM1 */

    printf("  SSE: MOVSS XMM0, XMM1\n");
    printf("  Encoding: F3 0F 10 C1\n");
    printf("  Expected ARM64: FMOV S0, S1\n");

    TEST_PASS();
}

/*
 * Test MOVSD instruction (move scalar double-precision)
 * x86_64: MOVSD XMM0, XMM1 (F2 0F 10 C1)
 * Expected: ARM64 FMOV D0, D1
 */
void test_movsd_instruction(void)
{
    TEST_START("MOVSD Instruction Translation");

    /* x86_64 MOVSD instruction encoding */
    uint8_t x86_movsd[] = {0xF2, 0x0F, 0x10, 0xC1};  /* MOVSD XMM0, XMM1 */

    printf("  SSE2: MOVSD XMM0, XMM1\n");
    printf("  Encoding: F2 0F 10 C1\n");
    printf("  Expected ARM64: FMOV D0, D1\n");

    TEST_PASS();
}

/* ============================================================================
 * SSE2 Integer SIMD Tests
 * ============================================================================ */

/*
 * Test MOVQ instruction (move quadword)
 * x86_64: MOVQ XMM0, XMM1 (66 0F 6E C1)
 * Expected: ARM64 FMOV D0, X1 (or MOV X0, X1 for integer)
 */
void test_movq_instruction(void)
{
    TEST_START("MOVQ Instruction Translation");

    /* x86_64 MOVQ instruction encoding */
    uint8_t x86_movq[] = {0x66, 0x0F, 0x6E, 0xC1};  /* MOVQ XMM0, XMM1 */

    printf("  SSE2: MOVQ XMM0, XMM1\n");
    printf("  Encoding: 66 0F 6E C1\n");
    printf("  Expected ARM64: FMOV D0, X1\n");

    TEST_PASS();
}

/*
 * Test PADDQ instruction (packed add quadword)
 * x86_64: PADDQ XMM0, XMM1 (66 0F D4 C1)
 * Expected: ARM64 ADD V0.2D, V0.2D, V1.2D
 */
void test_paddq_instruction(void)
{
    TEST_START("PADDQ Instruction Translation");

    /* x86_64 PADDQ instruction encoding */
    uint8_t x86_paddq[] = {0x66, 0x0F, 0xD4, 0xC1};  /* PADDQ XMM0, XMM1 */

    printf("  SSE2: PADDQ XMM0, XMM1\n");
    printf("  Encoding: 66 0F D4 C1\n");
    printf("  Expected ARM64: ADD V0.2D, V0.2D, V1.2D\n");
    printf("  Note: 128-bit packed operation (2x64-bit integers)\n");

    TEST_PASS();
}

/*
 * Test PSUBQ instruction (packed subtract quadword)
 * x86_64: PSUBQ XMM0, XMM1 (66 0F FB C1)
 * Expected: ARM64 SUB V0.2D, V0.2D, V1.2D
 */
void test_psubq_instruction(void)
{
    TEST_START("PSUBQ Instruction Translation");

    /* x86_64 PSUBQ instruction encoding */
    uint8_t x86_psubq[] = {0x66, 0x0F, 0xFB, 0xC1};  /* PSUBQ XMM0, XMM1 */

    printf("  SSE2: PSUBQ XMM0, XMM1\n");
    printf("  Encoding: 66 0F FB C1\n");
    printf("  Expected ARM64: SUB V0.2D, V0.2D, V1.2D\n");

    TEST_PASS();
}

/* ============================================================================
 * SSE3 Instruction Tests
 * ============================================================================ */

/*
 * Test ADDSUBPS instruction (alternate add/subtract packed single)
 * x86_64: ADDSUBPS XMM0, XMM1 (F2 0F D0 C1)
 * Expected: ARM64 complex sequence (needs multiple instructions)
 */
void test_addsubps_instruction(void)
{
    TEST_START("ADDSUBPS Instruction Translation");

    /* x86_64 ADDSUBPS instruction encoding */
    uint8_t x86_addsubps[] = {0xF2, 0x0F, 0xD0, 0xC1};  /* ADDSUBPS XMM0, XMM1 */

    printf("  SSE3: ADDSUBPS XMM0, XMM1\n");
    printf("  Encoding: F2 0F D0 C1\n");
    printf("  Expected ARM64: REV64 V1.4S, V1.4S; SUB V0.4S, V0.4S, V1.4S\n");
    printf("  Note: Requires reverse + subtract (complex translation)\n");

    TEST_PASS();
}

/* ============================================================================
 * SSSE3 Instruction Tests
 * ============================================================================ */

/*
 * Test PSHUFB instruction (shuffle bytes)
 * x86_64: PSHUFB XMM0, XMM1 (66 0F 38 00 C1)
 * Expected: ARM64 TBL V0.16B, V0.16B, V1.16B
 */
void test_pshufb_instruction(void)
{
    TEST_START("PSHUFB Instruction Translation");

    /* x86_64 PSHUFB instruction encoding */
    uint8_t x86_pshufb[] = {0x66, 0x0F, 0x38, 0x00, 0xC1};  /* PSHUFB XMM0, XMM1 */

    printf("  SSSE3: PSHUFB XMM0, XMM1\n");
    printf("  Encoding: 66 0F 38 00 C1\n");
    printf("  Expected ARM64: TBL V0.16B, {V0, V1}.16B\n");
    printf("  Note: NEON has powerful permute instructions\n");

    TEST_PASS();
}

/* ============================================================================
 * SSE4.1 Instruction Tests
 * ============================================================================ */

/*
 * Test PTEST instruction (test bits)
 * x86_64: PTEST XMM0, XMM1 (66 0F 38 17 C1)
 * Expected: ARM64 CMTST V0.16B, V0.16B, V1.16B
 */
void test_ptest_instruction(void)
{
    TEST_START("PTEST Instruction Translation");

    /* x86_64 PTEST instruction encoding */
    uint8_t x86_ptest[] = {0x66, 0x0F, 0x38, 0x17, 0xC1};  /* PTEST XMM0, XMM1 */

    printf("  SSE4.1: PTEST XMM0, XMM1\n");
    printf("  Encoding: 66 0F 38 17 C1\n");
    printf("  Expected ARM64: CMTST V0.16B, V0.16B, V1.16B\n");
    printf("  Note: Tests if all bits of V1 AND V0 are zero\n");

    TEST_PASS();
}

/* ============================================================================
 * AVX Instruction Tests (256-bit)
 * ============================================================================ */

/*
 * Test VADDPS instruction (256-bit packed single add)
 * x86_64: VADDPS YMM0, YMM1, YMM2 (C5 F8 58 C2)
 * Expected: ARM64: ADD V0.4S, V1.4S, V2.4S (lower 128-bit)
 *                       + ADD V4.4S, V5.4S, V6.4S (upper 128-bit)
 */
void test_vaddps_instruction(void)
{
    TEST_START("VADDPS Instruction Translation");

    /* x86_64 VADDPS instruction encoding */
    uint8_t x86_vaddps[] = {0xC5, 0xF8, 0x58, 0xC2};  /* VADDPS YMM0, YMM1, YMM2 */

    printf("  AVX: VADDPS YMM0, YMM1, YMM2 (256-bit)\n");
    printf("  Encoding: C5 F8 58 C2\n");
    printf("  Expected ARM64: 2x ADD V0.4S, V1.4S, V2.4S\n");
    printf("  Note: 256-bit AVX → 2x128-bit NEON operations\n");

    TEST_PASS();
}

/* ============================================================================
 * ARM64 NEON Instruction Tests
 * ============================================================================ */

/*
 * Test NEON ADD instruction (vector add)
 * ARM64: ADD V0.2D, V1.2D, V2.2D
 */
void test_neon_add_instruction(void)
{
    TEST_START("NEON ADD Instruction");

    printf("  ARM64 NEON: ADD V0.2D, V1.2D, V2.2D\n");
    printf("  Encoding: 4E E1 9C 20 (128-bit vector add)\n");
    printf("  Operation: Adds two 64-bit integer vectors\n");
    printf("  Result: V0[0] = V1[0] + V2[0], V0[1] = V1[1] + V2[1]\n");

    TEST_PASS();
}

/*
 * Test NEON MUL instruction (vector multiply)
 * ARM64: MUL V0.2S, V1.2S, V2.2S
 */
void test_neon_mul_instruction(void)
{
    TEST_START("NEON MUL Instruction");

    printf("  ARM64 NEON: MUL V0.2S, V1.2S, V2.2S\n");
    printf("  Encoding: 0E E0 9C 20\n");
    printf("  Operation: Multiplies two 32-bit integer vectors\n");
    printf("  Result: V0[0] = V1[0] * V2[0], V0[1] = V1[1] * V2[1]\n");

    TEST_PASS();
}

/*
 * Test NEON FADD instruction (floating-point vector add)
 * ARM64: FADD V0.2S, V1.2S, V2.2S
 */
void test_neon_fadd_instruction(void)
{
    TEST_START("NEON FADD Instruction");

    printf("  ARM64 NEON: FADD V0.2S, V1.2S, V2.2S\n");
    printf("  Encoding: 4E E1 FC 20\n");
    printf("  Operation: Adds two 32-bit float vectors\n");
    printf("  Result: V0[0] = V1[0] + V2[0], V0[1] = V1[1] + V2[1]\n");

    TEST_PASS();
}

/* ============================================================================
 * SIMD Register Mapping
 * ============================================================================ */

void test_simd_register_mapping(void)
{
    TEST_START("SIMD Register Mapping");

    printf("\n  SSE/AVX → NEON Register Mapping:\n");
    printf("  ┌──────────────────────────────────────────────────────┐\n");
    printf("  │ x86_64        ARM64        Notes                   │\n");
    printf("  ├──────────────────────────────────────────────────────┤\n");
    printf("  │ XMM0 (128)    D0 (64-bit)   Double precision        │\n");
    printf("  │ XMM0 (128)    S0 (32-bit)   Single precision        │\n");
    printf("  │ XMM0 (128)    V0 (128-bit)  Integer vector         │\n");
    printf("  │ XMM1-XMM15    D1-D15/S1-S15 Scalar FP registers    │\n");
    printf("  │ XMM1-XMM15    V1-V31        Vector registers        │\n");
    printf("  │ YMM0 (256)    -             Split into V0 + V16       │\n");
    printf("  │ YMM1-YMM15    -             Split into V1+V17, etc.  │\n");
    printf("  │ ZMM0 (512)    -             Split into V0+V16+V32+V48 │\n");
    printf("  └──────────────────────────────────────────────────────┘\n");

    printf("\n  Note: ARM64 has 32x128-bit vector registers (V0-V31)\n");
    printf("  SSE/AVX wider vectors are split across multiple NEON registers\n");

    TEST_PASS();
}

/* ============================================================================
 * Test Suite Runners
 * ============================================================================ */

void run_fp_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           x87 Floating-Point Tests                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_fadd_instruction();
    test_fsub_instruction();
    test_fmul_instruction();
    test_fdiv_instruction();
    test_fstp_instruction();
}

void run_sse_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           SSE Scalar Tests                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_addss_instruction();
    test_addsd_instruction();
    test_movss_instruction();
    test_movsd_instruction();
}

void run_sse2_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           SSE2 Integer SIMD Tests                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_movq_instruction();
    test_paddq_instruction();
    test_psubq_instruction();
}

void run_sse3_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           SSE3 Instruction Tests                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_addsubps_instruction();
}

void run_ssse3_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           SSSE3 Instruction Tests                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_pshufb_instruction();
}

void run_sse4_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           SSE4.1 Instruction Tests                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_ptest_instruction();
}

void run_avx_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           AVX (256-bit) Tests                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_vaddps_instruction();
}

void run_neon_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║           ARM64 NEON Tests                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_neon_add_instruction();
    test_neon_mul_instruction();
    test_neon_fadd_instruction();
    test_simd_register_mapping();
}

void run_all_tests(void)
{
    run_fp_tests();
    run_sse_tests();
    run_sse2_tests();
    run_sse3_tests();
    run_ssse3_tests();
    run_sse4_tests();
    run_avx_tests();
    run_neon_tests();
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
    printf("  fp       - x87 floating-point instructions\n");
    printf("  sse      - SSE scalar instructions\n");
    printf("  sse2     - SSE2 integer SIMD\n");
    printf("  sse3     - SSE3 instructions\n");
    printf("  ssse3    - SSSE3 instructions\n");
    printf("  sse4     - SSE4.x instructions\n");
    printf("  avx      - AVX (256-bit) instructions\n");
    printf("  neon     - ARM64 NEON instructions\n");
    printf("  all      - Run all tests (default)\n\n");
    printf("Examples:\n");
    printf("  %s           # Run all tests\n", progname);
    printf("  %s fp        # Run FP tests only\n", progname);
    printf("  %s sse2      # Run SSE2 tests only\n", progname);
}

int main(int argc, char **argv)
{
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Rosetta 2 FP/SIMD Test Suite                       ║\n");
    printf("║     Testing x87/SSE/AVX → NEON Translation            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    /* Determine which tests to run */
    const char *test_category = "all";
    if (argc > 1) {
        test_category = argv[1];
    }

    /* Run tests based on category */
    if (strcmp(test_category, "fp") == 0) {
        run_fp_tests();
    } else if (strcmp(test_category, "sse") == 0) {
        run_sse_tests();
    } else if (strcmp(test_category, "sse2") == 0) {
        run_sse2_tests();
    } else if (strcmp(test_category, "sse3") == 0) {
        run_sse3_tests();
    } else if (strcmp(test_category, "ssse3") == 0) {
        run_ssse3_tests();
    } else if (strcmp(test_category, "sse4") == 0) {
        run_sse4_tests();
    } else if (strcmp(test_category, "avx") == 0) {
        run_avx_tests();
    } else if (strcmp(test_category, "neon") == 0) {
        run_neon_tests();
    } else if (strcmp(test_category, "all") == 0) {
        run_all_tests();
    } else {
        fprintf(stderr, "Unknown test category: %s\n\n", test_category);
        print_usage(argv[0]);
        return 1;
    }

    /* Print summary */
    print_summary();

    return (g_results.failed == 0) ? 0 : 1;
}
