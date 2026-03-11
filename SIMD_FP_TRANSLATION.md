# Rosetta 2 SIMD and Floating-Point Translation Guide

## Overview

This document describes how the Rosetta 2 binary translator handles x86_64 SIMD and floating-point instructions, translating them to ARM64 NEON equivalents.

## Table of Contents

1. [Floating-Point Translation (x87)](#floating-point-translation-x87)
2. [SSE Scalar Translation](#sse-scalar-translation)
3. [SSE2 Integer SIMD](#sse2-integer-simd)
4. [SSE3/SSSE3 Instructions](#sse3ssse3-instructions)
5. [SSE4.x Instructions](#sse4x-instructions)
6. [AVX (256-bit) Translation](#avx-256-bit-translation)
7. [ARM64 NEON Architecture](#arm64-neon-architecture)
8. [Register Mapping](#register-mapping)
9. [Translation Examples](#translation-examples)

---

## Floating-Point Translation (x87)

### x87 Stack vs ARM64 Register File

**x87 Architecture:**
- Stack-based register file (ST0-ST7)
- Instructions operate on stack top
- Complex state management

**ARM64 Architecture:**
- Flat register file (D0-D31 for doubles, S0-S31 for singles)
- Direct register access
- Simpler state management

### x87 Instruction Translation

| x87 Instruction | Encoding | ARM64 Translation | Notes |
|-----------------|----------|-------------------|-------|
| FADD ST(0), ST(i) | D8 C0+i | FADD D0, Di | Stack → register |
| FSUB ST(0), ST(i) | D8 E0+i | FSUB D0, Di | |
| FMUL ST(0), ST(i) | D8 C8+i | FMUL D0, Di | |
| FDIV ST(0), ST(i) | D8 F0+i | FDIV D0, Di | |
| FSTP mem | DD /0 | STR D0, [mem] | Store + pop |
| FLD mem | DD /0 | LDR D0, [mem] | Load |
| FCOMIP ST(i) | DA D0+i | FCMP D0, Di | Compare + pop |

### Translation Strategy

1. **Stack Unrolling**: Map x87 stack slots to ARM64 registers
   ```
   ST0 → D0, ST1 → D1, ST2 → D2, ..., ST7 → D7
   ```

2. **Stack Operations**: Track stack pointer implicitly
   - FLD: Increment stack pointer, load into register
   - FSTP: Store value, decrement stack pointer
   - FADD: Operate on ST0 and ST(i), leave result in ST0

3. **Precision Handling**
   - x87 supports 32-bit (single), 64-bit (double), and 80-bit (extended)
   - ARM64 natively supports 32-bit (single) and 64-bit (double)
   - Extended precision (80-bit) requires software emulation

---

## SSE Scalar Translation

### SSE Scalar Single-Precision (ADDSS, SUBSS, MULSS, DIVSS)

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| ADDSS XMM0, XMM1 | F3 0F 58 C1 | FADD S0, S1 |
| SUBSS XMM0, XMM1 | F3 0F 5C C1 | FSUB S0, S1 |
| MULSS XMM0, XMM1 | F3 0F 59 C1 | FMUL S0, S1 |
| DIVSS XMM0, XMM1 | F3 0F 5E C1 | FDIV S0, S1 |

**Register Mapping:**
- XMM0 → S0 (lower 32 bits of D0)
- XMM1 → S1 (lower 32 bits of D1)
- Use S registers for 32-bit float operations

### SSE Scalar Double-Precision (ADDSD, SUBSD, MULSD, DIVSD)

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| ADDSD XMM0, XMM1 | F2 0F 58 C1 | FADD D0, D1 |
| SUBSD XMM0, XMM1 | F2 0F 5C C1 | FSUB D0, D1 |
| MULSD XMM0, XMM1 | F2 0F 59 C1 | FMUL D0, D1 |
| DIVSD XMM0, XMM1 | F2 0F 5E C1 | FDIV D0, D1 |

**Register Mapping:**
- XMM0 → D0
- XMM1 → D1
- Use D registers for 64-bit float operations

### SSE Move Instructions

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| MOVSS XMM0, XMM1 | F3 0F 10 C1 | FMOV S0, S1 |
| MOVSD XMM0, XMM1 | F2 0F 10 C1 | FMOV D0, D1 |
| MOVSS XMM0, mem | F3 0F 10 /m | LDR S0, [mem] |
| MOVSD XMM0, mem | F2 0F 10 /m | LDR D0, [mem] |

---

## SSE2 Integer SIMD

### SSE2 Packed Integer Operations

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| MOVQ XMM0, XMM1 | 66 0F 6E C1 | FMOV D0, X1 (as int) |
| MOVD XMM0, reg | 66 0F 6E C0 | FMOV S0, W0 (32-bit) |
| PADDQ XMM0, XMM1 | 66 0F D4 C1 | ADD V0.2D, V1.2D |
| PSUBQ XMM0, XMM1 | 66 0F FB C1 | SUB V0.2D, V1.2D |
| PAND XMM0, XMM1 | 66 0F DB C1 | AND V0.16B, V1.16B |
| POR XMM0, XMM1 | 66 0F EB C1 | ORR V0.16B, V1.16B |
| PXOR XMM0, XMM1 | 66 0F EF C1 | EOR V0.16B, V1.16B |

**Element Sizes:**
- **8-bit (bytes)**: V0.16B (16 bytes)
- **16-bit (halfwords)**: V0.8H (8 halfwords)
- **32-bit (words)**: V0.4S (4 words)
- **64-bit (doublewords)**: V0.2D (2 doublewords)

---

## SSE3/SSSE3 Instructions

### SSE3 Instructions

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| ADDSUBPS XMM0, XMM1 | F2 0F D0 C1 | Complex sequence |
| HADDPS XMM0, XMM1 | F2 0F 7C C1 | Complex sequence |
| HSUBPS XMM0, XMM1 | F2 0F 7D C1 | Complex sequence |

### SSSE3 Instructions

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| PSHUFB XMM0, XMM1 | 66 0F 38 00 C1 | TBL V0.16B, {V0,V1}.16B |
| PABSB XMM0, XMM1 | 66 0F 38 1C C1 | ABS V0.16B, V1.16B |
| PSIGNB XMM0, XMM1 | 66 0F 38 08 C1 | Complex sequence |

**Translation Notes:**
- SSSE3 shuffle operations map to NEON TBL (table lookup)
- NEON has powerful permute instructions that can handle most SSE shuffle patterns
- Some operations require multiple NEON instructions to emulate

---

## SSE4.x Instructions

### SSE4.1 Instructions

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| PTEST XMM0, XMM1 | 66 0F 38 17 C1 | CMTST V0.16B, V0.16B, V1.16B |
| BLENDVPS XMM0, XMM1, mem | 66 0F 14 0F /r | Complex sequence |
| PMINSB XMM0, XMM1 | 66 0F 38 3C C1 | SMIN V0.16B, V1.16B |
| PMAXSB XMM0, XMM1 | 66 0F 38 3E C1 | SMAX V0.16B, V1.16B |

### SSE4.2 Instructions

| Instruction | Encoding | ARM64 Translation |
|-------------|----------|-------------------|
| CRC32 eax, reg | F2 0F 38 F1 C0 | CRC32X W0, W1 (ARMv8.0+) |
| PCMPESTRI | 66 0F 3A 61 /r | Complex (requires multiple instrs) |

---

## AVX (256-bit) Translation

### AVX to NEON Mapping

**Key Challenge:** ARM64 NEON is 128-bit, AVX is 256-bit

**Solution:** Split AVX operations into two 128-bit NEON operations

| AVX Instruction | Register Mapping | ARM64 Translation |
|----------------|-----------------|-------------------|
| VADDPS YMM0, YMM1, YMM2 | YMM0→V0+V16, YMM1→V1+V17 | ADD V0.4S, V1.4S, V2.4S<br>ADD V16.4S, V17.4S, V18.4S |
| VMULPS YMM0, YMM1, YMM2 | YMM0→V0+V16, YMM1→V1+V17 | FMUL V0.4S, V1.4S, V2.4S<br>FMUL V16.4S, V17.4S, V18.4S |
| VORPS YMM0, YMM1, YMM2 | YMM0→V0+V16, YMM1→V1+V17 | ORR V0.16B, V1.16B, V2.16B<br>ORR V16.16B, V17.16B, V18.16B |

**Register Mapping Table:**

| AVX Register | Low 128-bit | High 128-bit |
|-------------|---------------|---------------|
| YMM0 | V0 | V16 |
| YMM1 | V1 | V17 |
| YMM2 | V2 | V18 |
| YMM3 | V3 | V19 |
| ... | ... | ... |
| YMM15 | V15 | V31 |

**AVX-512 (ZMM) Mapping:**
- ZMM0 (512-bit) → V0 + V16 + V32 + V48 (4x128-bit)
- Requires 4 NEON operations per AVX-512 instruction
- Significant performance overhead

---

## ARM64 NEON Architecture

### NEON Register File

```
V0  (128-bit) = D0 (64-bit) + D1 (64-bit) = S0-S3 (32-bit) = 8x16-bit
V1  (128-bit) = D2 (64-bit) + D3 (64-bit) = S4-S7 (32-bit) = 8x16-bit
...
V31 (128-bit) = D62 (64-bit) + D63 (64-bit) = S124-S127 (32-bit)
```

### NEON Data Types

| Data Type | Notation | Description | Example |
|-----------|----------|-------------|---------|
| 8-bit signed | Vn.16B | 16 signed bytes | [-128, 127] |
| 8-bit unsigned | Vn.16B | 16 unsigned bytes | [0, 255] |
| 16-bit signed | Vn.8H | 8 signed halfwords | [-32768, 32767] |
| 16-bit unsigned | Vn.8H | 8 unsigned halfwords | [0, 65535] |
| 32-bit signed | Vn.4S | 4 signed words | [-2^31, 2^31-1] |
| 32-bit unsigned | Vn.4S | 4 unsigned words | [0, 2^32-1] |
| 64-bit signed | Vn.2D | 2 signed doublewords | [-2^63, 2^63-1] |
| 64-bit unsigned | Vn.2D | 2 unsigned doublewords | [0, 2^64-1] |
| 32-bit float | Vn.4S | 4 single-precision floats | IEEE 754 |
| 64-bit float | Vn.2D | 2 double-precision floats | IEEE 754 |

### NEON Instruction Encoding

**Format:** `xxxx<op>xx<size><Q><-------------->`

- **op**: Operation code (what to do)
- **size**: Element size (00=8B, 01=16B, 10=32B, 11=64B)
- **Q**: Bit 0 indicates 128-bit (1) or 64-bit (0) operation
- **-------------->: Remaining bits specify registers

**Examples:**
```
ADD V0.4S, V1.4S, V2.4S    0x4E 0xE1 0x9C 0x20
ADD V0.2D, V1.2D, V2.2D    0x4E 0xE1 0x9C 0x60
FADD V0.4S, V1.4S, V2.4S   0x4E 0xE1 0xFC 0x20
```

---

## Register Mapping

### SIMD Register Mapping

| x86_64 | ARM64 | Notes |
|--------|-------|-------|
| XMM0 (128-bit float) | D0 (64-bit) or V0 (128-bit) | Split D/V based on operation |
| XMM1-XMM15 | D1-D15, V1-V31 | Direct 1:1 mapping |
| YMM0 (256-bit) | V0 + V16 | Split into two 128-bit regs |
| YMM1-YMM15 | V1-V15 + V17-V31 | Split into two 128-bit regs |
| ZMM0 (512-bit) | V0 + V16 + V32 + V48 | Split into four 128-bit regs |

### Special Cases

1. **Scalar SSE Operations (ADDSS, SUBSS, etc.)**
   - Use lower 32/64 bits of D registers
   - S0-S15 for single-precision
   - D0-D15 for double-precision

2. **Packed Operations (PADDQ, PSUBQ, etc.)**
   - Use full V registers
   - Element size encoded in instruction

3. **AVX 256-bit Operations**
   - Split into two 128-bit NEON ops
   - Lower half: V0-V15
   - Upper half: V16-V31

---

## Translation Examples

### Example 1: SSE ADDSS (Scalar Single-Precision Add)

**x86_64:**
```asm
ADDSS XMM0, XMM1    ; Add single float from XMM1 to XMM0
; Encoding: F3 0F 58 C1
; XMM0[31:0] = XMM0[31:0] + XMM1[31:0]
; XMM0[127:32] unchanged
```

**ARM64 Translation:**
```asm
FADD S0, S1         ; Add single float from S1 to S0
; Encoding: 1E 61 3C 20
; S0 = S0 + S1
; Note: Uses lower 32 bits of D0/D1
```

### Example 2: SSE2 PADDQ (Packed Add Quadword)

**x86_64:**
```asm
PADDQ XMM0, XMM1    ; Packed add 64-bit integers
; Encoding: 66 0F D4 C1
; XMM0[63:0] = XMM0[63:0] + XMM1[63:0]
; XMM0[127:64] = XMM0[127:64] + XMM1[127:64]
```

**ARM64 Translation:**
```asm
ADD V0.2D, V1.2D, V2.2D    ; Vector add 64-bit integers
; Encoding: 4E E1 9C 20
; V0[0] = V1[0] + V2[0]
; V0[1] = V1[1] + V2[1]
; Note: 2D = two 64-bit integers
```

### Example 3: AVX VADDPS (256-bit Packed Add)

**x86_64:**
```asm
VADDPS YMM0, YMM1, YMM2    ; 256-bit packed single add
; Encoding: C5 FC 58 C2
; YMM0[255:0] = YMM1[255:0] + YMM2[255:0]
```

**ARM64 Translation:**
```asm
; Lower 128-bit
ADD V0.4S, V1.4S, V2.4S    ; Encoding: 4E E1 9C 20
; V0[0-3] = V1[0-3] + V2[0-3]

; Upper 128-bit
ADD V16.4S, V17.4S, V18.4S  ; Encoding: 4E F1 9C 22
; V16[0-3] = V17[0-3] + V18[0-3]

; Note: Requires 2 instructions for AVX 256-bit
```

---

## Performance Considerations

### SSE/AVX to NEON Translation Overhead

| Instruction Class | Overhead | Notes |
|-------------------|----------|-------|
| SSE Scalar | 1x | Direct 1:1 mapping |
| SSE2 Packed 128-bit | 1x | Direct 1:1 mapping |
| AVX 256-bit | 2x | Requires 2 NEON instructions |
| AVX-512 512-bit | 4x | Requires 4 NEON instructions |

### Optimization Strategies

1. **Use 128-bit operations when possible**
   - Prefer SSE over AVX when 256-bit not needed
   - Reduces instruction count by 2x for AVX

2. **Leverage NEON's powerful permute operations**
   - TBL instruction for complex shuffles
   - ZIP/UZIP for transposing
   - EXT for inserting elements

3. **Batch similar operations**
   - Process multiple SIMD operations together
   - Improves instruction throughput

4. **Cache translated blocks**
   - Reuse translations for repeated code
   - 95-99% hit rate in steady state

---

## Limitations and Future Work

### Current Limitations

1. **AVX-512 Not Fully Supported**
   - Requires 4 NEON instructions per AVX-512 op
   - Significant performance overhead

2. **Complex Shuffles**
   - Some SSE shuffle patterns require multiple NEON ops
   - May be slower than native SSE

3. **80-bit Extended Precision**
   - Not natively supported on ARM64
   - Requires software emulation

### Future Improvements

1. **Advanced Optimizations**
   - Peephole optimization for SIMD patterns
   - Instruction fusion opportunities
   - Vector size promotion/demotion

2. **Better AVX Support**
   - Profile to find optimal AVX usage
   - Hybrid SSE/AVX code generation

3. **Cryptographic Extensions**
   - AES-NI → ARM64 crypto extensions
   - SHA → ARM64 hash instructions
   - CLMUL → ARM64 polynomial multiply

---

## Testing

### Run FP/SIMD Tests

```bash
# Test all FP/SIMD instructions
./test_fp_simd all

# Test specific categories
./test_fp_simd fp       # x87 floating-point
./test_fp_simd sse      # SSE scalar
./test_fp_simd sse2     # SSE2 integer SIMD
./test_fp_simd avx      # AVX 256-bit
./test_fp_simd neon     # ARM64 NEON
```

### Expected Results

All tests should pass with output showing:
- Instruction encodings
- Expected ARM64 translations
- Register mapping details
- Special cases and notes

---

## References

- [ARM NEON Intrinsics Reference](https://developer.arm.com/architectures/intrinsics/)
- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/)
- [ARM Architecture Reference Manual](https://developer.arm.com/documentation/)
- [Intel 64 and IA-32 Architectures SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

*Last updated: March 2026*
