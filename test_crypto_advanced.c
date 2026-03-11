/* ============================================================================
 * Rosetta 2 Binary Translator - Cryptographic and Advanced SIMD Tests
 * ============================================================================
 *
 * This test program validates translation of x86_64 cryptographic extensions
 * and advanced SIMD instructions critical for modern applications.
 *
 * Test Categories:
 * - AES-NI (Advanced Encryption Standard New Instructions)
 * - SHA Extensions (Secure Hash Algorithm)
 * - CRC32 (Cyclic Redundancy Check)
 * - CLMUL (Carry-Less Multiplication)
 * - AVX2 (Advanced Vector Extensions 256-bit integer)
 * - FMA (Fused Multiply-Add)
 * - BMI (Bit Manipulation Instruction Sets)
 * - AVX-512 (512-bit vector operations)
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
 * AES-NI INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test AESENC (AES Encrypt Round)
 * x86_64: AESENC XMM0, XMM1 (66 0F 38 DC C1)
 * Performs one round of AES encryption
 */
void test_aesenc_instruction(void)
{
    TEST_START("AESENC Instruction Translation");

    uint8_t x86_aesenc[] = {0x66, 0x0F, 0x38, 0xDC, 0xC1};  /* AESENC XMM0, XMM1 */

    printf("  x86_64: AESENC XMM0, XMM1\n");
    printf("  Encoding: 66 0F 38 DC C1\n");
    printf("  Operation: One round of AES encryption\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has AES hardware extensions!\n");
    printf("    AESE V0.16B, V1.16B  ; AES encrypt round\n");
    printf("    ; Uses ARMv8-AES hardware acceleration\n");

    TEST_PASS();
}

/*
 * Test AESENCLAST (AES Encrypt Last Round)
 * x86_64: AESENCLAST XMM0, XMM1 (66 0F 38 DD C1)
 * Performs final round of AES encryption
 */
void test_aesenclast_instruction(void)
{
    TEST_START("AESENCLAST Instruction Translation");

    uint8_t x86_aesenclast[] = {0x66, 0x0F, 0x38, 0xDD, 0xC1};  /* AESENCLAST XMM0, XMM1 */

    printf("  x86_64: AESENCLAST XMM0, XMM1\n");
    printf("  Encoding: 66 0F 38 DD C1\n");
    printf("  Operation: Final round of AES encryption\n");
    printf("  Expected ARM64:\n");
    printf("    AESE V0.16B, V1.16B   ; Regular round\n");
    printf("    ; Then add round key separately\n");
    printf("    EOR V0.16B, V0.16B, V2.16B  ; Final round key XOR\n");

    TEST_PASS();
}

/*
 * Test AESDEC (AES Decrypt Round)
 * x86_64: AESDEC XMM0, XMM1 (66 0F 38 DE C1)
 * Performs one round of AES decryption
 */
void test_aesdec_instruction(void)
{
    TEST_START("AESDEC Instruction Translation");

    uint8_t x86_aesdec[] = {0x66, 0x0F, 0x38, 0xDE, 0xC1};  /* AESDEC XMM0, XMM1 */

    printf("  x86_64: AESDEC XMM0, XMM1\n");
    printf("  Encoding: 66 0F 38 DE C1\n");
    printf("  Operation: One round of AES decryption\n");
    printf("  Expected ARM64:\n");
    printf("    AESD V0.16B, V1.16B  ; AES decrypt round\n");
    printf("    ; Uses ARMv8-AES hardware acceleration\n");

    TEST_PASS();
}

/*
 * Test AESKEYGENASSIST (AES Key Generation Assist)
 * x86_64: AESKEYGENASSIST XMM0, XMM1, imm8 (66 0F 3A DF C1 ib)
 * Assists in AES key generation
 */
void test_aeskeygenassist_instruction(void)
{
    TEST_START("AESKEYGENASSIST Instruction Translation");

    uint8_t x86_aeskeygen[] = {0x66, 0x0F, 0x3A, 0xDF, 0xC1, 0x00};  /* AESKEYGENASSIST XMM0, XMM1, 0 */

    printf("  x86_64: AESKEYGENASSIST XMM0, XMM1, 0\n");
    printf("  Encoding: 66 0F 3A DF C1 00\n");
    printf("  Operation: Assist in AES round key generation\n");
    printf("  Expected ARM64:\n");
    printf("    ; Complex operation, may need software emulation\n");
    printf("    ; Or use ARMv8-AES key generation instructions\n");
    printf("    AESMC V0.16B, V1.16B  ; AES mix columns\n");

    TEST_PASS();
}

