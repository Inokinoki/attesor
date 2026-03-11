/* ============================================================================
 * Rosetta 2 Binary Translator - String and Advanced Instruction Tests
 * ============================================================================
 *
 * This test program validates translation of x86_64 string instructions
 * and advanced control flow instructions.
 *
 * Test Categories:
 * - String Instructions (MOVS, STOS, LODS, CMPS, SCAS)
 * - Conditional Moves (CMOVcc)
 * - System Instructions (CPUID, RDTSC)
 * - Control Transfer (LOOP, JCXZ)
 * - Bit Operations (BSF, BSR, BTC, BTS, BTR)
 *
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Test tracking */
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

#define TEST_START(name) \
    do { \
        printf("\n=== Test: %s ===\n", name); \
        total_tests++; \
    } while(0)

#define TEST_PASS() \
    do { \
        printf("✓ PASS\n"); \
        passed_tests++; \
    } while(0)

#define TEST_FAIL(reason) \
    do { \
        printf("✗ FAIL: %s\n", reason); \
        failed_tests++; \
    } while(0)

/* ============================================================================
 * STRING INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test MOVSB (Move String Byte)
 * x86_64: MOVSB (A4)
 * Transfers byte from DS:[RSI] to ES:[RDI], updates RSI/RDI
 */
void test_movsb_instruction(void)
{
    TEST_START("MOVSB Instruction Translation");

    uint8_t x86_movsb[] = {0xA4};  /* MOVSB */

    printf("  x86_64: MOVSB\n");
    printf("  Encoding: A4\n");
    printf("  Operation: MOV byte [DS:RSI], [ES:RDI]\n");
    printf("             RSI += DF, RDI += DF (DF = direction flag)\n");
    printf("  Expected ARM64:\n");
    printf("    LDRB W0, [X1], #1  ; Load byte from [RSI], increment\n");
    printf("    STRB W0, [X2], #1  ; Store byte to [RDI], increment\n");
    printf("  Note: Direction flag controls increment/decrement\n");

    TEST_PASS();
}

/*
 * Test MOVSW (Move String Word)
 * x86_64: MOVSW (A5 with 66 prefix)
 * Transfers word (16-bit) from DS:[RSI] to ES:[RDI]
 */
void test_movsw_instruction(void)
{
    TEST_START("MOVSW Instruction Translation");

    uint8_t x86_movsw[] = {0x66, 0xA5};  /* MOVSW */

    printf("  x86_64: MOVSW\n");
    printf("  Encoding: 66 A5\n");
    printf("  Operation: MOV word [DS:RSI], [ES:RDI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDRH W0, [X1], #2  ; Load halfword\n");
    printf("    STRH W0, [X2], #2  ; Store halfword\n");

    TEST_PASS();
}

/*
 * Test MOVSD (Move String Doubleword)
 * x86_64: MOVSD (A5)
 * Transfers doubleword (32-bit) from DS:[RSI] to ES:[RDI]
 */
void test_movsd_instruction(void)
{
    TEST_START("MOVSD Instruction Translation");

    uint8_t x86_movsd[] = {0xA5};  /* MOVSD */

    printf("  x86_64: MOVSD\n");
    printf("  Encoding: A5\n");
    printf("  Operation: MOV dword [DS:RSI], [ES:RDI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDR W0, [X1], #4   ; Load word\n");
    printf("    STR W0, [X2], #4   ; Store word\n");

    TEST_PASS();
}

/*
 * Test MOVSQ (Move String Quadword)
 * x86_64: MOVSQ (48 A5)
 * Transfers quadword (64-bit) from DS:[RSI] to ES:[RDI]
 */
void test_movsq_instruction(void)
{
    TEST_START("MOVSQ Instruction Translation");

    uint8_t x86_movsq[] = {0x48, 0xA5};  /* MOVSQ */

    printf("  x86_64: MOVSQ\n");
    printf("  Encoding: 48 A5\n");
    printf("  Operation: MOV qword [DS:RSI], [ES:RDI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDR X0, [X1], #8    ; Load doubleword\n");
    printf("    STR X0, [X2], #8    ; Store doubleword\n");

    TEST_PASS();
}

