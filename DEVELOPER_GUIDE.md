# Rosetta 2 Binary Translator - Developer Guide

## Overview

This guide is for developers who want to contribute to, extend, or understand the Rosetta 2 binary translator project. It covers the codebase architecture, development workflow, and how to add new instruction translations.

## Table of Contents

1. [Project Structure](#project-structure)
2. [Development Workflow](#development-workflow)
3. [Adding New Instructions](#adding-new-instructions)
4. [Testing Guidelines](#testing-guidelines)
5. [Code Style](#code-style)
6. [Performance Optimization](#performance-optimization)
7. [Debugging Tips](#debugging-tips)
8. [Common Pitfalls](#common-pitfalls)

---

## Project Structure

### Directory Layout

```
attesor/
├── Core Translation
│   ├── rosetta_translate.c         # Main translation logic
│   ├── rosetta_translate_*.c       # Category-specific translators
│   ├── rosetta_jit.c               # JIT compilation
│   └── rosetta_cache.c             # Translation cache
│
├── Decoding
│   ├── rosetta_x86_decode.c        # x86_64 instruction decoder
│   └── rosetta_arm64_decode.c      # ARM64 instruction decoder
│
├── Code Generation
│   ├── rosetta_codegen.c           # Main code generator
│   ├── rosetta_arm64_emit.c        # ARM64 instruction emitter
│   └── rosetta_codegen_*.c         # Codegen helpers
│
├── Instruction Categories
│   ├── rosetta_translate_alu.c     # ALU instructions
│   ├── rosetta_translate_memory.c  # Memory operations
│   ├── rosetta_translate_branch.c  # Control flow
│   └── ... (one file per category)
│
├── SIMD/FP
│   ├── rosetta_simd.c              # SIMD core
│   ├── rosetta_fp_translate.c     # Floating-point translation
│   ├── rosetta_neon_*.c            # NEON operations
│   └── rosetta_refactored_float.c # FP implementation
│
├── Syscalls
│   ├── rosetta_syscalls.c          # Syscall translation
│   └── rosetta_refactored_syscall_handlers.c
│
├── Daemon
│   └── rosettad_refactored.c       # Rosetta daemon
│
├── Testing
│   ├── test_translator.c           # Instruction tests
│   ├── test_fp_simd.c              # FP/SIMD tests
│   ├── test_string_advanced.c      # String/advanced tests
│   └── test_crypto_advanced.c      # Crypto/advanced tests
│
└── Documentation
    ├── ARCHITECTURE.md             # System architecture
    ├── SIMD_FP_TRANSLATION.md      # SIMD/FP guide
    ├── CRYPTO_SIMD_TRANSLATION.md  # Crypto extensions
    ├── API_DOCUMENTATION.md        # API reference
    └── TESTING_GUIDE.md            # Testing guide
```

### Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│  (test programs, rosettad daemon, user applications)      │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────────┐
│                  Translation Interface                       │
│  (rosetta_translate.c, rosetta_jit.c, rosetta_cache.c)    │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
┌───────▼──────┐ ┌────▼─────┐ ┌─────▼────────┐
│   Decoder    │ │ Emitter  │ │  Translators │
│              │ │          │ │             │
│ - x86 decode │ │- ARM64   │ │ - ALU       │
│ - ARM64 decode│ │- emit    │ │ - Memory    │
└──────────────┘ └──────────┘ │ - Branch    │
                               │ - SIMD/FP   │
                               │ - Syscalls  │
                               └─────────────┘
```

---

## Development Workflow

### Setting Up Development Environment

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd attesor
   ```

2. **Install Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential gcc make

   # macOS
   xcode-select --install
   ```

3. **Build Project**
   ```bash
   make -f Makefile.modular all
   ```

4. **Run Tests**
   ```bash
   ./run_all_tests.sh
   ```

### Making Changes

1. **Create Feature Branch**
   ```bash
   git checkout -b feature/instruction-category
   ```

2. **Make Changes**
   - Edit source files
   - Add tests for new functionality
   - Update documentation

3. **Build and Test**
   ```bash
   make -f Makefile.modular clean
   make -f Makefile.modular all
   make -f Makefile.modular test
   ```

4. **Commit Changes**
   ```bash
   git add .
   git commit -m "Add: Description of changes"
   ```

5. **Push and Create PR**
   ```bash
   git push origin feature/instruction-category
   # Create pull request on GitHub
   ```

---

## Adding New Instructions

### Step 1: Understand the Instruction

Before implementing, understand:
- **Instruction encoding**: How is it encoded in x86_64?
- **Operation**: What does it do?
- **Operands**: What operands does it take?
- **Flags**: Which flags does it affect?
- **ARM64 equivalent**: Is there a direct ARM64 equivalent?

Resources:
- Intel 64 and IA-32 Architectures Software Developer's Manual
- ARM Architecture Reference Manual
- Existing translation examples

### Step 2: Add Decoder Support

If the instruction isn't already decoded:

**File**: `rosetta_x86_decode.c`

```c
// Add to x86_decode() function
case 0xXX:  // Your instruction opcode
    insn->opcode = data[0];
    insn->length = decode_operands(data, size, insn);
    insn->category = X86_CATEGORY_YOUR_CATEGORY;
    break;
```

### Step 3: Add Translation Function

**File**: `rosetta_translate_<category>.c`

```c
/**
 * Translate YOUR_INSTRUCTION instruction
 * x86_64: YOUR_INSTRUCTION operand1, operand2
 * Encoding: XX XX XX XX
 *
 * @param emit ARM64 emitter context
 * @param insn Decoded x86_64 instruction
 *
 * @return 0 on success, negative error code on failure
 */
int translate_your_instruction(arm64_emitter_t *emit,
                               const x86_insn_t *insn)
{
    // Validate operands
    if (insn->num_operands != 2) {
        return -1;  // Error: wrong number of operands
    }

    // Map x86_64 registers to ARM64
    uint8_t rd = map_x86_to_arm64_reg(insn->operands[0].reg);
    uint8_t rn = map_x86_to_arm64_reg(insn->operands[1].reg);

    // Emit ARM64 instruction(s)
    arm64_emit_your_instruction(emit, rd, rn);

    return 0;
}
```

### Step 4: Register with Dispatcher

**File**: `rosetta_translate_dispatch.c`

```c
// In translate_instruction() function
case X86_INSN_YOUR_INSTRUCTION:
    return translate_your_instruction(emit, insn);
```

### Step 5: Add Tests

**File**: `test_translator.c` (or new test file)

```c
void test_your_instruction(void)
{
    TEST_START("YOUR_INSTRUCTION Translation");

    uint8_t x86_insn[] = {0xXX, 0xXX, 0xXX, 0xXX};  // Encoding

    printf("  x86_64: YOUR_INSTRUCTION oper1, oper2\n");
    printf("  Encoding: %02X %02X %02X %02X\n",
           x86_insn[0], x86_insn[1], x86_insn[2], x86_insn[3]);
    printf("  Expected ARM64: YOUR_ARM_INSTRUCTION\n");

    // Add notes if needed
    printf("  Note: Any special cases or considerations\n");

    TEST_PASS();
}

// Add to test runner
void run_your_category_tests(void)
{
    test_your_instruction();
    // ... other tests
}
```

### Step 6: Update Documentation

**Update**:
- `ARCHITECTURE.md` - Add to supported instructions list
- `SIMD_FP_TRANSLATION.md` - If SIMD/FP instruction
- `API_DOCUMENTATION.md` - If new API functions
- Add inline comments to code

### Step 7: Build and Test

```bash
make -f Makefile.modular clean
make -f Makefile.modular all
./test_translator your_category
```

---

## Testing Guidelines

### Test Categories

1. **Unit Tests**: Test individual instructions
2. **Integration Tests**: Test instruction sequences
3. **End-to-End Tests**: Test full translation pipeline
4. **Performance Tests**: Benchmark translation overhead

### Writing Tests

#### Test Template

```c
void test_instruction_name(void)
{
    TEST_START("Instruction Name Translation");

    // 1. Define x86_64 instruction encoding
    uint8_t x86_insn[] = {0x...};

    // 2. Print instruction information
    printf("  x86_64: INSTR oper1, oper2\n");
    printf("  Encoding: %02X %02X ...\n", x86_insn[0], x86_insn[1]);

    // 3. Print expected ARM64 translation
    printf("  Expected ARM64: ARM_INSTR\n");

    // 4. Add notes if needed
    printf("  Note: Special behavior or considerations\n");

    TEST_PASS();
}
```

#### Test Categories

Group tests by instruction category:

```c
void run_alu_tests(void)
{
    test_add_instruction();
    test_sub_instruction();
    // ... more ALU tests
}

void run_memory_tests(void)
{
    test_mov_instruction();
    test_lea_instruction();
    // ... more memory tests
}
```

#### Test Execution

```c
int main(int argc, char *argv[])
{
    if (argc == 1 || strcmp(argv[1], "all") == 0) {
        run_all_tests();
    } else if (strcmp(argv[1], "alu") == 0) {
        run_alu_tests();
    } else if (strcmp(argv[1], "memory") == 0) {
        run_memory_tests();
    }
    // ... more categories
}
```

### Test Coverage Goals

Aim for:
- **All instructions**: At least one test per instruction
- **Edge cases**: Tests for boundary conditions
- **Combinations**: Tests for instruction sequences
- **Error cases**: Tests for invalid inputs

---

## Code Style

### Formatting

- **Indentation**: 4 spaces (no tabs)
- **Line width**: Maximum 100 characters
- **Braces**: Allman style (opening brace on new line)

```c
if (condition)
{
    do_something();
}
else
{
    do_something_else();
}
```

### Naming Conventions

- **Functions**: `snake_case`
  ```c
  int translate_add_instruction(arm64_emitter_t *emit);
  ```

- **Types**: `snake_case` with `_t` suffix
  ```c
  typedef struct {
      uint64_t guest_pc;
  } translation_context_t;
  ```

- **Constants**: `UPPER_SNAKE_CASE`
  ```c
  #define MAX_CACHE_SIZE 4096
  ```

- **Macros**: `UPPER_SNAKE_CASE`
  ```c
  #define EMIT_ADD(emit, rd, rn, rm) \
      arm64_emit_add((emit), (rd), (rn), (rm), false)
  ```

### Comments

- **File Headers**: Comprehensive module documentation
- **Function Headers**: Purpose, parameters, return values, examples
- **Inline Comments**: Explain complex logic
- **Algorithm Notes**: Explain non-obvious algorithms

```c
/**
 * Brief description of function
 *
 * Detailed description of what the function does,
 * algorithms used, and implementation notes.
 *
 * @param param1 Description of parameter 1
 * @param param2 Description of parameter 2
 *
 * @return Description of return value
 *
 * Example:
 *   int result = function(arg1, arg2);
 */
int function(int param1, int param2)
{
    // Step 1: Do this
    int value = param1 * 2;

    // Step 2: Do that (explain why)
    if (value > threshold) {
        value += param2;
    }

    return value;
}
```

---

## Performance Optimization

### Optimization Principles

1. **Cache Hit Rate**: Optimize for high cache hit rates
2. **Translation Speed**: Minimize translation overhead
3. **Code Size**: Balance between speed and code size
4. **Hot Paths**: Optimize frequently executed code

### Optimization Techniques

#### 1. Peephole Optimization

```c
// Before: Multiple instructions
emit_add(emit, X0, X1, X2);
emit_add(emit, X0, X0, X3);

// Optimize to: Single instruction
emit_add(emit, X0, X1, X2);
emit_add(emit, X0, X0, X3);
// Can combine to: emit_add_extended(emit, X0, X1, X2, X3);
```

#### 2. Constant Folding

```c
// Before: Emit load of immediate
emit_load_imm(emit, X0, 42);
emit_add(emit, X1, X1, X0);

// Optimize to: Use immediate directly
emit_add_imm(emit, X1, X1, 42);
```

#### 3. Dead Code Elimination

```c
// Detect and remove unnecessary instructions
if (is_dead_code(insn)) {
    return;  // Don't emit
}
```

#### 4. Instruction Selection

```c
// Choose optimal instruction variant
if (is_power_of_two(value) && value < 4096) {
    emit_logical_shift_left(emit, rd, rn, log2(value));
} else {
    emit_multiply_immediate(emit, rd, rn, value);
}
```

### Profiling

Use profiling to identify bottlenecks:

```c
#include <time.h>

void profile_translation()
{
    clock_t start = clock();

    // Translate code
    translate_block(&ctx, guest_pc, &code, &size);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Translation time: %.6f seconds\n", elapsed);
}
```

---

## Debugging Tips

### Enable Debug Output

```c
#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) \
    do {} while(0)
#endif
```

### Use GDB

```bash
# Build with debug symbols
gcc -g -o test_translator test_translator.c -L. -lrosetta -lm

# Run with GDB
gdb ./test_translator
(gdb) run
(gdb) bt  # Backtrace if crash
```

### Print Intermediate States

```c
void print_decoded_insn(const x86_insn_t *insn)
{
    printf("Decoded instruction:\n");
    printf("  Opcode: %02X\n", insn->opcode);
    printf("  Length: %d\n", insn->length);
    printf("  Operands: %d\n", insn->num_operands);

    for (int i = 0; i < insn->num_operands; i++) {
        printf("  Operand %d: type=%d, reg=%d\n",
               i, insn->operands[i].type,
               insn->operands[i].reg);
    }
}
```

### Validate Generated Code

```c
void validate_arm64_code(const uint8_t *code, uint32_t size)
{
    // Check for common errors
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t insn = *(uint32_t*)(code + i);

        // Check for undefined instructions
        if (is_undefined_arm64_insn(insn)) {
            printf("Error: Undefined instruction at offset %u\n", i);
        }
    }
}
```

---

## Common Pitfalls

### 1. Incorrect Register Mapping

**Problem**: Using wrong ARM64 register for x86_64 register

```c
// WRONG: Direct mapping doesn't work for all registers
uint8_t arm_reg = x86_reg;  // RAX (0) → X0 (correct) but not always

// CORRECT: Use mapping function
uint8_t arm_reg = map_x86_to_arm64_reg(x86_reg);
```

### 2. Ignoring Flags

**Problem**: Not updating condition flags

```c
// WRONG: Only emit operation
emit_add(emit, X0, X1, X2);

// CORRECT: Emit operation and update flags
emit_adds(emit, X0, X1, X2);  // Note the 'S' for set flags
```

### 3. Memory Ordering Issues

**Problem**: Not respecting memory ordering semantics

```c
// WRONG: Assume same ordering
emit_load(emit, X0, [X1]);

// CORRECT: Add memory barriers if needed
emit_dmb(emit);  // Data memory barrier
emit_load(emit, X0, [X1]);
```

### 4. Stack Frame Issues

**Problem**: Incorrect stack frame setup

```c
// WRONG: Assume same stack layout
emit_sub(emit, SP, SP, 8);  // x86_64 stack frame

// CORRECT: Account for different stack layouts
// x86_64: RBP grows downward, ARM64: SP grows downward
emit_stp(emit, X29, X30, [SP, -16]!);  // Save FP and LR
emit_mov(emit, X29, SP);                 // Set FP
emit_sub(emit, SP, SP, stack_size);      // Allocate stack
```

### 5. Ignoring Endianness

**Problem**: Assuming same endianness

```c
// WRONG: Load as-is
emit_load(emit, X0, [X1]);

// CORRECT: Handle byte order if needed
// Note: x86_64 and ARM64 are both little-endian,
// but memory-mapped I/O might differ
emit_rev(emit, X0, X0);  // Reverse bytes if needed
```

### 6. Floating-Point Precision

**Problem**: Wrong precision for FP operations

```c
// WRONG: Use wrong size register
emit_fadd(emit, D0, D1, D2);  // Double for single-precision

// CORRECT: Match precision
emit_fadd(emit, S0, S1, S2);  // Single-precision
```

### 7. Not Handling All Instruction Formats

**Problem**: Only handling one instruction variant

```c
// WRONG: Only handle REX.W prefix
if (insn->rex & 0x8) {
    // 64-bit form
}

// CORRECT: Handle all REX prefix bits
if (insn->rex & 0x8) {
    // 64-bit form
} else if (insn->rex & 0x4) {
    // REX.R prefix
} else {
    // Legacy form
}
```

### 8. Forgetting to Update Documentation

**Problem**: Code changes not reflected in docs

```bash
# Always update docs when adding features
1. Update function headers
2. Update API_DOCUMENTATION.md
3. Update relevant guides
4. Add examples
```

---

## Contributing Guidelines

### Before Contributing

1. **Check Existing Issues**: Look for related issues or PRs
2. **Discuss Large Changes**: Propose major changes first
3. **Write Tests**: Ensure new code has tests
4. **Update Docs**: Update relevant documentation

### Pull Request Checklist

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] New tests added for new features
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] Code follows style guidelines
- [ ] No regressions in performance

### Commit Message Format

```
Add: Brief description of change

Detailed description of what was changed and why.

- List specific changes
- Explain reasoning
- Reference issues if applicable

Closes #123
```

---

## Resources

### Documentation

- [Intel 64 and IA-32 Architectures SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [ARM Architecture Reference Manual](https://developer.arm.com/documentation/)
- [ARM NEON Intrinsics Reference](https://developer.arm.com/architectures/intrinsics/)
- [x86_64 Instruction Reference](https://www.felixcloutier.com/x86/)

### Tools

- **GNU Binutils**: For testing assembly
- **objdump**: For disassembling code
- **gdb**: For debugging
- **valgrind**: For memory checking
- **perf**: For performance profiling

### Community

- GitHub Issues: Report bugs and request features
- Pull Requests: Contribute code
- Discussions: Ask questions and share ideas

---

## Troubleshooting

### Build Issues

**Problem**: Compilation errors
```bash
# Solution: Check for duplicate symbols
nm -o *.o | grep duplicate_symbol

# Solution: Clean rebuild
make -f Makefile.modular clean
make -f Makefile.modular all
```

**Problem**: Undefined references
```bash
# Solution: Check library order
gcc -o test test.c -L. -lrosetta -lm

# Solution: Check for missing symbols
nm -u librosetta.a | grep undefined_symbol
```

### Runtime Issues

**Problem**: Segmentation fault
```bash
# Solution: Run with debugger
gdb ./test_program
(gdb) run
(gdb) bt  # Backtrace to find crash location
```

**Problem**: Incorrect translation
```bash
# Solution: Compare with reference implementation
# Use objdump to see generated code
objdump -d test_program

# Solution: Enable debug output
./test_program --debug
```

---

## Best Practices Summary

1. **Start Small**: Test one instruction at a time
2. **Test Thoroughly**: Add tests for each new instruction
3. **Document Well**: Explain what and why
4. **Profile First**: Optimize based on measurements
5. **Review Code**: Get feedback from others
6. **Keep It Simple**: Avoid over-engineering

---

*Happy translating!*

*Last updated: March 2026*