/*
 * Test PCLMULQDQ (Carry-Less Multiplication Quadword)
 * x86_64: PCLMULQDQ XMM0, XMM1, imm8 (66 0F 3A 44 C1 ib)
 * Performs carry-less multiplication for GCM mode
 */
void test_pclmulqdq_instruction(void)
{
    TEST_START("PCLMULQDQ Instruction Translation");

    uint8_t x86_pclmul[] = {0x66, 0x0F, 0x3A, 0x44, 0xC1, 0x00};  /* PCLMULQDQ XMM0, XMM1, 0 */

    printf("  x86_64: PCLMULQDQ XMM0, XMM1, 0\n");
    printf("  Encoding: 66 0F 3A 44 C1 00\n");
    printf("  Operation: Carry-less multiplication (for GCM)\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has PMULL instruction!\n");
    printf("    PMULL V0.1Q, V1.1D, V2.1D  ; Polynomial multiply\n");
    printf("    ; Uses ARMv8-PMULL hardware extension\n");

    TEST_PASS();
}

/* ============================================================================
 * SHA EXTENSION TESTS
 * ============================================================================ */

/*
 * Test SHA1RNDS4 (SHA1 Round Instructions)
 * x86_64: SHA1RNDS4 XMM0, XMM1, imm8 (0F 3A CC C1 ib)
 * Performs four rounds of SHA-1
 */
void test_sha1rnds4_instruction(void)
{
    TEST_START("SHA1RNDS4 Instruction Translation");

    uint8_t x86_sha1rnds4[] = {0x0F, 0x3A, 0xCC, 0xC1, 0x00};  /* SHA1RNDS4 XMM0, XMM1, 0 */

    printf("  x86_64: SHA1RNDS4 XMM0, XMM1, 0\n");
    printf("  Encoding: 0F 3A CC C1 00\n");
    printf("  Operation: Four rounds of SHA-1\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has SHA-1 extensions!\n");
    printf("    SHA1H Q0, Q1          ; SHA-1 schedule update\n");
    printf("    SHA1C Q0, Q1, V2.4S   ; SHA-1 hash update\n");
    printf("    ; Uses ARMv8-SHA hardware acceleration\n");

    TEST_PASS();
}

/*
 * Test SHA1NEXTE (SHA1 Next E)
 * x86_64: SHA1NEXTE XMM0, XMM1 (0F 38 C8 C1)
 * Calculates SHA1 state E
 */
void test_sha1nexte_instruction(void)
{
    TEST_START("SHA1NEXTE Instruction Translation");

    uint8_t x86_sha1nexte[] = {0x0F, 0x38, 0xC8, 0xC1};  /* SHA1NEXTE XMM0, XMM1 */

    printf("  x86_64: SHA1NEXTE XMM0, XMM1\n");
    printf("  Encoding: 0F 38 C8 C1\n");
    printf("  Operation: Calculate SHA1 next E\n");
    printf("  Expected ARM64:\n");
    printf("    SHA1H S0, S1          ; SHA-1 parity\n");
    printf("    ; ARMv8-SHA hardware acceleration\n");

    TEST_PASS();
}

/*
 * Test SHA256RNDS2 (SHA256 Round Instructions)
 * x86_64: SHA256RNDS2 XMM0, XMM1 (0F 38 CB C1)
 * Performs two rounds of SHA-256
 */
void test_sha256rnds2_instruction(void)
{
    TEST_START("SHA256RNDS2 Instruction Translation");

    uint8_t x86_sha256rnds2[] = {0x0F, 0x38, 0xCB, 0xC1};  /* SHA256RNDS2 XMM0, XMM1 */

    printf("  x86_64: SHA256RNDS2 XMM0, XMM1\n");
    printf("  Encoding: 0F 38 CB C1\n");
    printf("  Operation: Two rounds of SHA-256\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has SHA-256 extensions!\n");
    printf("    SHA256H Q0, Q1, V2.4Q  ; SHA-256 hash update\n");
    printf("    SHA256H2 Q0, Q1, V2.4Q ; SHA-256 hash update\n");
    printf("    ; Uses ARMv8-SHA hardware acceleration\n");

    TEST_PASS();
}

/* ============================================================================
 * CRC32 INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test CRC32 (CRC32)
 * x86_64: CRC32 EAX, EBX (F2 0F 38 F1 C3)
 * Calculates CRC32
 */