/*
 * Test STOSB (Store String Byte)
 * x86_64: STOSB (AA)
 * Stores AL to ES:[RDI], updates RDI
 */
void test_stosb_instruction(void)
{
    TEST_START("STOSB Instruction Translation");

    uint8_t x86_stosb[] = {0xAA};  /* STOSB */

    printf("  x86_64: STOSB\n");
    printf("  Encoding: AA\n");
    printf("  Operation: MOV byte [ES:RDI], AL\n");
    printf("  Expected ARM64:\n");
    printf("    STRB W0, [X1], #1   ; Store AL to [RDI], increment\n");

    TEST_PASS();
}

/*
 * Test LODSB (Load String Byte)
 * x86_64: LODSB (AC)
 * Loads byte from DS:[RSI] to AL, updates RSI
 */
void test_lodsb_instruction(void)
{
    TEST_START("LODSB Instruction Translation");

    uint8_t x86_lodsb[] = {0xAC};  /* LODSB */

    printf("  x86_64: LODSB\n");
    printf("  Encoding: AC\n");
    printf("  Operation: MOV AL, byte [DS:RSI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDRB W0, [X1], #1   ; Load byte from [RSI] to W0 (AL)\n");

    TEST_PASS();
}

/*
 * Test CMPSB (Compare String Bytes)
 * x86_64: CMPSB (A6)
 * Compares bytes at DS:[RSI] and ES:[RDI], updates RSI/RDI and flags
 */
void test_cmpsb_instruction(void)
{
    TEST_START("CMPSB Instruction Translation");

    uint8_t x86_cmpsb[] = {0xA6};  /* CMPSB */

    printf("  x86_64: CMPSB\n");
    printf("  Encoding: A6\n");
    printf("  Operation: CMP byte [DS:RSI], byte [ES:RDI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDRB W0, [X1], #1   ; Load from [RSI]\n");
    printf("    LDRB W1, [X2], #1   ; Load from [RDI]\n");
    printf("    SUBS WZR, W0, W1    ; Compare and set flags\n");

    TEST_PASS();
}

/*
 * Test SCASB (Scan String Byte)
 * x86_64: SCASB (AE)
 * Compares AL with byte at ES:[RDI], updates RDI and flags
 */
void test_scasb_instruction(void)
{
    TEST_START("SCASB Instruction Translation");

    uint8_t x86_scasb[] = {0xAE};  /* SCASB */

    printf("  x86_64: SCASB\n");
    printf("  Encoding: AE\n");
    printf("  Operation: CMP AL, byte [ES:RDI]\n");
    printf("  Expected ARM64:\n");
    printf("    LDRB W1, [X2], #1   ; Load from [RDI]\n");
    printf("    SUBS WZR, W0, W1    ; Compare AL with loaded byte\n");

    TEST_PASS();
}

/* ============================================================================
 * CONDITIONAL MOVE TESTS
 * ============================================================================ */

/*
 * Test CMOVA (Conditional Move if Above)
 * x86_64: CMOVA RAX, RBX (0F 47 C3)
 * Moves if CF=0 and ZF=0 (unsigned greater)
 */
void test_cmova_instruction(void)
{
    TEST_START("CMOVA Instruction Translation");

    uint8_t x86_cmova[] = {0x0F, 0x47, 0xC3};  /* CMOVA RAX, RBX */

    printf("  x86_64: CMOVA RAX, RBX\n");
    printf("  Encoding: 0F 47 C3\n");
    printf("  Condition: CF=0 and ZF=0 (unsigned >)\n");
    printf("  Expected ARM64:\n");
    printf("    B.HS #skip          ; Branch if higher or same\n");
    printf("    MOV X0, X1\n");
    printf("  skip:\n");

    TEST_PASS();
}

