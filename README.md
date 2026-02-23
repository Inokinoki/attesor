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

```
Rosetta2/
├── README.md                      # This file
├── rosetta_decomp.c               # Original decompilation (74,677 lines)
├── rosettad_decomp.c              # Daemon decompilation
├── rosetta_refactored.c           # Refactored implementations
├── rosetta_refactored.h           # Type definitions and declarations
├── rosetta_refactored_complete.c  # Complete refactored code
├── rosetta_refactored_complete.h  # Complete header with implementations
├── rosetta_function_map.h         # Function name mapping table
├── rosettad_refactored.c          # Daemon-side refactoring
├── REFACTORING_COMPLETE.md        # Refactoring completion summary
└── SESSION_*.md                   # Session-by-session progress logs
```

---

## Usage

### Building

```bash
# Compile with GCC
gcc -c rosetta_refactored.c -o rosetta_refactored.o

# Include in your project
#include "rosetta_refactored.h"

# Or use the single-header implementation
#define ROSETTA_IMPLEMENTATION
#include "rosetta_refactored_complete.h"
```

### Example: Using Translation Functions

```c
#include "rosetta_refactored.h"

// Initialize Rosetta state
thread_state_t *state = create_thread_state();

// Translate a basic block
void *translated = translate_block(guest_pc);

// Execute translated code
execute_translated_block(translated, state);
```

---

## Progress

### Current Status

| Metric | Value |
|--------|-------|
| Total Functions | 828 |
| Functions Implemented | 612 |
| Completion | 74% |
| Categories Complete | 66/66 |

### Recent Sessions

| Session | Functions | Focus |
|---------|-----------|-------|
| 34 | 27 | Additional Utility Functions |
| 33 | 19 | Cryptographic Extensions (SHA/CRC32) |
| 32 | 10 | Cryptographic Extensions (AES) |
| 31 | 27 | Advanced SIMD Operations |
| 30 | 21 | Saturating Convert Operations |

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
- All code in this repository is original clean-room implementation
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