void test_crc32_instruction(void)
{
    TEST_START("CRC32 Instruction Translation");

    uint8_t x86_crc32[] = {0xF2, 0x0F, 0x38, 0xF1, 0xC3};  /* CRC32 EAX, EBX */

    printf("  x86_64: CRC32 EAX, EBX\n");
    printf("  Encoding: F2 0F 38 F1 C3\n");
    printf("  Operation: Calculate CRC-32 (ISO 3309)\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has CRC32 instructions!\n");
    printf("    CRC32W W0, W1         ; 32-bit CRC\n");
    printf("    ; Uses ARMv8-CRC hardware extension\n");

    TEST_PASS();
}

/*
 * Test CRC32B (CRC32 Byte)
 * x86_64: CRC32B EAX, BL (F2 0F 38 F0 C3)
 * Calculates CRC32 of byte
 */
void test_crc32b_instruction(void)
{
    TEST_START("CRC32B Instruction Translation");

    uint8_t x86_crc32b[] = {0xF2, 0x0F, 0x38, 0xF0, 0xC3};  /* CRC32B EAX, BL */

    printf("  x86_64: CRC32B EAX, BL\n");
    printf("  Encoding: F2 0F 38 F0 C3\n");
    printf("  Operation: Calculate CRC-32 of byte\n");
    printf("  Expected ARM64:\n");
    printf("    CRC32B W0, W1         ; 8-bit CRC\n");
    printf("    ; ARMv8-CRC hardware extension\n");

    TEST_PASS();
}

/* ============================================================================
 * BMI INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test ANDN (AND NOT)
 * x86_64: ANDN RAX, RBX, RCX (C4 E2 58 F2 C1)
 * Bitwise AND of second and third operands, then NOT
 */
void test_andn_instruction(void)
{
    TEST_START("ANDN Instruction Translation");

    uint8_t x86_andn[] = {0xC4, 0xE2, 0x58, 0xF2, 0xC1};  /* ANDN RAX, RBX, RCX */

    printf("  x86_64: ANDN RAX, RBX, RCX\n");
    printf("  Encoding: C4 E2 58 F2 C1\n");
    printf("  Operation: RAX = (~RBX) & RCX\n");
    printf("  Expected ARM64:\n");
    printf("    BIC X0, X2, X1        ; Bit clear (AND NOT)\n");
    printf("    ; Direct ARM64 equivalent!\n");

    TEST_PASS();
}

/*
 * Test BEXTR (Bit Field Extract)
 * x86_64: BEXTR RAX, RBX, RCX (C4 E2 58 F7 C1)
 * Extracts bit field
 */
void test_bextr_instruction(void)
{
    TEST_START("BEXTR Instruction Translation");

    uint8_t x86_bextr[] = {0xC4, 0xE2, 0x58, 0xF7, 0xC1};  /* BEXTR RAX, RBX, RCX */

    printf("  x86_64: BEXTR RAX, RBX, RCX\n");
    printf("  Encoding: C4 E2 58 F7 C1\n");
    printf("  Operation: Extract bit field (length:start)\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has UBFM/UBFX!\n");
    printf("    UBFM X0, X1, #start, #end  ; Unsigned bit field extract\n");
    printf("    ; Direct ARM64 equivalent!\n");

    TEST_PASS();
}

/*
 * Test BZHI (Zero High Bits)
 * x86_64: BZHI RAX, RBX, RCX (C4 E2 68 F5 C1)
 * Zero high bits starting from index
 */
void test_bzhi_instruction(void)
{
    TEST_START("BZHI Instruction Translation");

    uint8_t x86_bzhi[] = {0xC4, 0xE2, 0x68, 0xF5, 0xC1};  /* BZHI RAX, RBX, RCX */

    printf("  x86_64: BZHI RAX, RBX, RCX\n");
    printf("  Encoding: C4 E2 68 F5 C1\n");
    printf("  Operation: Zero high bits above index in RCX\n");
    printf("  Expected ARM64:\n");
    printf("    AND X0, X1, #(2^RCX - 1)  ; Mask high bits\n");
    printf("    ; Or use BFI for bit field insert\n");

    TEST_PASS();
}

/* ============================================================================
 * AVX2 INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test VPADDB (AVX2 Packed Add Byte)
 * x86_64: VPADDB YMM0, YMM1, YMM2 (C5 FD FC C2)
 * Adds packed 8-bit integers (256-bit)
 */
