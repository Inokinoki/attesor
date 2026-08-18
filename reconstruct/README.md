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
  bin/           optional original ELF (gitignored)
  oah            new host executable (make oah)
  oah-aarch64    same program, cross-built for qemu-user (make oah-aarch64)
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
| 6 | New ELF `oah`: map x86_64 guest, **x86→ARM translate**, runtime syscall shuffle | First runnable equivalent |
| 7 | Native RX jump + **qemu-user** (`oah-aarch64`) | Prove the emitted ARM is real, not only a C interpreter |

`oah` is a **new** program, not Apple’s binary. It maps an x86_64 Linux ELF (`ElfMapper`: first `PT_LOAD` at file offset 0), translates a long-mode slice to ARM64 using the recovered runtime GPR map, emits a `runtime_syscall` shuffle + `svc #0`, and runs that fragment.

### What `oah` actually does

1. **Map the guest** at its `p_vaddr` (`MAP_FIXED`). The nolibc fixture `tests/hello.x86_64` is `ET_EXEC` at `0x400000` (`tools/mk_hello_elf.py`): `write(1, "hello\n"); exit(0)`.
2. **Pack GPRs** with the runtime map from `FUN_800000040650`:
   - `X8–X15` = `RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI`
   - `X0–X7` = `R13, R14, R12, R11, R15, R10, R9, R8`
   Guest `RSP` therefore lives in **X12**, not host `SP`.
3. **Translate** `mov` / `xor` / `lea [rip]` / `push` / `pop` / `jmp` / `syscall`. `call` / `ret` still fail closed (decode error) so the dispatcher cannot loop forever.
4. **`syscall`** emits the recovered shuffle into Linux AArch64 SVC ABI (`X8` = nr, `X0–X5` = args), then `svc #0`, then restore. We do **not** copy Apple’s `runtime_syscall` bytes at `0x8000000964b8`.
5. Each fragment ends with `RET`.

Two ways to *run* that ARM:

| Path | When | What happens |
|------|------|----------------|
| Interpreter (`src/arm_exec.c`) | `make test`, and any non-aarch64 `./oah` | C switch on MOVZ/K, ORR, EOR, ADD/SUB, LDR/STR, SVC, RET. X8 still holds the **x86** nr; `oah_x86_syscall_to_host` remaps. `exit` / `exit_group` return `OAH_INTERP_EXIT` so unit tests are not killed. |
| Native (`src/arm_native.c`) | `./oah` built for aarch64, including **qemu-user** | mmap RW → memcpy → `mprotect` RX → `ic ivau` → trampoline loads `X0–X15` → `blr` fragment. `RET` returns to the trampoline. Before `svc`, compare/branch rewrites x86 nrs (`write` 1→64, `exit` 60→93, `exit_group` 231→94, …). A live exit SVC **does** terminate the process (correct for `hello`). |

`oah-aarch64` is linked **static** at `0x2000000000` (`-Wl,-Ttext-segment=0x2000000000`) so it cannot collide with the guest at `0x400000`. A default `-static` aarch64 binary would itself load at `0x400000` and `MAP_FIXED` of the guest would fail.

Glibc guests, full x86, and `call`/`ret` traces are still out of scope.

## Build and verify

Needs: `gcc`, `make`, `python3`. Optional original ELF is **not** required.

### 1. Interpreter path (any x86_64 or aarch64 Linux host)

From the repository root:

```sh
make -C reconstruct test
./reconstruct/oah reconstruct/tests/hello.x86_64
echo "exit=$?"
```

Expect every `tests/test_*` line to print `<suite>: N checks passed` (non-zero `N`), then:

```
== oah tests/hello.x86_64
```

and the second command prints `hello` and `exit=0`. `tests/test_arm_native` on x86_64 only checks that native exec is stubbed (`OAH_INTERP_DECODE`); it does not jump ARM.

On failure, `make test` exits 1 and prints `FAIL …` from `tests/test_common.h`.

### 2. Native ARM via qemu-user (x86_64 host)

Debian / Ubuntu:

```sh
sudo apt-get update
sudo apt-get install -y qemu-user-static gcc-aarch64-linux-gnu libc6-dev-arm64-cross
```

Confirm the tools:

```sh
qemu-aarch64-static --version | head -1
aarch64-linux-gnu-gcc --version | head -1
```

Then:

```sh
make -C reconstruct test-qemu
```

That target:

1. Cross-compiles `reconstruct/oah-aarch64` and `reconstruct/tests/test_arm_native-aarch64` (static, load address `0x2000000000`).
2. Runs the trampoline test under qemu (`movz x8,#42; ret` must come back with `x8==42`).
3. Runs `qemu-aarch64-static ./oah-aarch64 tests/hello.x86_64` and requires stdout to be exactly `hello\n`.

Expected:

```
== qemu qemu-aarch64-static tests/test_arm_native-aarch64
arm_native: 5 checks passed
== qemu qemu-aarch64-static oah-aarch64 tests/hello.x86_64
```

The second recipe writes `reconstruct/tests/hello.qemu.got`. Check it yourself:

```sh
od -c reconstruct/tests/hello.qemu.got
# 0000000   h   e   l   l   o  \n
```

### 3. Prove the qemu binary is ARM, not the host interpreter

```sh
file reconstruct/oah-aarch64
readelf -l reconstruct/oah-aarch64 | head -16
```

Expect `ELF 64-bit LSB executable, ARM aarch64`, statically linked, and a `LOAD` `VirtAddr` of `0x0000002000000000`.

Manual equivalent of `test-qemu` (after `make -C reconstruct oah-aarch64 tests/hello.x86_64`):

```sh
qemu-aarch64-static ./reconstruct/oah-aarch64 ./reconstruct/tests/hello.x86_64
# hello
```

Override tools if they are not on `PATH` under the default names:

```sh
make -C reconstruct test-qemu \
    CC_AARCH64=aarch64-linux-gnu-gcc \
    QEMU_AARCH64=qemu-aarch64-static
```

### 4. Native aarch64 host (no qemu)

On Linux aarch64, host `cc` already targets ARM64, so `make -C reconstruct test` runs `tests/test_arm_native` for real (5 checks: `x8` becomes 42, `x0` is preserved). `./reconstruct/oah reconstruct/tests/hello.x86_64` uses `oah_jit_run_native` (`blr` + real `svc`). Keep the guest at `0x400000` free, or link `oah` the same way `oah-aarch64` does.

## What we will not do

- Name a `FUN_*` from its address order (`v128_add` at `0x26cdc` is a syscall
  errno wrapper, not a vector add).
- Claim production-ready status.
- Treat the sibling `rosetta_trans_*.c` tree as ground truth.
- Check in Apple's `rosetta` ELF.
- Copy Apple `runtime_*` machine code; only the recovered ABI is reimplemented.
