# Rosetta 2 Cryptographic and Advanced SIMD Translation Guide

## Overview

This document describes how the Rosetta 2 binary translator handles x86_64 cryptographic extensions and advanced SIMD instructions, translating them to ARM64 equivalents.

## Table of Contents

1. [AES-NI Instructions](#aes-ni-instructions)
2. [SHA Extensions](#sha-extensions)
3. [CRC32 Instructions](#crc32-instructions)
4. [BMI Instructions](#bmi-instructions)
5. [AVX2 Instructions](#avx2-instructions)
6. [FMA Instructions](#fma-instructions)
7. [AVX-512 Instructions](#avx-512-instructions)
8. [Advanced Shuffle Operations](#advanced-shuffle-operations)
9. [Performance Considerations](#performance-considerations)
10. [ARM64 Hardware Extensions](#arm64-hardware-extensions)

---

## AES-NI Instructions

### Overview

AES-NI (Advanced Encryption Standard New Instructions) are x86_64 extensions that accelerate AES encryption and decryption. These instructions are critical for:

- Full disk encryption (FileVault, BitLocker)
- SSL/TLS acceleration
- VPN performance
- Database encryption

### Instruction Mapping

| x86_64 Instruction | Encoding | ARM64 Translation | Notes |
|-------------------|----------|-------------------|-------|
| AESENC XMM0, XMM1 | 66 0F 38 DC C1 | AESE V0.16B, V1.16B | Direct hardware equivalent ✓ |
| AESENCLAST XMM0, XMM1 | 66 0F 38 DD C1 | AESE + EOR | Final round variant |
| AESDEC XMM0, XMM1 | 66 0F 38 DE C1 | AESD V0.16B, V1.16B | Direct hardware equivalent ✓ |
| AESDECLAST XMM0, XMM1 | 66 0F 38 DF C1 | AESD + EOR | Final round variant |
| AESKEYGENASSIST | 66 0F 3A DF /r | AESMC V0.16B, V1.16B | Key generation assist |
| PCLMULQDQ XMM0, XMM1, imm | 66 0F 3A 44 /r ib | PMULL V0.1Q, V1.1D, V2.1D | Carry-less multiply (GCM) |

### Translation Strategy

**Good News**: ARM64 has hardware AES extensions!

- **AES Encryption**: Direct 1:1 mapping using `AESE` instruction
- **AES Decryption**: Direct 1:1 mapping using `AESD` instruction
- **Carry-less Multiply**: Direct 1:1 mapping using `PMULL` instruction
- **Performance**: Same performance as native x86_64 AES-NI

### Example: AESENC Translation

**x86_64:**
```asm
AESENC XMM0, XMM1    ; One round of AES encryption
; Encoding: 66 0F 38 DC C1
```

**ARM64 Translation:**
```asm
AESE V0.16B, V1.16B  ; One round of AES encryption
; Encoding: ARMv8-AES hardware instruction
; Same latency, same throughput
```

### Example: PCLMULQDQ Translation

**x86_64:**
```asm
PCLMULQDQ XMM0, XMM1, 0    ; Carry-less multiply
; Used for GCM mode
```

**ARM64 Translation:**
```asm
PMULL V0.1Q, V1.1D, V2.1D  ; Polynomial multiply long
; ARMv8-PMULL hardware extension
; Same operation, different name
```

---

## SHA Extensions

### Overview

SHA extensions accelerate Secure Hash Algorithm computations used for:

- Digital signatures
- Integrity checking
- Cryptographic hashing
- Blockchain operations

### Instruction Mapping

| x86_64 Instruction | Encoding | ARM64 Translation | Notes |
|-------------------|----------|-------------------|-------|
| SHA1RNDS4 XMM0, XMM1, imm | 0F 3A CC /r ib | SHA1C Q0, Q1, V2.4S | SHA-1 compression |
| SHA1NEXTE XMM0, XMM1 | 0F 38 C8 /r | SHA1H S0, S1 | SHA-1 schedule |
| SHA1MSG1 XMM0, XMM1 | 0F 38 C9 /r | SHA1SU0 V0.4S, V1.4S | SHA-1 preprocessing |
| SHA256RNDS2 XMM0, XMM1 | 0F 38 CB /r | SHA256H Q0, Q1, V2.4Q | SHA-256 compression |
| SHA256MSG1 XMM0, XMM1 | 0F 38 CC /r | SHA256SU0 V0.4S, V1.4S | SHA-256 preprocessing |

### Translation Strategy

**Good News**: ARM64 has hardware SHA extensions!

- **SHA-1**: Direct mapping using `SHA1C`, `SHA1H`, `SHA1SU0` instructions
- **SHA-256**: Direct mapping using `SHA256H`, `SHA256H2`, `SHA256SU0` instructions
- **Performance**: Hardware-accelerated SHA on both architectures

### Example: SHA256RNDS2 Translation

**x86_64:**
```asm
SHA256RNDS2 XMM0, XMM1    ; Two rounds of SHA-256
; Encoding: 0F 38 CB C1
```

**ARM64 Translation:**
```asm
SHA256H Q0, Q1, V2.4Q    ; SHA-256 hash update (first round)
SHA256H2 Q0, Q1, V2.4Q   ; SHA-256 hash update (second round)
; ARMv8-SHA hardware extension
```

---

## CRC32 Instructions

### Overview

CRC32 instructions accelerate Cyclic Redundancy Check computations used for:

- Data integrity verification
- Network protocols (Ethernet, ZIP)
- Storage systems
- Error detection

### Instruction Mapping

| x86_64 Instruction | Encoding | ARM64 Translation | Notes |
|-------------------|----------|-------------------|-------|
| CRC32 EAX, EBX | F2 0F 38 F1 /r | CRC32W W0, W1 | 32-bit CRC |
| CRC32B EAX, BL | F2 0F 38 F0 /r | CRC32B W0, W1 | 8-bit CRC |
| CRC32D EAX, EBX | F2 0F 38 F1 /r | CRC32X W0, W1 | 64-bit CRC (CRC64) |
| CRC32Q RAX, RBX | 48 F2 0F 38 F1 /r | CRC32X X0, X1 | 64-bit CRC |

### Translation Strategy

**Good News**: ARM64 has hardware CRC32 extensions!

- **CRC32**: Direct 1:1 mapping using `CRC32B`, `CRC32W`, `CRC32X` instructions
- **Performance**: Hardware-accelerated CRC32 on both architectures
- **Polynomial**: Both use ISO 3309 polynomial (same as Ethernet)

### Example: CRC32 Translation

**x86_64:**
```asm
CRC32 EAX, EBX    ; Calculate CRC-32
; Encoding: F2 0F 38 F1 C3
; Polynomial: ISO 3309
```

**ARM64 Translation:**
```asm
CRC32W W0, W1    ; Calculate CRC-32
; ARMv8-CRC hardware extension
; Same polynomial, same result
```

---

## BMI Instructions

### Overview

BMI (Bit Manipulation Instructions) provide efficient bit operations for:

- Algorithm optimization
- Data compression
- Graphics processing
- Cryptographic operations

### Instruction Mapping

| x86_64 Instruction | Encoding | ARM64 Translation | Notes |
|-------------------|----------|-------------------|-------|
| ANDN RAX, RBX, RCX | C4 E2 58 F2 /r | BIC X0, X2, X1 | AND NOT (direct equiv) |
| BEXTR RAX, RBX, RCX | C4 E2 58 F7 /r | UBFM X0, X1, #s, #e | Bit field extract |
| BZHI RAX, RBX, RCX | C4 E2 68 F5 /r | AND + mask | Zero high bits |
| BLSI RAX, RBX | C4 E2 78 F3 /r | AND X0, X1, -X1 | Extract lowest set bit |
| BLSMSK RAX, RBX | C4 E2 78 F2 /r | EOR X0, X1, (X1-1) | Bit mask |
| BLSR RAX, RBX | C4 E2 78 F1 /r | AND X0, X1, (X1-1) | Reset lowest set bit |

### Translation Strategy

**Excellent News**: Most BMI instructions have direct ARM64 equivalents!

- **ANDN**: Direct 1:1 mapping using `BIC` (bit clear)
- **BEXTR**: Direct mapping using `UBFM` (unsigned bit field extract)
- **BZHI**: Mask operation using `AND`
- **BLSI/BLSMSK/BLSR**: Simple arithmetic operations

### Example: ANDN Translation

**x86_64:**
```asm
ANDN RAX, RBX, RCX    ; RAX = (~RBX) & RCX
; Encoding: C4 E2 58 F2 C1
```

**ARM64 Translation:**
```asm
BIC X0, X2, X1    ; X0 = X2 & ~X1
; Direct ARM64 equivalent!
; Same operation, different syntax
```

### Example: BEXTR Translation

**x86_64:**
```asm
BEXTR RAX, RBX, RCX    ; Extract bit field
; RCX[7:0] = length, RCX[15:8] = start
```

**ARM64 Translation:**
```asm
UBFM X0, X1, #start, #end    ; Unsigned bit field extract
; Direct ARM64 equivalent!
```

---

## AVX2 Instructions

### Overview

AVX2 extends AVX with 256-bit integer operations:

- Image processing
- Video encoding/decoding
- Scientific computing
- Database operations

### Instruction Mapping

| x86_64 Instruction | ARM64 Translation | Overhead |
|-------------------|-------------------|----------|
| VPBROADCASTB YMM0, XMM1 | DUP V0.16B, W1 + DUP V16.16B, W1 | 2x |
| VPERMD YMM0, YMM1, YMM2 | TBL V0.4S + TBL V16.4S | 2x |
| VPSLLDQ YMM0, YMM1, imm | EXT V0.16B + MOV V16.16B, #0 | 2x |
| VPADDB YMM0, YMM1, YMM2 | ADD V0.16B + ADD V16.16B | 2x |
| VPADDD YMM0, YMM1, YMM2 | ADD V0.4S + ADD V16.4S | 2x |

### Translation Strategy

**Challenge**: ARM64 NEON is 128-bit, AVX2 is 256-bit

**Solution**: Split AVX2 operations into two 128-bit NEON operations

- **VPBROADCASTB**: Broadcast byte to 32 lanes (split across 2 NEON registers)
- **VPERMD**: Complex permutation using `TBL` instruction
- **VPSLLDQ**: Byte shift with cross-lane handling
- **VPADDD**: Simple integer addition (split across 2 NEON registers)

### Example: VPBROADCASTB Translation

**x86_64:**
```asm
VPBROADCASTB YMM0, XMM1    ; Broadcast byte to all 32 lanes
; YMM0[255:0] = XMM1[7:0] replicated 32 times
```

**ARM64 Translation:**
```asm
DUP V0.16B, W1        ; Broadcast to lower 128-bit
DUP V16.16B, W1       ; Broadcast to upper 128-bit
; Result: V0+V16 = YMM0 equivalent
```

### Example: VPERMD Translation

**x86_64:**
```asm
VPERMD YMM0, YMM1, YMM2    ; Permute 32-bit integers
; YMM0[i] = YMM2[YMM1[i]]
```

**ARM64 Translation:**
```asm
TBL V0.4S, {V0,V1}, V2     ; Permute lower 128-bit
TBL V16.4S, {V16,V17}, V18 ; Permute upper 128-bit
; Uses NEON table lookup instruction
```

---

## FMA Instructions

### Overview

FMA (Fused Multiply-Add) combines multiplication and addition in one operation:

- Higher precision (rounds once instead of twice)
- Better performance (single instruction)
- Critical for scientific computing
- Used in machine learning

### Instruction Mapping

| x86_64 Instruction | ARM64 Translation | Notes |
|-------------------|-------------------|-------|
| VFMADD213PS YMM0, YMM1, YMM2 | FMLA V0.4S + FMLA V16.4S | Fused multiply-add |
| VFMADD231SD XMM0, XMM1, XMM2 | FMADD D0, D1, D2, D0 | Direct equiv! |
| VFMSUB213PS YMM0, YMM1, YMM2 | FMLS V0.4S + FMLS V16.4S | Fused multiply-sub |
| VFNMSUB213PS YMM0, YMM1, YMM2 | FNMLA V0.4S + FNMLA V16.4S | Fused neg multiply-add |

### Translation Strategy

**Excellent News**: ARM64 has FMA instructions!

- **Scalar FMA**: Direct 1:1 mapping using `FMADD`, `FMSUB`, `FNMLADD`, `FNMLSUB`
- **Vector FMA**: 256-bit requires 2 NEON operations
- **Performance**: Same latency/throughput on both architectures

### Example: VFMADD231SD Translation

**x86_64:**
```asm
VFMADD231SD XMM0, XMM1, XMM2    ; XMM0 = XMM1 * XMM2 + XMM0
; Fused multiply-add (rounds once)
```

**ARM64 Translation:**
```asm
FMADD D0, D1, D2, D0    ; D0 = D1 * D2 + D0
; Direct ARM64 FMA instruction!
; Same operation, different syntax
```

### Example: VFMADD213PS Translation

**x86_64:**
```asm
VFMADD213PS YMM0, YMM1, YMM2    ; YMM0 = YMM1 * YMM0 + YMM2
; Fused multiply-add of 8 floats
```

**ARM64 Translation:**
```asm
FMLA V0.4S, V1.4S, V2.4S    ; Lower 128-bit
FMLA V16.4S, V17.4S, V18.4S ; Upper 128-bit
; Split into 2 NEON operations
```

---

## AVX-512 Instructions

### Overview

AVX-512 extends SIMD to 512-bit vectors:

- High-performance computing
- AI/ML workloads
- Video processing
- Scientific simulations

### Instruction Mapping

| x86_64 Instruction | ARM64 Translation | Overhead |
|-------------------|-------------------|----------|
| VADDPS ZMM0, ZMM1, ZMM2 | FADD V0.4S + V16.4S + V32.4S + V48.4S | 4x |
| VPANDQ ZMM0, ZMM1, ZMM2 | AND V0.2D + V16.2D + V32.2D + V48.2D | 4x |
| VORPS ZMM0, ZMM1, ZMM2 | ORR V0.16B + V16.16B + V32.16B + V48.16B | 4x |
| VXORPS ZMM0, ZMM1, ZMM2 | EOR V0.16B + V16.16B + V32.16B + V48.16B | 4x |

### Translation Strategy

**Major Challenge**: ARM64 NEON is 128-bit, AVX-512 is 512-bit

**Solution**: Split AVX-512 operations into FOUR 128-bit NEON operations

- **VADDPS**: 512-bit add → 4x 128-bit adds (4x overhead)
- **VPANDQ**: 512-bit AND → 4x 128-bit ANDs (4x overhead)
- **Performance**: Significant overhead, but correct

### Example: VADDPS AVX-512 Translation

**x86_64:**
```asm
VADDPS ZMM0, ZMM1, ZMM2    ; Add 512-bit vector
; ZMM0[511:0] = ZMM1[511:0] + ZMM2[511:0]
; 16 single-precision floats
```

**ARM64 Translation:**
```asm
FADD V0.4S, V1.4S, V2.4S    ; Quadword 0 (floats 0-3)
FADD V16.4S, V17.4S, V18.4S ; Quadword 1 (floats 4-7)
FADD V32.4S, V33.4S, V34.4S ; Quadword 2 (floats 8-11)
FADD V48.4S, V49.4S, V50.4S ; Quadword 3 (floats 12-15)
; 4x overhead for AVX-512 translation
```

---

## Advanced Shuffle Operations

### Overview

Shuffle operations rearrange vector elements:

- Data reorganization
- Matrix transposition
- Color space conversion
- Compression algorithms

### Instruction Mapping

| x86_64 Instruction | ARM64 Translation | Notes |
|-------------------|-------------------|-------|
| VSHUFPD YMM0, YMM1, YMM2, imm | TRN1 V0.2D + TRN2 V16.2D | Shuffle doubles |
| VUNPCKLPD YMM0, YMM1, YMM2 | ZIP1 V0.2D + ZIP2 V16.2D | Unpack low doubles |
| VUNPCKHPD YMM0, YMM1, YMM2 | ZIP1 V0.2D + ZIP2 V16.2D | Unpack high doubles |
| VSHUFPS YMM0, YMM1, YMM2, imm | TBL + complex | Shuffle floats |

### Translation Strategy

**Good News**: ARM64 has powerful shuffle instructions

- **VSHUFPD**: Use `TRN1`/`TRN2` (transpose) instructions
- **VUNPCKL/HPD**: Use `ZIP1`/`ZIP2` (zip) instructions
- **VSHUFPS**: Complex permutation using `TBL` (table lookup)

### Example: VUNPCKLPD Translation

**x86_64:**
```asm
VUNPCKLPD YMM0, YMM1, YMM2    ; Unpack low doubles
; YMM0[127:0] = YMM1[63:0], YMM2[63:0]
; YMM0[255:128] = YMM1[191:128], YMM2[191:128]
```

**ARM64 Translation:**
```asm
ZIP1 V0.2D, V1.2D, V2.2D    ; Zip low 128-bit
ZIP2 V16.2D, V17.2D, V18.2D ; Zip high 128-bit
; ARM64 has dedicated ZIP instructions!
```

---

## Performance Considerations

### Translation Overhead Summary

| Instruction Class | Overhead | Notes |
|-------------------|----------|-------|
| AES-NI | 1x | Direct hardware equivalent ✓ |
| SHA | 1x | Direct hardware equivalent ✓ |
| CRC32 | 1x | Direct hardware equivalent ✓ |
| BMI | 1x | Direct ARM64 equivalents ✓ |
| FMA (scalar) | 1x | Direct hardware equivalent ✓ |
| SSE (128-bit) | 1x | Direct 1:1 mapping |
| AVX (256-bit) | 2x | Split into 2 NEON ops |
| AVX2 (256-bit) | 2x | Split into 2 NEON ops |
| FMA (vector) | 2x | Split into 2 NEON ops |
| AVX-512 (512-bit) | 4x | Split into 4 NEON ops |

### Hardware Extension Availability

ARM64 hardware extensions (varies by CPU):

- **ARMv8-AES**: AES encryption/decryption ✓
- **ARMv8-SHA**: SHA-1/SHA-256 hashing ✓
- **ARMv8-CRC**: CRC32 calculation ✓
- **ARMv8-PMULL**: Carry-less multiply ✓
- **ARMv8-FMA**: Fused multiply-add ✓

**Note**: Not all ARM64 CPUs have all extensions. Fall back to software emulation if hardware not available.

### Optimization Strategies

1. **Use 128-bit operations when possible**
   - Prefer SSE over AVX when 256-bit not needed
   - Reduces instruction count by 2x for AVX

2. **Leverage hardware extensions**
   - Use ARMv8-AES for encryption (same performance as x86_64)
   - Use ARMv8-SHA for hashing (same performance as x86_64)
   - Use ARMv8-CRC for CRC32 (same performance as x86_64)

3. **Batch similar operations**
   - Process multiple SIMD operations together
   - Improves instruction throughput

4. **Cache translated blocks**
   - Reuse translations for repeated code
   - 95-99% hit rate in steady state

---

## ARM64 Hardware Extensions

### Checking Extension Availability

```c
// Check if CPU has AES support
bool has_aes() {
#ifdef __linux__
    uint64_t features;
    __asm__("mrs %0, ID_AA64ISAR0_EL1" : "=r"(features));
    return (features & 0x1) != 0;  // Check AES bit
#else
    return false;  // macOS: assume support or emulate
#endif
}

// Check if CPU has SHA support
bool has_sha() {
#ifdef __linux__
    uint64_t features;
    __asm__("mrs %0, ID_AA64ISAR0_EL1" : "=r"(features));
    return (features & 0x2) != 0;  // Check SHA bits
#else
    return false;
#endif
}
```

### Feature Detection

ARM64 system registers:

- **ID_AA64ISAR0_EL1**: Instruction set attributes
  - Bits [3:0]: AES support
  - Bits [7:4]: SHA1/SHA256 support
  - Bits [11:8]: CRC32 support
  - Bits [15:12]: SIMD capability

### Software Fallback

If hardware extension not available:

```c
void aes_encrypt_emulation(uint8_t *state, const uint8_t *key) {
    if (has_aes()) {
        __asm__("AESE %0.16B, %1.16B" : "=w"(state) : "w"(key));
    } else {
        aes_encrypt_software(state, key);  // Software fallback
    }
}
```

---

## Testing

### Run Crypto/Advanced SIMD Tests

```bash
# Test all cryptographic and advanced SIMD instructions
./test_crypto_advanced all

# Test specific categories
./test_crypto_advanced aes     # AES-NI encryption
./test_crypto_advanced sha     # SHA hash extensions
./test_crypto_advanced crc     # CRC32 instructions
./test_crypto_advanced bmi     # Bit manipulation
./test_crypto_advanced avx2    # AVX2 integer operations
./test_crypto_advanced fma     # Fused multiply-add
./test_crypto_advanced avx512  # AVX-512 512-bit operations
./test_crypto_advanced shuffle # Advanced shuffle operations
```

### Expected Results

All tests should pass with output showing:
- Instruction encodings
- Expected ARM64 translations
- Hardware extension mappings
- Performance overhead notes

---

## References

- [ARM Architecture Reference Manual](https://developer.arm.com/documentation/)
- [Intel 64 and IA-32 Architectures SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [ARM NEON Intrinsics Reference](https://developer.arm.com/architectures/intrinsics/)
- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/)

---

*Last updated: March 2026*
