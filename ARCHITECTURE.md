# Rosetta 2 Architecture Documentation

## Overview

This project is a clean-room reverse engineering implementation of Apple's Rosetta 2 binary translation technology. It translates x86_64 (AMD64) instructions to ARM64 instructions, enabling x86_64 binaries to run on ARM64 platforms.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     User Application (x86_64)                   │
├─────────────────────────────────────────────────────────────────┤
│                    Rosetta 2 Translation Layer                  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │
│  │   Decoder    │  │  Translator  │  │   Code Cache         │   │
│  │  (x86_64)    │→ │  (x86→ARM64) │→ │   (AOT/JIT)          │   │
│  └──────────────┘  └──────────────┘  └──────────────────────┘   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │
│  │   Optimizer  │  │  Runtime     │  │  Syscall Handler     │   │
│  │ (Peephole)   │  │  Library     │  │  (x86→ARM64)         │   │
│  └──────────────┘  └──────────────┘  └──────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                    Host OS (ARM64)                               │
├─────────────────────────────────────────────────────────────────┤
│                    ARM64 Hardware                                │
└─────────────────────────────────────────────────────────────────┘
```

## Translation Pipeline

### 1. Binary Loading (`rosetta_macho_loader.c`)

**Purpose**: Load and parse x86_64 binaries

**Key Functions**:
- `rosetta_macho_load()` - Load Mach-O binary
- `rosetta_macho_parse_header()` - Parse file header
- `rosetta_macho_parse_segments()` - Load segments into memory

**Process**:
1. Open and validate Mach-O file
2. Parse load commands
3. Map __TEXT, __DATA, __LINKEDIT segments
4. Apply relocations if needed
5. Setup entry point

### 2. Instruction Decoding (`rosetta_x86_decode.c`)

**Purpose**: Decode x86_64 instructions into intermediate representation

**Key Data Structures**:
```c
typedef struct {
    uint8_t opcode;           // Instruction opcode
    uint8_t rex_prefix;       // REX prefix (64-bit mode)
    uint8_t legacy_prefixes;  // Legacy prefixes
    uint8_t modrm;            // ModR/M byte
    uint8_t sib;              // SIB byte
    uint64_t displacement;    // Displacement value
    uint64_t immediate;       // Immediate value
    operand_t src;            // Source operand
    operand_t dst;            // Destination operand
} x86_insn_t;
```

**Key Functions**:
- `x86_decode_insn()` - Decode single instruction
- `x86_decode_operand()` - Decode operand
- `x86_decode_modrm()` - Parse ModR/M byte
- `x86_decode_sib()` - Parse SIB byte

**Supported Instructions**:
- **ALU**: ADD, SUB, AND, OR, XOR, MUL, DIV, INC, DEC, NEG, NOT
- **Memory**: MOV, MOVZX, MOVSX, LEA, PUSH, POP
- **Branch**: JMP, Jcc, CALL, RET, CMOVcc, SETcc
- **Bit**: BSF, BSR, BT, BTS, BTR, BTC
- **String**: MOVS, STOS, LODS, CMPS, SCAS
- **System**: CPUID, RDTSC, SYSCALL

### 3. Translation (`rosetta_translate_*.c`)

**Purpose**: Translate x86_64 IR to ARM64 machine code

**Translation Strategy**:

#### Register Mapping
```
x86_64          ARM64
─────────────────────────
RAX      →      X0
RCX      →      X1
RDX      →      X2
RBX      →      X3
RSP      →      X4
RBP      →      X5
RSI      →      X6
RDI      →      X7
R8-R15   →      X8-X15
```

#### Flags Translation
```
x86_64 EFLAGS                ARM64 NZCV
─────────────────────────────────────────
CF (Carry)          →        C flag
PF (Parity)         →        Computed from result
AF (Auxiliary)      →        Computed from result
ZF (Zero)           →        Z flag
SF (Sign)           →        N flag
OF (Overflow)       →        V flag
```

**Key Translation Files**:
- `rosetta_translate_alu.c` - Arithmetic/logic instructions
- `rosetta_translate_memory.c` - Load/store operations
- `rosetta_translate_branch.c` - Control flow instructions
- `rosetta_translate_bit.c` - Bit manipulation
- `rosetta_translate_string.c` - String operations
- `rosetta_translate_simd.c` - SIMD/SSE instructions

### 4. Code Generation (`rosetta_codegen.c`)

**Purpose**: Emit ARM64 machine code

**Key Functions**:
- `emit_add_reg_reg()` - Emit ADD instruction
- `emit_sub_reg_reg()` - Emit SUB instruction
- `emit_ldr_reg_offset()` - Emit load with offset
- `emit_str_reg_offset()` - Emit store with offset
- `emit_branch_cond()` - Emit conditional branch

**Encoding Format**:
ARM64 instructions are fixed-width 32-bit:
```
31  28 24 20 16 12 8  4 0
┌─────┬─────┬─────┬─────┐
│ op  │     │     │     │
└─────┴─────┴─────┴─────┘
```

### 5. Translation Cache (`rosetta_cache.c`, `rosetta_transcache.c`)

**Purpose**: Cache translated code blocks

**Cache Structure**:
```c
typedef struct {
    uint64_t guest_addr;     // Original x86_64 address
    uint64_t host_addr;      // Translated ARM64 address
    uint32_t block_size;     // Size of translated block
    uint32_t hash;           // Address hash
    uint32_t refcount;       // Reference count
    uint32_t flags;          // Block flags
    struct cache_entry *next; // Next entry (chaining)
} TranslationCacheEntry;
```

**Cache Policies**:
- **Direct-Mapped**: Single entry per hash bucket
- **LRU**: Least-recently-used eviction
- **Block Chaining**: Direct links between blocks

### 6. Syscall Translation (`rosetta_syscalls.c`)

**Purpose**: Translate x86_64 syscalls to ARM64 syscalls

**Syscall Mapping**:
```c
static const SyscallEntry syscall_table[] = {
    {X86_64_SYS_READ,     ARM64_SYS_READ,     syscall_read},
    {X86_64_SYS_WRITE,    ARM64_SYS_WRITE,    syscall_write},
    {X86_64_SYS_OPEN,     ARM64_SYS_OPEN,     syscall_open},
    // ... more mappings
};
```

**Argument Translation**:
```
x86_64: RDI, RSI, RDX, R10, R8, R9
ARM64:   X0,  X1,  X2,  X3,  X4, X5
```

### 7. Runtime Support (`rosetta_runtime.c`, `rosetta_exec.c`)

**Purpose**: Execute translated code and manage runtime state

**Thread State**:
```c
typedef struct {
    CPUContext guest;       // Guest (x86_64) CPU state
    CPUContext host;        // Host (ARM64) CPU state
    uint64_t syscall_result;// Syscall return value
    void *translation_cache;// Active translation cache
    uint32_t flags;         // Runtime flags
} ThreadState;
```

## Execution Modes

### AOT (Ahead-of-Time) Translation

**Process**:
1. Load x86_64 binary
2. Disassemble all code sections
3. Translate to ARM64
4. Store in persistent cache
5. Load and execute translated code

**Benefits**:
- Faster startup (no translation overhead)
- Better optimization opportunities
- Persistent cache across runs

**File**: `rosetta_cache.c` (AOT mode)

### JIT (Just-in-Time) Translation

**Process**:
1. Load x86_64 binary
2. Start execution
3. On cache miss: translate block on-demand
4. Cache translated block
5. Continue execution

**Benefits**:
- Faster initial load
- Only translates executed code
- Handles self-modifying code

**File**: `rosetta_jit.c`

### Interpretation

**Process**:
1. Decode x86_64 instruction
2. Execute via software emulation
3. Update guest state
4. Fetch next instruction

**Use Cases**:
- Debugging
- Single-stepping
- Non-performance-critical code

**File**: `rosetta_exec.c` (interpreter mode)

## Optimization Passes

### 1. Peephole Optimization (`rosetta_peephole.c`)

**Patterns**:
- Remove redundant MOVs
- Fold constant arithmetic
- Eliminate dead code
- Combine load/store pairs

**Example**:
```
Before:  MOV X0, X1
         MOV X2, X0

