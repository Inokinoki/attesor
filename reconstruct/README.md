# Reconstruction from RosettaLinux decompilation

This tree is a **from-scratch, evidence-based reconstruction** of Apple's
`RosettaLinux/rosetta` translator. It does **not** continue the earlier
`rosetta_*.c` rewrite (that tree mixed guessed names with empty tests).

Sources of truth, in order:

1. The original ELF (optional, gitignored): drop it at `bin/rosetta`
2. `../rosetta_decomp.c` (Ghidra dump of that ELF)

Do **not** commit the binary. `make inventory-binary` regenerates
`inventory/from_binary/` from it.

## Method

1. Extract original C++ file/function names from assertion strings.
2. Reconstruct only what the dump **or the ELF** actually supports
   (encodings, asserts, string literals, rodata tables, `movz x8,#NR`).
3. Mark every function with:
   - `decomp:` Ghidra `FUN_*` address
   - `orig:` `file:line:name` when an assert names it
   - confidence: `high` (bits/strings copied) / `derived` (inferred)
4. Tests must fail if the reconstructed behavior is wrong. Printing an
   expected encoding and calling `PASS` is not a test.
5. Tests must pass **without** the ELF in CI. `tests/test_binary` memcmps
   tables against `bin/rosetta` when that file is present, and skips
   otherwise.

## Layout

```
reconstruct/
  bin/           optional ELF (gitignored)
  inventory/     Layer 0 — maps from the dump and, when present, the ELF
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
| 5 | ELF rodata/text: CC table, `register_to_string`, host syscall NRs | Ghidra collapsed `svc` immediates |

Later layers (not in this PR): `Translator.cpp` IR, AOT/JIT cache, GDB stub.
Those functions are tens of thousands of Ghidra lines and still need vtables.

## What we will not do

- Name a `FUN_*` from its address order (`v128_add` at `0x26cdc` is a syscall
  errno wrapper, not a vector add).
- Claim production-ready status.
- Treat the sibling `rosetta_trans_*.c` tree as ground truth.
- Check in Apple's `rosetta` ELF.

```
make -C reconstruct test
```
