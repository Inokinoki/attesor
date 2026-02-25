/* ============================================================================
 * Rosetta Translation Module - Unit Tests
 * ============================================================================
 *
 * Compile: gcc -o test_translate test_translate.c rosetta_translate.c rosetta_codegen.c -I. -O2
 * Run: ./test_translate
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "rosetta_types.h"
#include "rosetta_codegen.h"
#include "rosetta_translate.h"

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static int test_##name(void); \
    static void run_test_##name(void) { \
        tests_run++; \
        printf("Running %s... ", #name); \
        if (test_##name()) { \
            tests_passed++; \
            printf("PASSED\n"); \
        } else { \
            tests_failed++; \
            printf("FAILED\n"); \
        } \
    } \
    static int test_##name(void)

#define ASSERT(cond) do { if (!(cond)) return 0; } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) return 0; } while(0)
#define ASSERT_NEQ(a, b) do { if ((a) == (b)) return 0; } while(0)
#define RUN_TEST(name) run_test_##name()

/* Helper to create a test code buffer */
static u8 test_code_buf[4096];
static code_buffer_t test_buf;

static void setup_test_buf(void)
{
    code_buffer_init(&test_buf, test_code_buf, sizeof(test_code_buf));
}

static int compare_bytes(u8 *expected, u8 *actual, size_t len)
{
    return memcmp(expected, actual, len) == 0;
}

/* ============================================================================
 * Register Mapping Tests
 * ============================================================================ */

TEST(x86_map_gpr_basic)
{
    /* ARM64 X0-X7 should map to x86_64 RAX-RBX (0-7) */
    ASSERT_EQ(x86_map_gpr(0), RAX);
    ASSERT_EQ(x86_map_gpr(1), RCX);
    ASSERT_EQ(x86_map_gpr(2), RDX);
    ASSERT_EQ(x86_map_gpr(3), RBX);
    ASSERT_EQ(x86_map_gpr(4), RSI);
    ASSERT_EQ(x86_map_gpr(5), RDI);
    ASSERT_EQ(x86_map_gpr(6), RBP);
    ASSERT_EQ(x86_map_gpr(7), RSP);

    return 1;
}

TEST(x86_map_gpr_high)
{
    /* ARM64 X8-X15 should map to x86_64 R8-R15 (8-15) */
    ASSERT_EQ(x86_map_gpr(8), R8);
    ASSERT_EQ(x86_map_gpr(9), R9);
    ASSERT_EQ(x86_map_gpr(10), R10);
    ASSERT_EQ(x86_map_gpr(11), R11);
    ASSERT_EQ(x86_map_gpr(12), R12);
    ASSERT_EQ(x86_map_gpr(13), R13);
    ASSERT_EQ(x86_map_gpr(14), R14);
    ASSERT_EQ(x86_map_gpr(15), R15);

    return 1;
}

TEST(x86_map_gpr_wrap)
{
    /* ARM64 X16-X30 should wrap around */
    ASSERT_EQ(x86_map_gpr(16), RAX);
    ASSERT_EQ(x86_map_gpr(17), RCX);
    ASSERT_EQ(x86_map_gpr(20), 4);  /* R4 */
    ASSERT_EQ(x86_map_gpr(24), R8);
    ASSERT_EQ(x86_map_gpr(30), R14);
    ASSERT_EQ(x86_map_gpr(31), R15);  /* X31 (SP) */

    return 1;
}

TEST(x86_map_xmm_basic)
{
    /* ARM64 V0-V15 should map directly to XMM0-XMM15 */
    int i;
    for (i = 0; i < 16; i++) {
        ASSERT_EQ(x86_map_xmm(i), i);
    }

    return 1;
}

TEST(x86_map_xmm_wrap)
{
    /* ARM64 V16-V31 should wrap modulo 16 */
    ASSERT_EQ(x86_map_xmm(16), 0);
    ASSERT_EQ(x86_map_xmm(20), 4);
    ASSERT_EQ(x86_map_xmm(31), 15);

    return 1;
}

/* ============================================================================
 * Data Processing - Register Tests
 * ============================================================================ */