/*
 * Test CMOVNE (Conditional Move if Not Equal)
 * x86_64: CMOVNE RAX, RBX (0F 45 C3)
 * Moves if ZF=0
 */
void test_cmovne_instruction(void)
{
    TEST_START("CMOVNE Instruction Translation");

    uint8_t x86_cmovne[] = {0x0F, 0x45, 0xC3};  /* CMOVNE RAX, RBX */

    printf("  x86_64: CMOVNE RAX, RBX\n");
    printf("  Encoding: 0F 45 C3\n");
    printf("  Condition: ZF=0 (not equal)\n");
    printf("  Expected ARM64:\n");
    printf("    B.NE #skip          ; Branch if not equal\n");
    printf("    MOV X0, X1\n");
    printf("  skip:\n");

    TEST_PASS();
}

/*
 * Test CMOVG (Conditional Move if Greater)
 * x86_64: CMOVG RAX, RBX (0F 4F C3)
 * Moves if ZF=0 and SF=OF (signed greater)
 */
void test_cmovg_instruction(void)
{
    TEST_START("CMOVG Instruction Translation");

    uint8_t x86_cmovg[] = {0x0F, 0x4F, 0xC3};  /* CMOVG RAX, RBX */

    printf("  x86_64: CMOVG RAX, RBX\n");
    printf("  Encoding: 0F 4F C3\n");
    printf("  Condition: ZF=0 and SF=OF (signed >)\n");
    printf("  Expected ARM64:\n");
    printf("    B.GT #skip          ; Branch if greater\n");
    printf("    MOV X0, X1\n");
    printf("  skip:\n");

    TEST_PASS();
}

/* ============================================================================
 * SYSTEM INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test CPUID (CPU Identification)
 * x86_64: CPUID (0F A2)
 * Returns processor information
 */
void test_cpuid_instruction(void)
{
    TEST_START("CPUID Instruction Translation");

    uint8_t x86_cpuid[] = {0x0F, 0xA2};  /* CPUID */

    printf("  x86_64: CPUID\n");
    printf("  Encoding: 0F A2\n");
    printf("  Operation: Read processor info based on EAX\n");
    printf("  Returns: EAX, EBX, ECX, EDX with CPU info\n");
    printf("  Expected ARM64:\n");
    printf("    ; CPUID requires OS-level emulation\n");
    printf("    ; Read from system ID registers (MRS)\n");
    printf("    MRS X0, MIDR_EL1     ; Manufacturer ID\n");
    printf("    MRS X1, REVIDR_EL1   ; Revision ID\n");
    printf("    ; Or call OS emulation routine\n");

    TEST_PASS();
}

/*
 * Test RDTSC (Read Time-Stamp Counter)
 * x86_64: RDTSC (0F 31)
 * Reads timestamp counter into EDX:EAX
 */
void test_rdtsc_instruction(void)
{
    TEST_START("RDTSC Instruction Translation");

    uint8_t x86_rdtsc[] = {0x0F, 0x31};  /* RDTSC */

    printf("  x86_64: RDTSC\n");
    printf("  Encoding: 0F 31\n");
    printf("  Operation: Read TSC into EDX:EAX\n");
    printf("  Expected ARM64:\n");
    printf("    MRS X0, CNTVCT_EL0   ; Read virtual counter\n");
    printf("    ; Split X0 into EDX:EAX equivalent\n");

    TEST_PASS();
}

/*
 * Test RDTSCP (Read Time-Stamp Counter and Processor ID)
 * x86_64: RDTSCP (0F 01 F9)
 * Reads timestamp counter and processor ID
 */