After:   MOV X2, X1
```

### 2. Block Chaining (`rosetta_jit.c`)

**Purpose**: Direct links between blocks

**Implementation**:
```c
int translation_chain_blocks(TranslationBlock *from,
                              TranslationBlock *to) {
    from->successor = to;
    to->predecessor = from;
    from->flags |= BLOCK_FLAG_LINKED;
    return 0;
}
```

**Benefits**:
- Eliminates dispatch overhead
- Improves branch prediction
- Better I-cache utilization

### 3. Hot Path Optimization (`rosetta_optimizer.c`)

**Strategies**:
- Inline frequently-called functions
- Unroll small loops
- Specialize for constant operands

## Memory Management

### Guest Memory Layout

```
0x00000000  ┌─────────────────┐
            │  Reserved       │
0x00100000  ├─────────────────┤
            │  Text Segment   │  (Code)
            ├─────────────────┤
            │  Data Segment   │  (Data)
            ├─────────────────┤
            │  BSS Segment    │  (Zero-filled)
            ├─────────────────┤
            │  Heap           │  (Grows up)
            │                 │
            │                 │
            │     Stack       │  (Grows down)
0x7fffffff  └─────────────────┘
```

### Memory Mapping Functions

**File**: `rosetta_memory_utils.c`, `rosetta_memmgmt.c`

- `memory_map_guest()` - Map guest memory region
- `memory_unmap_guest()` - Unmap guest memory region
- `memory_protect_guest()` - Change protection flags
- `memory_translate_addr()` - Translate guest to host address

## SIMD/NEON Translation

### SSE to NEON Mapping

**File**: `rosetta_neon_*.c`, `rosetta_simd_*.c`

| SSE (128-bit) | NEON (128-bit) |
|---------------|----------------|
| XMM0-XMM15    | V0-V31         |
| MOVAPS        | LD1/ST1        |
| ADDPS         | FADD           |
| MULPS         | FMUL           |
| ANDPS         | AND            |

**Key Modules**:
- `rosetta_neon_arith.c` - Vector arithmetic
- `rosetta_neon_logic.c` - Vector logic operations
- `rosetta_neon_shift.c` - Vector shifts
- `rosetta_neon_compare.c` - Vector comparisons
- `rosetta_neon_loadstore.c` - Vector memory operations

## Exception Handling

### Signal Translation

**File**: `rosetta_refactored_signal.c`

**Signals**:
- SIGSEGV - Memory access violation
- SIGILL - Illegal instruction
- SIGFPE - Floating-point exception
- SIGBUS - Bus error

**Process**:
1. Catch signal on ARM64 host
2. Determine cause
3. Map to x86_64 exception
4. Deliver to guest process

## Performance Considerations

### Translation Overhead

- **Cold start**: First execution requires translation
- **Warm start**: Cached translations execute natively
- **Interpretation**: 10-100x slower than native

### Optimization Tips

1. **Use AOT for frequently-run code**
2. **Enable block chaining for hot paths**
3. **Profile to find hot blocks**
4. **Adjust cache size for workload**

## Debugging

### Debug Functions

**File**: `rosetta_refactored_debug.c`, `rosetta_refactored_stats.c`

- `rosetta_print_state()` - Print CPU state
- `rosetta_dump_cache()` - Dump translation cache
- `rosetta_get_stats()` - Get execution statistics

### Statistics Tracked

- Blocks translated
- Cache hits/misses
- Instructions executed
- Syscalls made
- Exceptions raised

## Module Dependencies

```
rosetta_refactored.c (main)
├── rosetta_init.c              (initialization)
├── rosetta_exec.c              (execution)
├── rosetta_x86_decode.c        (x86 decoding)
├── rosetta_codegen.c           (code generation)
├── rosetta_translate_*.c       (translation)
├── rosetta_cache.c             (caching)
├── rosetta_syscalls.c          (syscalls)
└── rosetta_runtime.c           (runtime)
```

## Future Work

1. **AVX Support** - 256-bit vector translation
2. **Optimization** - More aggressive optimizations
3. **Profiling** - Built-in profiler
4. **Debugging** - Enhanced debugging support
5. **Testing** - Comprehensive test suite

## References

- [ARM64 Reference Manual](https://developer.arm.com/documentation/)
- [Intel 64 and IA-32 Architectures SDM](https://www.int.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Apple Rosetta 2 Documentation](https://developer.apple.com/documentation/apple-silicon/about-the-rosetta-translation-environment)