TEST(translate_add_reg)
{
    setup_test_buf();

    /* X0 = X1 + X2 => MOV RAX, RCX; ADD RAX, RDX */
    translate_add_reg(&test_buf, 0, 1, 2);

    /* Should have emitted at least some bytes */
    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_sub_reg)
{
    setup_test_buf();

    /* X0 = X1 - X2 */
    translate_sub_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_and_reg)
{
    setup_test_buf();

    /* X0 = X1 & X2 */
    translate_and_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_orr_reg)
{
    setup_test_buf();

    /* X0 = X1 | X2 */
    translate_orr_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_eor_reg)
{
    setup_test_buf();

    /* X0 = X1 ^ X2 (XOR) */
    translate_eor_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_mvn_reg)
{
    setup_test_buf();

    /* X0 = ~X1 (NOT) */
    translate_mvn_reg(&test_buf, 0, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_mul_reg)
{
    setup_test_buf();

    /* X0 = X1 * X2 */
    translate_mul_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_sdiv_reg)
{
    setup_test_buf();

    /* X0 = X1 / X2 */
    translate_sdiv_reg(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Data Processing - Immediate Tests
 * ============================================================================ */

TEST(translate_add_imm)
{
    setup_test_buf();

    /* X0 = X1 + 42 */
    translate_add_imm(&test_buf, 0, 1, 42);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_sub_imm)
{
    setup_test_buf();

    /* X0 = X1 - 100 */
    translate_sub_imm(&test_buf, 0, 1, 100);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_movz)
{
    setup_test_buf();

    /* X0 = 0x1234 << (0*16) = 0x1234 */
    translate_movz(&test_buf, 0, 0x1234, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_movk)
{
    setup_test_buf();

    /* X0 = X0 | (0x5678 << 16) */
    translate_movk(&test_buf, 0, 0x5678, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_movn)
{
    setup_test_buf();

    /* X0 = ~0xFFFF = 0xFFFF0000... */
    translate_movn(&test_buf, 0, 0xFFFF, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Compare Tests
 * ============================================================================ */

TEST(translate_cmp_reg)
{
    setup_test_buf();

    /* CMP X1, X2 */
    translate_cmp_reg(&test_buf, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_cmn_reg)
{
    setup_test_buf();

    /* CMN X1, X2 (X1 + X2, flags only) */
    translate_cmn_reg(&test_buf, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_tst_reg)
{
    setup_test_buf();

    /* TST X1, X2 (X1 & X2, flags only) */
    translate_tst_reg(&test_buf, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Branch Tests
 * ============================================================================ */

TEST(translate_b)
{
    setup_test_buf();

    /* B +1000 */
    u32 offset = translate_b(&test_buf, 1000);

    /* Should return an offset for patching */
    ASSERT_NEQ(offset, 0);

    return 1;
}

TEST(translate_bl)
{
    setup_test_buf();

    /* BL +1000, return address 0x8000 */
    u32 offset = translate_bl(&test_buf, 30, 0x8000, 1000);

    ASSERT_NEQ(offset, 0);

    return 1;
}

TEST(translate_br)
{
    setup_test_buf();

    /* BR X5 (branch to register) */
    translate_br(&test_buf, 5);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ret)
{
    setup_test_buf();

    /* RET */
    translate_ret(&test_buf);

    /* RET is 0xC3, should be 1 byte */
    ASSERT(code_buffer_get_size(&test_buf) >= 1);

    return 1;
}

TEST(translate_cond_branch)
{
    setup_test_buf();

    /* B.EQ (condition = 0) */
    u32 offset = translate_cond_branch(&test_buf, 0);

    ASSERT_NEQ(offset, 0);

    return 1;
}

/* ============================================================================
 * Load/Store Tests
 * ============================================================================ */

TEST(translate_ldr_imm)
{
    setup_test_buf();

    /* LDR X0, [X1, #256] */
    translate_ldr_imm(&test_buf, 0, 1, 256);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_str_imm)
{
    setup_test_buf();

    /* STR X0, [X1, #256] */
    translate_str_imm(&test_buf, 0, 1, 256);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ldrb_imm)
{
    setup_test_buf();

    /* LDRB X0, [X1, #10] */
    translate_ldrb_imm(&test_buf, 0, 1, 10);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_strb_imm)
{
    setup_test_buf();

    /* STRB X0, [X1, #10] */
    translate_strb_imm(&test_buf, 0, 1, 10);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ldrh_imm)
{
    setup_test_buf();

    /* LDRH X0, [X1, #20] */
    translate_ldrh_imm(&test_buf, 0, 1, 20);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_strh_imm)
{
    setup_test_buf();

    /* STRH X0, [X1, #20] */
    translate_strh_imm(&test_buf, 0, 1, 20);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ldrsw_imm)
{
    setup_test_buf();

    /* LDRSW X0, [X1, #32] (sign-extend word) */
    translate_ldrsw_imm(&test_buf, 0, 1, 32);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ldur)
{
    setup_test_buf();

    /* LDUR X0, [X1, #-8] (unscaled offset) */
    translate_ldur(&test_buf, 0, 1, -8);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_stur)
{
    setup_test_buf();

    /* STUR X0, [X1, #-8] */
    translate_stur(&test_buf, 0, 1, -8);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Load/Store Pair Tests
 * ============================================================================ */

TEST(translate_ldp)
{
    setup_test_buf();

    /* LDP X0, X1, [X2, #0] */
    translate_ldp(&test_buf, 0, 1, 2, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_stp)
{
    setup_test_buf();

    /* STP X0, X1, [X2, #16] */
    translate_stp(&test_buf, 0, 1, 2, 16);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * System Tests
 * ============================================================================ */

TEST(translate_svc)
{
    setup_test_buf();

    /* SVC #0 */
    translate_svc(&test_buf, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_brk)
{
    setup_test_buf();

    /* BRK #0 (breakpoint) */
    translate_brk(&test_buf, 0);

    /* BRK should emit UD2 (0x0F 0x0B) */
    ASSERT(code_buffer_get_size(&test_buf) >= 2);

    return 1;
}

TEST(translate_hlt)
{
    setup_test_buf();

    /* HLT #0 */
    translate_hlt(&test_buf, 0);

    ASSERT(code_buffer_get_size(&test_buf) >= 2);

    return 1;
}

/* ============================================================================
 * NEON Load/Store Tests
 * ============================================================================ */

TEST(translate_ld1)
{
    setup_test_buf();

    /* LD1 {V0.16B}, [X1] */
    translate_ld1(&test_buf, 0, 1, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_st1)
{
    setup_test_buf();

    /* ST1 {V0.16B}, [X1] */
    translate_st1(&test_buf, 0, 1, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ld2)
{
    setup_test_buf();

    /* LD2 {V0.16B, V1.16B}, [X2] */
    translate_ld2(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_st2)
{
    setup_test_buf();

    /* ST2 {V0.16B, V1.16B}, [X2] */
    translate_st2(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ld3)
{
    setup_test_buf();

    /* LD3 {V0.16B, V1.16B, V2.16B}, [X3] (RGB de-interleave) */
    translate_ld3(&test_buf, 0, 1, 2, 3);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_st3)
{
    setup_test_buf();

    /* ST3 {V0.16B, V1.16B, V2.16B}, [X3] (RGB interleave) */
    translate_st3(&test_buf, 0, 1, 2, 3);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_ld4)
{
    setup_test_buf();

    /* LD4 {V0.16B, V1.16B, V2.16B, V3.16B}, [X4] (RGBA de-interleave) */
    translate_ld4(&test_buf, 0, 1, 2, 3, 4);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_st4)
{
    setup_test_buf();

    /* ST4 {V0.16B, V1.16B, V2.16B, V3.16B}, [X4] */
    translate_st4(&test_buf, 0, 1, 2, 3, 4);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * NEON Vector Arithmetic Tests
 * ============================================================================ */

TEST(translate_add_vec)
{
    setup_test_buf();

    /* ADD V0.4S, V1.4S, V2.4S (size=2 for 32-bit) */
    translate_add_vec(&test_buf, 0, 1, 2, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_sub_vec)
{
    setup_test_buf();

    /* SUB V0.4S, V1.4S, V2.4S */
    translate_sub_vec(&test_buf, 0, 1, 2, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_and_vec)
{
    setup_test_buf();

    /* AND V0.16B, V1.16B, V2.16B */
    translate_and_vec(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_orr_vec)
{
    setup_test_buf();

    /* ORR V0.16B, V1.16B, V2.16B */
    translate_orr_vec(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_eor_vec)
{
    setup_test_buf();

    /* EOR V0.16B, V1.16B, V2.16B (XOR) */
    translate_eor_vec(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_bic_vec)
{
    setup_test_buf();

    /* BIC V0.16B, V1.16B, V2.16B (AND NOT) */
    translate_bic_vec(&test_buf, 0, 1, 2);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Floating Point Tests
 * ============================================================================ */

TEST(translate_fmov_reg)
{
    setup_test_buf();

    /* FMOV D0, D1 (double) */
    translate_fmov_reg(&test_buf, 0, 1, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fadd)
{
    setup_test_buf();

    /* FADD D0, D1, D2 (double) */
    translate_fadd(&test_buf, 0, 1, 2, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fsub)
{
    setup_test_buf();

    /* FSUB D0, D1, D2 */
    translate_fsub(&test_buf, 0, 1, 2, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fmul)
{
    setup_test_buf();

    /* FMUL D0, D1, D2 */
    translate_fmul(&test_buf, 0, 1, 2, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fdiv)
{
    setup_test_buf();

    /* FDIV D0, D1, D2 */
    translate_fdiv(&test_buf, 0, 1, 2, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fsqrt)
{
    setup_test_buf();

    /* FSQRT D0, D1 */
    translate_fsqrt(&test_buf, 0, 1, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fcmp)
{
    setup_test_buf();

    /* FCMP D0, D1 */
    translate_fcmp(&test_buf, 0, 1, 1);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fcvtds)
{
    setup_test_buf();

    /* FCVTDS S0, D0 (double to single) */
    translate_fcvtds(&test_buf, 0, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

TEST(translate_fcvtsd)
{
    setup_test_buf();

    /* FCVTSD D0, S0 (single to double) */
    translate_fcvtsd(&test_buf, 0, 0);

    ASSERT(code_buffer_get_size(&test_buf) > 0);

    return 1;
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

TEST(sequence_add_sub_mul)
{
    setup_test_buf();

    /* X0 = X1 + X2; X0 = X0 - X3; X0 = X0 * X4 */
    translate_add_reg(&test_buf, 0, 1, 2);
    translate_sub_reg(&test_buf, 0, 0, 3);
    translate_mul_reg(&test_buf, 0, 0, 4);

    /* Should have emitted multiple instructions */
    ASSERT(code_buffer_get_size(&test_buf) > 10);

    return 1;
}

TEST(sequence_load_compute_store)
{
    setup_test_buf();

    /* X0 = [X1 + 0]; X0 = X0 + 42; [X1 + 8] = X0 */
    translate_ldr_imm(&test_buf, 0, 1, 0);
    translate_add_imm(&test_buf, 0, 0, 42);
    translate_str_imm(&test_buf, 0, 1, 8);

    ASSERT(code_buffer_get_size(&test_buf) > 10);

    return 1;
}

TEST(sequence_movz_movk_64bit)
{
    setup_test_buf();

    /* Build 64-bit constant 0x123456789ABCDEF0 */
    translate_movz(&test_buf, 0, 0xCDEF, 0);
    translate_movk(&test_buf, 0, 0x9ABC, 1);
    translate_movk(&test_buf, 0, 0x5678, 2);
    translate_movk(&test_buf, 0, 0x1234, 3);

    ASSERT(code_buffer_get_size(&test_buf) > 10);

    return 1;
}

TEST(sequence_branch_chain)
{
    setup_test_buf();

    /* CMP X0, 0; B.EQ label */
    translate_cmp_reg(&test_buf, 0, 1);
    u32 br_offset = translate_cond_branch(&test_buf, 0);  /* EQ */

    ASSERT_NEQ(br_offset, 0);

    return 1;
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(void)
{
    printf("==================================================\n");
    printf("Rosetta Translation Module Unit Tests\n");
    printf("==================================================\n\n");

    /* Register mapping tests */
    printf("--- Register Mapping Tests ---\n");
    RUN_TEST(x86_map_gpr_basic);
    RUN_TEST(x86_map_gpr_high);
    RUN_TEST(x86_map_gpr_wrap);
    RUN_TEST(x86_map_xmm_basic);
    RUN_TEST(x86_map_xmm_wrap);
    printf("\n");

    /* Data processing - register tests */
    printf("--- Data Processing (Register) Tests ---\n");
    RUN_TEST(translate_add_reg);
    RUN_TEST(translate_sub_reg);
    RUN_TEST(translate_and_reg);
    RUN_TEST(translate_orr_reg);
    RUN_TEST(translate_eor_reg);
    RUN_TEST(translate_mvn_reg);
    RUN_TEST(translate_mul_reg);
    RUN_TEST(translate_sdiv_reg);
    printf("\n");

    /* Data processing - immediate tests */
    printf("--- Data Processing (Immediate) Tests ---\n");
    RUN_TEST(translate_add_imm);
    RUN_TEST(translate_sub_imm);
    RUN_TEST(translate_movz);
    RUN_TEST(translate_movk);
    RUN_TEST(translate_movn);
    printf("\n");

    /* Compare tests */
    printf("--- Compare Tests ---\n");
    RUN_TEST(translate_cmp_reg);
    RUN_TEST(translate_cmn_reg);
    RUN_TEST(translate_tst_reg);
    printf("\n");

    /* Branch tests */
    printf("--- Branch Tests ---\n");
    RUN_TEST(translate_b);
    RUN_TEST(translate_bl);
    RUN_TEST(translate_br);
    RUN_TEST(translate_ret);
    RUN_TEST(translate_cond_branch);
    printf("\n");

    /* Load/Store tests */
    printf("--- Load/Store Tests ---\n");
    RUN_TEST(translate_ldr_imm);
    RUN_TEST(translate_str_imm);
    RUN_TEST(translate_ldrb_imm);
    RUN_TEST(translate_strb_imm);
    RUN_TEST(translate_ldrh_imm);
    RUN_TEST(translate_strh_imm);
    RUN_TEST(translate_ldrsw_imm);
    RUN_TEST(translate_ldur);
    RUN_TEST(translate_stur);
    printf("\n");

    /* Load/Store pair tests */
    printf("--- Load/Store Pair Tests ---\n");
    RUN_TEST(translate_ldp);
    RUN_TEST(translate_stp);
    printf("\n");

    /* System tests */
    printf("--- System Tests ---\n");
    RUN_TEST(translate_svc);
    RUN_TEST(translate_brk);
    RUN_TEST(translate_hlt);
    printf("\n");

    /* NEON load/store tests */
    printf("--- NEON Load/Store Tests ---\n");
    RUN_TEST(translate_ld1);
    RUN_TEST(translate_st1);
    RUN_TEST(translate_ld2);
    RUN_TEST(translate_st2);
    RUN_TEST(translate_ld3);
    RUN_TEST(translate_st3);
    RUN_TEST(translate_ld4);
    RUN_TEST(translate_st4);
    printf("\n");

    /* NEON arithmetic tests */
    printf("--- NEON Arithmetic Tests ---\n");
    RUN_TEST(translate_add_vec);
    RUN_TEST(translate_sub_vec);
    RUN_TEST(translate_and_vec);
    RUN_TEST(translate_orr_vec);
    RUN_TEST(translate_eor_vec);
    RUN_TEST(translate_bic_vec);
    printf("\n");

    /* Floating point tests */
    printf("--- Floating Point Tests ---\n");
    RUN_TEST(translate_fmov_reg);
    RUN_TEST(translate_fadd);
    RUN_TEST(translate_fsub);
    RUN_TEST(translate_fmul);
    RUN_TEST(translate_fdiv);
    RUN_TEST(translate_fsqrt);
    RUN_TEST(translate_fcmp);
    RUN_TEST(translate_fcvtds);
    RUN_TEST(translate_fcvtsd);
    printf("\n");

    /* Integration tests */
    printf("--- Integration Tests ---\n");
    RUN_TEST(sequence_add_sub_mul);
    RUN_TEST(sequence_load_compute_store);
    RUN_TEST(sequence_movz_movk_64bit);
    RUN_TEST(sequence_branch_chain);
    printf("\n");

    /* Summary */
    printf("==================================================\n");
    printf("Tests Run: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("==================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
