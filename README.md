# Rosetta 2 Reverse Engineering Project

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Status](https://img.shields.io/badge/status-in--progress-blue)](STATUS)

A comprehensive reverse-engineering effort to understand and document Apple's Rosetta 2 binary translation technology.

## Table of Contents

1. [Background](#background)
2. [What is Rosetta?](#what-is-rosetta)
3. [What is Rosetta 2?](#what-is-rosetta-2)
4. [How Apple Delivers Rosetta 2 in macOS](#how-apple-delivers-rosetta-2-in-macos)
5. [Technical Architecture](#technical-architecture)
6. [This Project](#this-project)
7. [File Structure](#file-structure)
8. [Usage](#usage)
9. [Progress](#progress)
10. [References](#references)

---

## Background

### The Architecture Transition

In November 2020, Apple announced their first Apple Silicon Macs, marking a historic transition from Intel x86_64 processors to their own ARM-based M1 chips. This was Apple's third major architecture transition:

1. **1994**: Motorola 68000 -> PowerPC
2. **2006**: PowerPC -> Intel x86_64
3. **2020**: Intel x86_64 -> Apple Silicon (ARM64)

Each transition required a binary translation solution to run existing software during the migration period. Rosetta 2 is Apple's most sophisticated binary translation system yet.

---

## What is Rosetta?

**Rosetta** (2006-2011) was Apple's first dynamic binary translation software, enabling PowerPC applications to run on Intel-based Macs.

### Key Features:
- **Dynamic Translation**: Translated PowerPC code to x86_64 at runtime
- **OS Integration**: Built into Mac OS X 10.4 (Tiger) through 10.6 (Snow Leopard)
- **Transparent Operation**: Users launched PowerPC apps normally
- **Performance Overhead**: Typically 20-50% slower than native code

Rosetta was removed in Mac OS X 10.7 (Lion), completing the Intel transition.

---

## What is Rosetta 2?

**Rosetta 2** is Apple's advanced dynamic binary translation technology that enables applications compiled for Intel x86_64 Macs to run on Apple Silicon (ARM64) Macs.

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    User Application (x86_64)                 │
├─────────────────────────────────────────────────────────────┤
│                     Rosetta 2 Layer                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Translator │  │  Runtime    │  │  System Call        │  │
│  │  (AOT/JIT)  │  │  Library    │  │  Translation        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    macOS Kernel (ARM64)                      │
├─────────────────────────────────────────────────────────────┤
│                    Apple Silicon Hardware                    │
└─────────────────────────────────────────────────────────────┘
```

### Key Technologies

1. **Ahead-of-Time (AOT) Translation**
   - Translates x86_64 binaries to ARM64 at install time
   - Stores translated code in a cache for faster subsequent launches
   - Reduces runtime overhead compared to pure JIT translation

2. **Just-in-Time (JIT) Translation**
   - Translates code blocks on-demand during execution
   - Handles dynamically loaded code and self-modifying code
   - Maintains translation cache for efficiency

3. **Instruction Set Translation**
   - x86_64 -> ARM64 instruction mapping
   - SSE/AVX -> NEON vector instruction translation
   - x86_64 flags -> ARM64 condition codes

4. **System Call Translation**
   - Translates x86_64 macOS syscalls to ARM64 equivalents
   - Handles different calling conventions
   - Manages register state across syscall boundaries

5. **Runtime Support**
   - CPU feature detection emulation
   - Thread-local storage handling
   - Signal and exception handling

---

## How Apple Delivers Rosetta 2 in macOS

### Installation Location

Rosetta 2 is located at:
```
/Library/Apple/usr/libexec/oah/
├── rosetta        # Main translator binary
├── rosettad       # Rosetta daemon
└── librosetta.*   # Runtime libraries
```

The `oah` directory stands for "Old Architecture Hardware" - a continuation from the PowerPC transition era.

### Automatic Installation

On Apple Silicon Macs, Rosetta 2 is **not** installed by default. It's triggered in two ways:

1. **First Launch Prompt**
   ```
   The "Rosetta" software is not installed on your Mac.
   Rosetta translates apps from Intel-based Macs for use on Apple Silicon Macs.
   ```

2. **Command-Line Installation**
   ```bash
   softwareupdate --install-rosetta --agree-to-license
   ```

### Components Delivered

| Component | Description |
|-----------|-------------|
| `RosettaLinux/rosetta` | Core ARM64 binary containing translation engine |
| `RosettaLinux/rosettad` | System daemon managing translation services |
| `debugserver -> /usr/libexec/rosetta/debugserver` | Debugging support for translated processes |
| `libRosettaRuntime` | Runtime library linked during translation |
| `translate_tool -> /usr/libexec/rosetta/translate_tool` | Translation tool for building translated binaries |

### Integration with macOS

1. **launchd Integration**: Rosetta daemon runs as a system service
2. **Code Signing**: Translated binaries are code-signed automatically
3. **Gatekeeper**: Rosetta-translated apps pass security checks
4. **System Integrity Protection**: Protected from modification

---

## Technical Architecture

### Translation Process

```
┌──────────────────────────────────────────────────────────────────┐
│ Phase 1: Binary Loading                                          │
│ ───────────────────────────────────────────────────────────────  │
│ 1. Load x86_64 Mach-O binary                                    │
│ 2. Parse segments, sections, symbols                            │
│ 3. Validate code signatures                                      │
│ 4. Map into translation context                                  │
└──────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│ Phase 2: AOT Translation                                         │
│ ───────────────────────────────────────────────────────────────  │
│ 1. Disassemble x86_64 code sections                              │
│ 2. Translate instructions to ARM64                               │
│ 3. Apply optimizations                                           │
│ 4. Store in translation cache (~/.oah)                          │
└──────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────────────┐
│ Phase 3: Runtime Execution                                       │
│ ───────────────────────────────────────────────────────────────  │
│ 1. Load translated ARM64 code                                    │
│ 2. Set up x86_64 emulation context                               │
│ 3. Handle JIT translations for dynamic code                      │
│ 4. Translate syscalls on-the-fly                                 │
└──────────────────────────────────────────────────────────────────┘
```

### Key Translation Challenges

1. **Register Mapping**
   - x86_64 has 16 GPRs; ARM64 has 31 GPRs
   - x86_64 flags register -> ARM64 NZCV flags
   - RIP (instruction pointer) emulation

2. **Memory Ordering**
   - x86_64: Strong memory ordering (TSO)
   - ARM64: Weak memory ordering
   - Requires memory barriers for correctness

3. **Vector Instructions**
   - SSE (128-bit) -> NEON (128-bit) direct mapping
   - AVX (256-bit) -> NEON pair emulation
   - Different exception handling for SIMD

4. **Calling Conventions**
   - x86_64: First 6 args in registers (RDI, RSI, RDX, RCX, R8, R9)
   - ARM64: First 8 args in registers (X0-X7)
   - Different stack frame layouts

---

## This Project

This repository contains reverse-engineered implementations of functions from the Rosetta 2 binaries. Through careful analysis and decompilation, we've identified and documented the semantic purpose of hundreds of functions.

### Goals

1. **Educational**: Understand how Rosetta 2 works internally
2. **Documentation**: Create comprehensive documentation of translation techniques
3. **Implementation**: Provide clean, well-documented C implementations
4. **Community**: Share knowledge with the reverse-engineering community

### What We've Accomplished

- **828 functions** identified and named in the main `rosetta` binary
- **612 functions** fully implemented with clean C code
- **66 categories** of functionality documented
- Complete function name mappings with semantic names

### Categories of Functions

| Category | Functions | Description |
|----------|-----------|-------------|
| Entry Point | 1 | Rosetta initialization |
| FP/Vector Operations | ~20 | Floating-point and SIMD state management |
| SIMD Memory Operations | ~10 | memchr, memcmp, memcpy with SIMD |
| Vector Operations | ~30 | NEON vector arithmetic, comparison |
| Binary Translation | ~50 | x86_64 -> ARM64 instruction translation |
| Syscall Handlers | ~60 | System call translation and forwarding |
| Memory Management | ~20 | malloc, free, mmap wrappers |
| Hash Functions | ~5 | Address hashing for translation cache |
| String Operations | ~30 | SIMD-optimized string functions |
| Cryptographic Extensions | ~30 | AES, SHA, CRC32 passthrough |
| ELF Parsing | ~15 | Linux binary format support |
| Translation Cache | ~20 | AOT/JIT cache management |

---

## File Structure

### Core Files

```
Rosetta2/
├── README.md                      # This file
├── rosetta_decomp.c               # Original decompilation (74,677 lines)
├── rosettad_decomp.c              # Daemon decompilation (44,064 lines)
├── rosetta_refactored.c           # Minimal wrapper (59 lines) - includes modular headers
├── rosetta_refactored.c.legacy    # Legacy refactored code (19,302 lines) - archived
├── rosetta_refactored.h           # Main header (1,215 lines)
├── rosetta_refactored_complete.c  # Single-file implementation (2,686 lines)
├── rosetta_function_map.h         # Function name mapping (828 functions)
├── rosettad_refactored.c          # Daemon-side refactoring
└── SESSION_*.md                   # Session logs (30+ sessions)
```

### Modular Translation Infrastructure (56 C files + 65 H files)

The translation infrastructure is fully modularized into categorized components:

```
├── rosetta_types.h                # Base type definitions
├── rosetta_x86_decode.h/.c        # x86 decoder
├── rosetta_arm64_emit.h/.c        # ARM64 emitter
├── rosetta_translate_alu.h/.c     # ALU translations
├── rosetta_translate_memory.h/.c  # Memory translations
├── rosetta_translate_branch.h/.c  # Branch translations
├── rosetta_translate_bit.h/.c     # Bit manipulation
├── rosetta_translate_string.h/.c  # String operations
├── rosetta_translate_special.h/.c # Special instructions
├── rosetta_translate_block.h/.c   # Block translation coordinator
├── rosetta_translate_dispatch.h/.c # Instruction dispatch
├── rosetta_trans_dispatch.h/.c    # Main dispatch layer
├── rosetta_trans_alu.h/.c         # ALU emulation layer
├── rosetta_trans_mem.h/.c         # Memory emulation layer
├── rosetta_trans_branch.h/.c      # Branch emulation layer
├── rosetta_trans_bit.h/.c         # Bit emulation layer
├── rosetta_trans_string.h/.c      # String emulation layer
├── rosetta_trans_special.h/.c     # Special instruction emulation
├── rosetta_trans_system.h/.c      # System instruction emulation
├── rosetta_trans_neon.h/.c        # NEON emulation layer
└── Makefile.modular               # Modular build system
```

### Functional Modules

Core infrastructure and support modules:

```
├── rosetta_jit.h/.c               # JIT compilation infrastructure
├── rosetta_exec.h/.c              # Execution engine (NEW)
├── rosetta_init.h/.c              # Initialization & FP registers (NEW)
├── rosetta_codegen.h/.c           # Code generation primitives
├── rosetta_cache.h/.c             # Translation cache (AOT/JIT)
├── rosetta_transcache.h/.c        # Translation cache management
├── rosetta_context.h/.c           # CPU context management
├── rosetta_runtime.h/.c           # Runtime support
├── rosetta_hash.h/.c              # Address hashing
├── rosetta_memmgmt.h/.c           # Memory management
├── rosetta_memory_utils.h/.c      # Memory utilities
├── rosetta_utils.h/.c             # Utility functions
├── rosetta_string_utils.h/.c      # String utilities
├── rosetta_trans_helpers.h/.c     # Translation helpers
├── rosetta_refactored_helpers.h/.c # Refactoring helpers
└── rosetta_refactored_types.h     # Refactored type definitions
```

### SIMD/Vector Modules

SIMD and vector operation modules:

```
├── rosetta_simd.h/.c              # SIMD operations
├── rosetta_simd_mem.h/.c          # SIMD memory operations
├── rosetta_simd_mem_helpers.h/.c  # SIMD memory helpers
├── rosetta_vector.h/.c            # Vector operations
├── rosetta_refactored_vector.h/.c # Refactored vector ops
├── rosetta_jit_emit.h/.c          # JIT emission
├── rosetta_jit_emit_simd.h/.c     # SIMD JIT emission
├── rosetta_fp_translate.h/.c      # FP translation
├── rosetta_fp_helpers.h/.c        # FP helpers
├── rosetta_trans_neon.c           # NEON translation
└── rosetta_string_simd.c          # SIMD string operations
```

### Syscall Modules

Guest syscall handling and translation:

```
├── rosetta_syscalls.h/.c          # Syscall translation core
├── rosetta_syscalls_impl.h/.c     # Syscall implementations
└── rosetta_crypto.h/.c            # Crypto instructions (AES, SHA, CRC32)
```

### Additional Modules

Additional translation and support modules:

```
├── rosetta_translate.h/.c         # Translation core
├── rosetta_translate_alu_impl.h/.c     # ALU implementation details
├── rosetta_translate_memory_impl.h/.c  # Memory implementation details
├── rosetta_translate_branch_impl.h/.c  # Branch implementation details
├── rosetta_translate_special_impl.h/.c # Special implementation details
└── rosetta_arm64_insns.h          # ARM64 instruction definitions
```

### Test Files

```
├── test_jit.c                     # JIT unit tests (737 lines)
└── test_translate.c               # Translation tests (1,059 lines)
```

**Total: 50+ source files, ~150,000+ lines of code**

---

## Build System

### Prerequisites

- GCC or Clang with C11 support
- macOS or Linux (POSIX-compatible system)

### Building the Library

```bash
# Build static library using modular Makefile
make -f Makefile.modular all

# This creates librosetta.a static library
```

### Building Individual Components

```bash
# Compile core modules
gcc -c -I. -std=c11 rosetta_types.h
gcc -c -I. -std=c11 rosetta_codegen.c
gcc -c -I. -std=c11 rosetta_jit.c
gcc -c -I. -std=c11 rosetta_x86_decode.c
gcc -c -I. -std=c11 rosetta_arm64_emit.c

# Compile translation modules
gcc -c -I. -std=c11 rosetta_translate_alu.c
gcc -c -I. -std=c11 rosetta_translate_memory.c
gcc -c -I. -std=c11 rosetta_translate_branch.c
gcc -c -I. -std=c11 rosetta_translate_block.c
gcc -c -I. -std=c11 rosetta_translate_dispatch.c

# Compile support modules
gcc -c -I. -std=c11 rosetta_cache.c
gcc -c -I. -std=c11 rosetta_context.c
gcc -c -I. -std=c11 rosetta_syscalls.c
gcc -c -I. -std=c11 rosetta_runtime.c
```

### Running Tests

```bash
# Build and run JIT tests
make -f Makefile.modular test_jit

# Build and run translation tests
make -f Makefile.modular test_translate
```

### Using as a Library

```bash
# Link against the static library
gcc -o my_app my_app.c -L. -lrosetta

# Or compile with source files directly
gcc -I. -o my_app my_app.c rosetta_*.c
```

---

## Decompiled Source File Analysis

The original decompiled file `rosetta_decomp.c` contains string literals that reveal the original source code structure. These file names appear in assertion/error messages throughout the binary.

**Note:** The refactored code uses a different, more modular structure than the original.

### Header Files (.h)
- `Register.h`
- `TaggedPointer.h`
- `RedBlackTree.h`
- `TransactionalList.h`
- `Translator.h`
- `AssemblerBuffer.h`
- `BuilderBase.h`
- `IrBuilder_x86.h`

### C++ Source Files (.cpp)
- `Repatch.cpp`
- `Decoder.cpp`
- `Fixup.cpp`
- `AssemblerHelpers.cpp`
- `Operand.cpp`
- `Opcode.cpp`
- `BasicBlock.cpp`
- `ThreadContextFcntl.cpp`
- `InitStack.cpp`
- `Thread.cpp`
- `ThreadContext.cpp`
- `ThreadContextRuntimeSignals.cpp`
- `ThreadContextVm.cpp`
- `VMAllocationTracker.cpp`
- `Vdso.cpp`
- `ProcMapsParser.cpp`
- `ThreadContextSignals.cpp`
- `ThreadContextSyscalls.cpp`
- `TranslationCacheAot.cpp`
- `TranslationCacheJit.cpp`
- `TranslationCache.cpp`
- `Translator.cpp`
- `RuntimeLibraryBridgeInternal.cpp`
- `TwoLevelOffsetMap.cpp`
- `DeltaCodedOffsetMap.cpp`

### C++ Header Files (.hpp)
- `AssemblerBase.hpp`
- `TranslatorBase.hpp`

---

## Progress

### Current Status

| Metric | Value |
|--------|-------|
| Total Functions | 828 |
| Functions Mapped | 828 (100%) |
| Functions Implemented | 600+ |
| Completion | ~75% |
| Categories Documented | 66 |
| Source Files | 56 C + 65 H |
| Total Lines of Code | ~150,000+ |

### Modular Architecture (Complete)

The translation infrastructure is fully modularized into the following components:

| Module Category | Files | Description |
|-----------------|-------|-------------|
| Core Types | `rosetta_types.h` | Base type definitions |
| Execution Engine | `rosetta_exec.h/.c` | execute_translated, context switching |
| Initialization | `rosetta_init.h/.c` | init_translation_env, FP registers |
| x86 Decoding | `rosetta_x86_decode.h/.c` | x86_64 instruction decoder |
| ARM64 Emission | `rosetta_arm64_emit.h/.c` | ARM64 code emission |
| Code Generation | `rosetta_codegen.h/.c` | Code generation primitives |
| JIT Core | `rosetta_jit.h/.c` | JIT compilation infrastructure |
| Translation Cache | `rosetta_cache.h/.c`, `rosetta_transcache.h/.c` | Block caching (AOT/JIT) |
| Block Translation | `rosetta_translate_block.h/.c` | Basic block translation |
| Instruction Dispatch | `rosetta_trans_dispatch.h/.c` | Instruction dispatching |
| ALU Translation | `rosetta_translate_alu.h/.c`, `rosetta_trans_alu.h/.c` | Arithmetic/logic ops |
| Memory Translation | `rosetta_translate_memory.h/.c`, `rosetta_trans_mem.h/.c` | Load/store operations |
| Branch Translation | `rosetta_translate_branch.h/.c`, `rosetta_trans_branch.h/.c` | Control flow |
| Bit Translation | `rosetta_translate_bit.h/.c`, `rosetta_trans_bit.h/.c` | Bit manipulation |
| String Translation | `rosetta_translate_string.h/.c`, `rosetta_trans_string.h/.c` | String operations |
| Special Translation | `rosetta_translate_special.h/.c`, `rosetta_trans_special.h/.c` | Special instructions |
| System Translation | `rosetta_trans_system.h/.c` | System registers |
| NEON Translation | `rosetta_trans_neon.c` | SIMD/NEON operations |
| SIMD Ops | `rosetta_simd.h/.c`, `rosetta_simd_mem.h/.c` | SIMD operations |
| Vector Ops | `rosetta_vector.h/.c` | Vector operations |
| FP Translation | `rosetta_fp_translate.h/.c`, `rosetta_fp_helpers.h/.c` | Floating-point |
| JIT Emit | `rosetta_jit_emit.h/.c`, `rosetta_jit_emit_simd.h/.c` | JIT emission |
| Syscalls | `rosetta_syscalls.h/.c`, `rosetta_syscalls_impl.h/.c` | Syscall handling |
| Crypto | `rosetta_crypto.h/.c` | AES, SHA, CRC32 |
| Context | `rosetta_context.h/.c` | CPU context save/restore |
| Runtime | `rosetta_runtime.h/.c` | Runtime entry point |
| Memory Mgmt | `rosetta_memmgmt.h/.c` | Memory management |
| Utilities | `rosetta_utils.h/.c`, `rosetta_string_utils.h/.c` | Utility functions |

**Total: 40+ modular components**

### Translation Coverage

| Category | Instructions |
|----------|--------------|
| ALU | ADD, SUB, AND, OR, XOR, MUL, DIV, INC, DEC, NEG, NOT, SHL, SHR, SAR, ROL, ROR |
| Memory | MOV, MOVZX, MOVSX, MOVSXD, LEA, PUSH, POP, CMP, TEST |
| Branch | Jcc, JMP, CALL, RET, CMOVcc, SETcc, XCHG |
| Bit | BSF, BSR, POPCNT, BT, BTS, BTR, BTC |
| String | MOVS, STOS, LODS, CMPS, SCAS |
| Special | CPUID, RDTSC, SHLD, SHRD, CWD, CDQ, CQO, CLI, STI, NOP |
| SIMD | SSE, SSE2, SSE3, SSSE3, SSE4.x |
| FP | x87, SSE scalar FP |
| Crypto | AES-NI, SHA, CRC32 |

### Recent Sessions

| Session | Focus | Files Created/Modified |
|---------|-------|------------------------|
| 61+ | Full Modularization | 35+ modular components |
| 61 | Translation Modularization | 6 translation modules + x86_decode enhancements |
| 60 | Translation Infrastructure | translate_block() core implementation |
| 59 | Syscall Implementation | Additional syscall handlers |
| 58 | Syscall Translation | I/O vector and network handlers |
| 57 | Memory Management | VM allocation tracker enhancements |
| 56 | SIMD Operations | Advanced SIMD translations |
| 55 | FP/SIMD | Floating-point instruction translation |
| 54 | Crypto Extensions | AES-NI passthrough implementation |
| 53 | Crypto Extensions | SHA and CRC32 instructions |
| 52 | String Operations | SIMD-optimized string functions |
| 51 | Vector Operations | NEON vector arithmetic |
| 50 | Vector Conversions | Floating-point conversions |
| 49 | Translation Cache | AOT/JIT cache management |
| 48 | JIT Core | JIT compilation infrastructure |
| 46-47 | Code Generation | x86_64 code generation helpers |
| 45 | Decode Helpers | ARM64 decode utilities |

---

## References

### Official Apple Documentation

- [Porting Your macOS Apps to Apple Silicon](https://developer.apple.com/documentation/apple_silicon/porting_your_macos_apps_to_apple_silicon)
- [Addressing Architecture Differences](https://developer.apple.com/documentation/apple_silicon/about_the_rosetta_translation_environment)

### Technical Resources

- [ARM64 System V ABI](https://github.com/ARM-software/abi-aa)
- [Intel 64 and IA-32 Architectures SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [ARM Architecture Reference Manual](https://developer.arm.com/documentation)

### Related Projects

- [FEX-Emu](https://github.com/FEX-Emu/FEX) - Linux x86_64 on ARM64 emulator
- [QEMU](https://www.qemu.org/) - Generic machine emulator
- [Rosetta Linux](https://github.com/Apple-Rosetta) - Community research project

---

## Disclaimer

This project is for **educational and research purposes only**.

- Rosetta 2 is proprietary Apple software
- This project does not distribute Apple's binaries
- All code in this repository is written by Claude Code with Qwen 3.5.
- Do not use this project to circumvent Apple's security measures

---

## License

MIT License - See [LICENSE](LICENSE) file for details.

---

## Contributing

Contributions are welcome! Areas of interest:

1. Implementing remaining functions
2. Improving documentation
3. Adding test cases
4. Performance analysis
5. Architecture diagrams

---

*Last updated: February 2026*