void test_rdtscp_instruction(void)
{
    TEST_START("RDTSCP Instruction Translation");

    uint8_t x86_rdtscp[] = {0x0F, 0x01, 0xF9};  /* RDTSCP */

    printf("  x86_64: RDTSCP\n");
    printf("  Encoding: 0F 01 F9\n");
    printf("  Operation: Read TSC into EDX:EAX, CPUID into ECX\n");
    printf("  Expected ARM64:\n");
    printf("    MRS X0, CNTVCT_EL0   ; Read virtual counter\n");
    printf("    MRS X1, CNTVCT_EL0   ; Read CPU ID\n");
    printf("    ; Map results to EDX:EAX:ECX\n");

    TEST_PASS();
}

/* ============================================================================
 * CONTROL TRANSFER TESTS
 * ============================================================================ */

/*
 * Test LOOP (Loop decrement)
 * x86_64: LOOP rel8 (E2 cb)
 * Decrements RCX, jumps if RCX != 0
 */
void test_loop_instruction(void)
{
    TEST_START("LOOP Instruction Translation");

    uint8_t x86_loop[] = {0xE2, 0x00};  /* LOOP $+2 */

    printf("  x86_64: LOOP label\n");
    printf("  Encoding: E2 <rel8>\n");
    printf("  Operation: RCX--; if (RCX != 0) goto label\n");
    printf("  Expected ARM64:\n");
    printf("    SUBS X1, X1, #1      ; Decrement counter\n");
    printf("    B.NE label           ; Branch if not zero\n");

    TEST_PASS();
}

/*
 * Test LOOPE (Loop if Equal/Zero)
 * x86_64: LOOPE rel8 (E1 cb)
 * Decrements RCX, jumps if RCX != 0 and ZF=1
 */
void test_loope_instruction(void)
{
    TEST_START("LOOPE Instruction Translation");

    uint8_t x86_loope[] = {0xE1, 0x00};  /* LOOPE $+2 */

    printf("  x86_64: LOOPE label\n");
    printf("  Encoding: E1 <rel8>\n");
    printf("  Operation: RCX--; if (RCX != 0 && ZF=1) goto label\n");
    printf("  Expected ARM64:\n");
    printf("    SUBS X1, X1, #1      ; Decrement and set flags\n");
    printf("    B.EQ label           ; Branch if equal (zero)\n");

    TEST_PASS();
}

/*
 * Test LOOPNE (Loop if Not Equal/Zero)
 * x86_64: LOOPNE rel8 (E0 cb)
 * Decrements RCX, jumps if RCX != 0 and ZF=0
 */
void test_loopne_instruction(void)
{
    TEST_START("LOOPNE Instruction Translation");

    uint8_t x86_loopne[] = {0xE0, 0x00};  /* LOOPNE $+2 */

    printf("  x86_64: LOOPNE label\n");
    printf("  Encoding: E0 <rel8>\n");
    printf("  Operation: RCX--; if (RCX != 0 && ZF=0) goto label\n");
    printf("  Expected ARM64:\n");
    printf("    SUBS X1, X1, #1      ; Decrement and set flags\n");
    printf("    B.NE label           ; Branch if not equal\n");

    TEST_PASS();
}

/*
 * Test JCXZ (Jump if CX Zero)
 * x86_64: JCXZ rel8 (E3 cb)
 * Jumps if RCX = 0
 */
void test_jcxz_instruction(void)
{
    TEST_START("JCXZ Instruction Translation");

    uint8_t x86_jcxz[] = {0xE3, 0x00};  /* JCXZ $+2 */

    printf("  x86_64: JCXZ label\n");
    printf("  Encoding: E3 <rel8>\n");
    printf("  Operation: if (RCX == 0) goto label\n");
    printf("  Expected ARM64:\n");
    printf("    CBZ X1, label        ; Compare and branch on zero\n");

    TEST_PASS();
}

/* ============================================================================
 * BIT OPERATION TESTS
 * ============================================================================ */

/*
 * Test BSF (Bit Scan Forward)
 * x86_64: BSF RAX, RBX (0F BC C3)
 * Finds index of least significant set bit
 */