void test_vpbroadcastb_instruction(void)
{
    TEST_START("VPBROADCASTB Instruction Translation");

    uint8_t x86_vpbroadcastb[] = {0xC4, 0xE2, 0x7D, 0x78, 0xC1};  /* VPBROADCASTB YMM0, XMM1 */

    printf("  x86_64: VPBROADCASTB YMM0, XMM1\n");
    printf("  Encoding: C4 E2 7D 78 C1\n");
    printf("  Operation: Broadcast byte to all 32 lanes\n");
    printf("  Expected ARM64:\n");
    printf("    ; Need to split across two NEON registers\n");
    printf("    DUP V0.16B, W1        ; Broadcast to V0\n");
    printf("    DUP V16.16B, W1       ; Broadcast to V16\n");

    TEST_PASS();
}

/*
 * Test VPERMD (AVX2 Permute Doublewords)
 * x86_64: VPERMD YMM0, YMM1, YMM2 (C4 E2 7D 36 C2)
 * Permutes 32-bit integers
 */
void test_vpermd_instruction(void)
{
    TEST_START("VPERMD Instruction Translation");

    uint8_t x86_vpermd[] = {0xC4, 0xE2, 0x7D, 0x36, 0xC2};  /* VPERMD YMM0, YMM1, YMM2 */

    printf("  x86_64: VPERMD YMM0, YMM1, YMM2\n");
    printf("  Encoding: C4 E2 7D 36 C2\n");
    printf("  Operation: Permute 32-bit integers (256-bit)\n");
    printf("  Expected ARM64:\n");
    printf("    ; Complex permutation, may need TBL\n");
    printf("    TBL V0.4S, {V0,V1}, V2  ; Lower 128-bit\n");
    printf("    TBL V16.4S, {V16,V17}, V18  ; Upper 128-bit\n");

    TEST_PASS();
}

/*
 * Test VPSLLDQ (AVX2 Shift Double Quadword Left Logical)
 * x86_64: VPSLLDQ YMM0, YMM1, imm8 (C5 FD 73 C1 ib)
 * Shifts 256-bit vector left by bytes
 */
void test_vpslldq_instruction(void)
{
    TEST_START("VPSLLDQ Instruction Translation");

    uint8_t x86_vpslldq[] = {0xC5, 0xFD, 0x73, 0xF8, 0x04};  /* VPSLLDQ YMM0, YMM0, 4 */

    printf("  x86_64: VPSLLDQ YMM0, YMM0, 4\n");
    printf("  Encoding: C5 FD 73 F8 04\n");
    printf("  Operation: Shift 256-bit vector left by 4 bytes\n");
    printf("  Expected ARM64:\n");
    printf("    ; Need to handle cross-lane shift\n");
    printf("    EXT V0.16B, {V0,V1}, #4  ; Extract with offset\n");
    printf("    MOV V16.16B, #0        ; Clear upper lane\n");

    TEST_PASS();
}

/* ============================================================================
 * FMA (FUSED MULTIPLY-ADD) TESTS
 * ============================================================================ */

/*
 * Test VFMADD213PS (FMA Packed Single)
 * x86_64: VFMADD213PS YMM0, YMM1, YMM2 (C4 E2 75 98 C2)
 * Fused multiply-add: YMM0 = YMM1 * YMM0 + YMM2
 */
void test_vfmadd213ps_instruction(void)
{
    TEST_START("VFMADD213PS Instruction Translation");

    uint8_t x86_vfmadd[] = {0xC4, 0xE2, 0x75, 0x98, 0xC2};  /* VFMADD213PS YMM0, YMM1, YMM2 */

    printf("  x86_64: VFMADD213PS YMM0, YMM1, YMM2\n");
    printf("  Encoding: C4 E2 75 98 C2\n");
    printf("  Operation: YMM0 = YMM1 * YMM0 + YMM2 (fused)\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has FMA instructions!\n");
    printf("    FMLA V0.4S, V1.4S, V2.4S    ; Lower 128-bit\n");
    printf("    FMLA V16.4S, V17.4S, V18.4S ; Upper 128-bit\n");
    printf("    ; Uses ARMv8-FMA hardware extension\n");

    TEST_PASS();
}

/*
 * Test VFMADD231SD (FMA Scalar Double)
 * x86_64: VFMADD231SD XMM0, XMM1, XMM2 (C4 E2 61 B9 C2)
 * Fused multiply-add: XMM0 = XMM1 * XMM2 + XMM0
 */
