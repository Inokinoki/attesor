# Reconstruction from RosettaLinux decompilation

This tree is a **from-scratch, evidence-based reconstruction** of Apple's
`RosettaLinux/rosetta` translator. It does **not** continue the earlier
`rosetta_*.c` rewrite (that tree mixed guessed names with empty tests).

Source of truth: `../rosetta_decomp.c` (Ghidra dump of the AArch64 binary).

## Method

1. Extract original C++ file/function names from assertion strings.
2. Reconstruct only what the dump actually supports (encodings, asserts,
   string literals, register copies).
3. Mark every function with:
   - `decomp:` Ghidra `FUN_*` address
   - `orig:` `file:line:name` when an assert names it
   - confidence: `high` (bits/strings copied) / `derived` (inferred)
4. Tests must fail if the reconstructed behavior is wrong. Printing an
   expected encoding and calling `PASS` is not a test.

## Layout

```
reconstruct/
  inventory/     Layer 0 — maps extracted from the dump
  include/oah/   Public headers
  src/           Readable C, layered
  tests/         Behavioral tests
  tools/         Extraction scripts
```

## Layers (this PR)

| Layer | What | Why this first |
|-------|------|----------------|
| 0 | Source-file / function inventory | Names come from the binary, not guesses |
| 1 | Types, Register, syscall `Result`, AssemblerBuffer | Small, fully constrained |
| 2 | ARM64 emitters named by `AssemblerBase.hpp` | Opcode bits are in the dump |
| 3 | Linux ABI: `/proc` paths, open flags, fake cpuinfo | Format strings + path compares |
| 4 | Flags (x86 RFLAGS ↔ NZCV), condition codes, x86 decoder `read_int` | Bit copies + 15-byte insn limit |

Later layers (not in this PR): `Translator.cpp` IR, AOT/JIT cache, GDB stub.
Those functions are tens of thousands of Ghidra lines and need the original
binary in IDA/BN to recover vtables and rodata tables.

## What we will not do

- Name a `FUN_*` from its address order (`v128_add` at `0x26cdc` is a syscall
  errno wrapper, not a vector add).
- Claim production-ready status.
- Treat the sibling `rosetta_trans_*.c` tree as ground truth.