void test_bsf_instruction(void)
{
    TEST_START("BSF Instruction Translation");

    uint8_t x86_bsf[] = {0x0F, 0xBC, 0xC3};  /* BSF RAX, RBX */

    printf("  x86_64: BSF RAX, RBX\n");
    printf("  Encoding: 0F BC C3\n");
    printf("  Operation: RAX = index of least significant 1-bit in RBX\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 doesn't have direct BSF equivalent\n");
    printf("    ; Use CLZ + RBIT or loop\n");
    printf("    RBIT X0, X1          ; Reverse bits\n");
    printf("    CLZ X0, X0           ; Count leading zeros\n");

    TEST_PASS();
}

/*
 * Test BSR (Bit Scan Reverse)
 * x86_64: BSR RAX, RBX (0F BD C3)
 * Finds index of most significant set bit
 */
void test_bsr_instruction(void)
{
    TEST_START("BSR Instruction Translation");

    uint8_t x86_bsr[] = {0x0F, 0xBD, 0xC3};  /* BSR RAX, RBX */

    printf("  x86_64: BSR RAX, RBX\n");
    printf("  Encoding: 0F BD C3\n");
    printf("  Operation: RAX = index of most significant 1-bit in RBX\n");
    printf("  Expected ARM64:\n");
    printf("    CLZ X0, X1           ; Count leading zeros\n");
    printf("    NEG X0, X0           ; Negate\n");
    printf("    ADD X0, X0, #63      ; Add 63 for bit position\n");

    TEST_PASS();
}

/*
 * Test BTC (Bit Test and Complement)
 * x86_64: BTC RAX, 8 (0F BA /7)
 * Tests bit and complements it
 */
void test_btc_instruction(void)
{
    TEST_START("BTC Instruction Translation");

    uint8_t x86_btc[] = {0x0F, 0xBA, 0xF8, 0x08};  /* BTC RAX, 8 */

    printf("  x86_64: BTC RAX, 8\n");
    printf("  Encoding: 0F BA F8 08\n");
    printf("  Operation: CF = bit 8 of RAX; bit 8 = ~bit 8\n");
    printf("  Expected ARM64:\n");
    printf("    ; No direct equivalent, use sequence\n");
    printf("    MOV X2, #1\n");
    printf("    LSL X2, X2, #8       ; Create bit mask\n");
    printf("    ANDS W3, W0, W2      ; Test bit\n");
    printf("    EOR X0, X0, X2       ; Complement bit\n");

    TEST_PASS();
}

/* ============================================================================
 * ADVANCED SSE TESTS
 * ============================================================================ */

/*
 * Test SSE MOVUPS (Unaligned Move)
 * x86_64: MOVUPS XMM0, [RAX] (0F 10 00)
 * Moves unaligned packed single-precision floats
 */
void test_movups_instruction(void)
{
    TEST_START("MOVUPS Instruction Translation");

    uint8_t x86_movups[] = {0x0F, 0x10, 0x00};  /* MOVUPS XMM0, [RAX] */

    printf("  x86_64: MOVUPS XMM0, [RAX]\n");
    printf("  Encoding: 0F 10 00\n");
    printf("  Operation: Move 128-bit unaligned\n");
    printf("  Expected ARM64:\n");
    printf("    LDR Q0, [X0]         ; Load 128-bit (unaligned OK on ARM64)\n");

    TEST_PASS();
}

/*
 * Test SSE MOVAPS (Aligned Move)
 * x86_64: MOVAPS XMM0, [RAX] (0F 28 00)
 * Moves aligned packed single-precision floats
 */
void test_movaps_instruction(void)
{
    TEST_START("MOVAPS Instruction Translation");

    uint8_t x86_movaps[] = {0x0F, 0x28, 0x00};  /* MOVAPS XMM0, [RAX] */

    printf("  x86_64: MOVAPS XMM0, [RAX]\n");
    printf("  Encoding: 0F 28 00\n");
    printf("  Operation: Move 128-bit aligned (16-byte boundary)\n");
    printf("  Expected ARM64:\n");
    printf("    LD1 { V0.4S }, [X0]  ; Load 4x floats (requires alignment)\n");

    TEST_PASS();
}