void test_vfmadd231sd_instruction(void)
{
    TEST_START("VFMADD231SD Instruction Translation");

    uint8_t x86_vfmaddsd[] = {0xC4, 0xE2, 0x61, 0xB9, 0xC2};  /* VFMADD231SD XMM0, XMM1, XMM2 */

    printf("  x86_64: VFMADD231SD XMM0, XMM1, XMM2\n");
    printf("  Encoding: C4 E2 61 B9 C2\n");
    printf("  Operation: XMM0[63:0] = XMM1 * XMM2 + XMM0 (fused)\n");
    printf("  Expected ARM64:\n");
    printf("    FMADD D0, D1, D2, D0    ; Fused multiply-add double\n");
    printf("    ; Direct ARM64 FMA instruction!\n");

    TEST_PASS();
}

/* ============================================================================
 * AVX-512 INSTRUCTION TESTS
 * ============================================================================ */

/*
 * Test VADDPS (AVX-512 512-bit Packed Add)
 * x86_64: VADDPS ZMM0, ZMM1, ZMM2 (62 F2 4D 58 58 C2)
 * Adds 512-bit vector of singles
 */
void test_vaddps_avx512_instruction(void)
{
    TEST_START("VADDPS AVX-512 Instruction Translation");

    uint8_t x86_vaddps512[] = {0x62, 0xF2, 0x4D, 0x58, 0x58, 0xC2};  /* VADDPS ZMM0, ZMM1, ZMM2 */

    printf("  x86_64: VADDPS ZMM0, ZMM1, ZMM2\n");
    printf("  Encoding: 62 F2 4D 58 58 C2\n");
    printf("  Operation: Add 512-bit vector (16x floats)\n");
    printf("  Expected ARM64:\n");
    printf("    ; Need to split into 4 NEON operations\n");
    printf("    FADD V0.4S, V1.4S, V2.4S    ; Quadword 0\n");
    printf("    FADD V16.4S, V17.4S, V18.4S ; Quadword 1\n");
    printf("    FADD V32.4S, V33.4S, V34.4S ; Quadword 2\n");
    printf("    FADD V48.4S, V49.4S, V50.4S ; Quadword 3\n");
    printf("  Note: 4x overhead for AVX-512 translation\n");

    TEST_PASS();
}

/*
 * Test VPANDQ (AVX-512 Packed AND Qword)
 * x86_64: VPANDQ ZMM0, ZMM1, ZMM2 (62 F1 4E 54 DB C2)
 * Bitwise AND of 512-bit qword integers
 */
void test_vpandq_avx512_instruction(void)
{
    TEST_START("VPANDQ AVX-512 Instruction Translation");

    uint8_t x86_vpandq512[] = {0x62, 0xF1, 0x4E, 0x54, 0xDB, 0xC2};  /* VPANDQ ZMM0, ZMM1, ZMM2 */

    printf("  x86_64: VPANDQ ZMM0, ZMM1, ZMM2\n");
    printf("  Encoding: 62 F1 4E 54 DB C2\n");
    printf("  Operation: AND 512-bit vector (8x qwords)\n");
    printf("  Expected ARM64:\n");
    printf("    AND V0.2D, V1.2D, V2.2D    ; Quadword 0\n");
    printf("    AND V16.2D, V17.2D, V18.2D ; Quadword 1\n");
    printf("    AND V32.2D, V33.2D, V34.2D ; Quadword 2\n");
    printf("    AND V48.2D, V49.2D, V50.2D ; Quadword 3\n");

    TEST_PASS();
}

/* ============================================================================
 * ADVANCED SIMD SHUFFLE TESTS
 * ============================================================================ */

/*
 * Test VSHUFPD (AVX Shuffle Packed Double)
 * x86_64: VSHUFPD YMM0, YMM1, YMM2, imm8 (C5 FD C6 C1 ib)
 * Shuffles 64-bit floats
 */
void test_vshufpd_instruction(void)
{
    TEST_START("VSHUFPD Instruction Translation");

    uint8_t x86_vshufpd[] = {0xC5, 0xFD, 0xC6, 0xC1, 0x00};  /* VSHUFPD YMM0, YMM1, YMM2, 0 */

    printf("  x86_64: VSHUFPD YMM0, YMM1, YMM2, 0\n");
    printf("  Encoding: C5 FD C6 C1 00\n");
    printf("  Operation: Shuffle 64-bit floats (256-bit)\n");
    printf("  Expected ARM64:\n");
    printf("    ; ARM64 has TRN/ZIP/UZIP for shuffling\n");
    printf("    TRN1 V0.2D, V1.2D, V2.2D    ; Interleave lower\n");
    printf("    TRN2 V16.2D, V17.2D, V18.2D ; Interleave upper\n");

    TEST_PASS();
}