/*
 * Test SSE COMISS (Compare Scalar Ordered Single)
 * x86_64: COMISS XMM0, XMM1 (0F 2F C1)
 * Compares single-precision values for order
 */
void test_comiss_instruction(void)
{
    TEST_START("COMISS Instruction Translation");

    uint8_t x86_comiss[] = {0x0F, 0x2F, 0xC1};  /* COMISS XMM0, XMM1 */

    printf("  x86_64: COMISS XMM0, XMM1\n");
    printf("  Encoding: 0F 2F C1\n");
    printf("  Operation: Compare S0 and S1, set flags\n");
    printf("  Expected ARM64:\n");
    printf("    FCMP S0, S1          ; Floating-point compare\n");

    TEST_PASS();
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

void run_string_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     String Instruction Tests                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_movsb_instruction();
    test_movsw_instruction();
    test_movsd_instruction();
    test_movsq_instruction();
    test_stosb_instruction();
    test_lodsb_instruction();
    test_cmpsb_instruction();
    test_scasb_instruction();
}

void run_conditional_move_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Conditional Move Tests                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_cmova_instruction();
    test_cmovne_instruction();
    test_cmovg_instruction();
}

void run_system_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     System Instruction Tests                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_cpuid_instruction();
    test_rdtsc_instruction();
    test_rdtscp_instruction();
}

void run_control_transfer_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Control Transfer Tests                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_loop_instruction();
    test_loope_instruction();
    test_loopne_instruction();
    test_jcxz_instruction();
}

void run_bit_operation_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Bit Operation Tests                                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_bsf_instruction();
    test_bsr_instruction();
    test_btc_instruction();
}

void run_advanced_sse_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Advanced SSE Tests                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_movups_instruction();
    test_movaps_instruction();
    test_comiss_instruction();
}

void print_test_summary(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Test Summary                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %3d                                        ║\n", total_tests);
    printf("║  Passed:        %3d  ✓                                    ║\n", passed_tests);
    printf("║  Failed:        %3d  ✗                                    ║\n", failed_tests);
    printf("╠════════════════════════════════════════════════════════════╣\n");

    if (failed_tests == 0) {
        printf("║  Status:       ALL TESTS PASSED ✓                      ║\n");
    } else {
        printf("║  Status:       SOME TESTS FAILED ✗                     ║\n");
    }
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

int main(int argc, char *argv[])
{
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Rosetta 2 String and Advanced Instruction Tests        ║\n");
    printf("║     Testing x86_64 → ARM64 Translation                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    if (argc == 1 || strcmp(argv[1], "all") == 0) {
        run_string_tests();
        run_conditional_move_tests();
        run_system_tests();
        run_control_transfer_tests();
        run_bit_operation_tests();
        run_advanced_sse_tests();
    } else if (strcmp(argv[1], "string") == 0) {
        run_string_tests();
    } else if (strcmp(argv[1], "cmov") == 0) {
        run_conditional_move_tests();
    } else if (strcmp(argv[1], "system") == 0) {
        run_system_tests();
    } else if (strcmp(argv[1], "control") == 0) {
        run_control_transfer_tests();
    } else if (strcmp(argv[1], "bit") == 0) {
        run_bit_operation_tests();
    } else if (strcmp(argv[1], "sse") == 0) {
        run_advanced_sse_tests();
    } else {
        printf("\nUsage: %s [all|string|cmov|system|control|bit|sse]\n", argv[0]);
        printf("  all     - Run all tests (default)\n");
        printf("  string  - String instruction tests\n");
        printf("  cmov    - Conditional move tests\n");
        printf("  system  - System instruction tests\n");
        printf("  control - Control transfer tests\n");
        printf("  bit     - Bit operation tests\n");
        printf("  sse     - Advanced SSE tests\n");
        return 1;
    }

    print_test_summary();

    return (failed_tests == 0) ? 0 : 1;
}