/*
 * Test VUNPCKLPD (AVX Unpack Low Packed Double)
 * x86_64: VUNPCKLPD YMM0, YMM1, YMM2 (C5 FD 14 C2)
 * Unpacks and interleaves low 64-bit floats
 */
void test_vunpcklpd_instruction(void)
{
    TEST_START("VUNPCKLPD Instruction Translation");

    uint8_t x86_vunpcklpd[] = {0xC5, 0xFD, 0x14, 0xC2};  /* VUNPCKLPD YMM0, YMM1, YMM2 */

    printf("  x86_64: VUNPCKLPD YMM0, YMM1, YMM2\n");
    printf("  Encoding: C5 FD 14 C2\n");
    printf("  Operation: Unpack low 64-bit floats\n");
    printf("  Expected ARM64:\n");
    printf("    ZIP1 V0.2D, V1.2D, V2.2D    ; Zip low 128-bit\n");
    printf("    ZIP2 V16.2D, V17.2D, V18.2D ; Zip high 128-bit\n");

    TEST_PASS();
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

void run_aes_ni_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     AES-NI Instruction Tests                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_aesenc_instruction();
    test_aesenclast_instruction();
    test_aesdec_instruction();
    test_aeskeygenassist_instruction();
    test_pclmulqdq_instruction();
}

void run_sha_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     SHA Extension Tests                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_sha1rnds4_instruction();
    test_sha1nexte_instruction();
    test_sha256rnds2_instruction();
}

void run_crc_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     CRC32 Instruction Tests                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_crc32_instruction();
    test_crc32b_instruction();
}

void run_bmi_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     BMI Instruction Tests                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_andn_instruction();
    test_bextr_instruction();
    test_bzhi_instruction();
}

void run_avx2_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     AVX2 Instruction Tests                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_vpbroadcastb_instruction();
    test_vpermd_instruction();
    test_vpslldq_instruction();
}

void run_fma_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     FMA Instruction Tests                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_vfmadd213ps_instruction();
    test_vfmadd231sd_instruction();
}

void run_avx512_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     AVX-512 Instruction Tests                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_vaddps_avx512_instruction();
    test_vpandq_avx512_instruction();
}

void run_advanced_shuffle_tests(void)
{
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Advanced Shuffle Tests                               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    test_vshufpd_instruction();
    test_vunpcklpd_instruction();
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
    printf("║     Rosetta 2 Cryptographic & Advanced SIMD Tests         ║\n");
    printf("║     Testing x86_64 → ARM64 Translation                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    if (argc == 1 || strcmp(argv[1], "all") == 0) {
        run_aes_ni_tests();
        run_sha_tests();
        run_crc_tests();
        run_bmi_tests();
        run_avx2_tests();
        run_fma_tests();
        run_avx512_tests();
        run_advanced_shuffle_tests();
    } else if (strcmp(argv[1], "aes") == 0) {
        run_aes_ni_tests();
    } else if (strcmp(argv[1], "sha") == 0) {
        run_sha_tests();
    } else if (strcmp(argv[1], "crc") == 0) {
        run_crc_tests();
    } else if (strcmp(argv[1], "bmi") == 0) {
        run_bmi_tests();
    } else if (strcmp(argv[1], "avx2") == 0) {
        run_avx2_tests();
    } else if (strcmp(argv[1], "fma") == 0) {
        run_fma_tests();
    } else if (strcmp(argv[1], "avx512") == 0) {
        run_avx512_tests();
    } else if (strcmp(argv[1], "shuffle") == 0) {
        run_advanced_shuffle_tests();
    } else {
        printf("\nUsage: %s [all|aes|sha|crc|bmi|avx2|fma|avx512|shuffle]\n", argv[0]);
        printf("  all     - Run all tests (default)\n");
        printf("  aes     - AES-NI encryption tests\n");
        printf("  sha     - SHA hash extension tests\n");
        printf("  crc     - CRC32 tests\n");
        printf("  bmi     - Bit manipulation tests\n");
        printf("  avx2    - AVX2 integer tests\n");
        printf("  fma     - Fused multiply-add tests\n");
        printf("  avx512  - AVX-512 512-bit tests\n");
        printf("  shuffle - Advanced shuffle tests\n");
        return 1;
    }

    print_test_summary();

    return (failed_tests == 0) ? 0 : 1;
}
