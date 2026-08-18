#!/usr/bin/env python3
"""Extract rodata / text tables from the RosettaLinux ELF.

Place the binary at reconstruct/bin/rosetta (gitignored; do not commit it).
Ghidra VAs match the ELF load address 0x800000000000, and PT_LOAD is
identity-mapped, so file offset = VA - base.
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

BASE = 0x800000000000
CMN_X0_FFF = 0xB13FFC1F  # cmn x0, #0xfff
SVC_0 = 0xD4000001
RET = 0xD65F03C0

# asm-generic unistd names for AArch64 (only NRs we actually recover).
AARCH64_NR = {
    0: "io_setup",
    1: "io_destroy",
    5: "setxattr",
    8: "getxattr",
    11: "listxattr",
    14: "removexattr",
    21: "epoll_ctl",
    22: "epoll_pwait",
    23: "dup",
    24: "dup3",
    25: "fcntl",
    29: "ioctl",
    32: "flock",
    34: "mkdirat",
    35: "unlinkat",
    43: "statfs",
    46: "ftruncate",
    48: "faccessat",
    53: "fchmodat",
    54: "fchownat",
    56: "openat",
    57: "close",
    61: "getdents64",
    62: "lseek",
    63: "read",
    64: "write",
    67: "pread64",
    68: "pwrite64",
    72: "pselect6",
    73: "ppoll",
    74: "signalfd4",
    78: "readlinkat",
    79: "newfstatat",
    80: "fstat",
    88: "utimensat",
    93: "exit",
    98: "futex",
    101: "nanosleep",
    103: "setitimer",
    117: "ptrace",
    129: "kill",
    131: "tgkill",
    134: "rt_sigaction",
    135: "rt_sigprocmask",
    139: "rt_sigreturn",
    160: "uname",
    163: "getrlimit",
    167: "prctl",
    168: "getcpu",
    169: "gettimeofday",
    172: "getpid",
    178: "gettid",
    179: "sysinfo",
    191: "mq_getsetattr",
    195: "semget",
    196: "semctl",
    197: "semtimedop",
    198: "socket",
    200: "bind",
    201: "listen",
    202: "accept",
    203: "connect",
    208: "setsockopt",
    211: "sendmsg",
    212: "recvmsg",
    215: "munmap",
    216: "mremap",
    220: "clone",
    221: "execve",
    222: "mmap",
    226: "mprotect",
    233: "madvise",
    240: "rt_tgsigqueueinfo",
    260: "wait4",
    278: "getrandom",
    279: "memfd_create",
    281: "execveat",
    288: "pkey_mprotect",
}

X86_CC = "O NO B AE E NE BE A S NS P NP L GE LE G".split()
ARM_CC = [
    "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC",
    "HI", "LS", "GE", "LT", "GT", "LE", "AL", "NV",
]


def va_off(va: int) -> int:
    return va - BASE


def u32(data: bytes, va: int) -> int:
    return struct.unpack_from("<I", data, va_off(va))[0]


def u64(data: bytes, va: int) -> int:
    return struct.unpack_from("<Q", data, va_off(va))[0]


def cstr(data: bytes, va: int, maxlen: int = 64) -> str:
    off = va_off(va)
    end = data.find(b"\x00", off, off + maxlen)
    if end < 0:
        end = off + maxlen
    return data[off:end].decode("ascii", "replace")


def decode_movz_x8(w: int) -> int | None:
    """MOVZ/MOVN X8, #imm — returns the 16-bit immediate in X8, or None."""
    # MOVZ: sf=1 opc=10 100101 hw imm16 Rd
    if (w >> 23) == 0x1A5 and (w & 31) == 8:
        hw = (w >> 21) & 3
        imm = (w >> 5) & 0xFFFF
        return imm << (hw * 16)
    # MOVN (not observed for syscall NR, but keep symmetric)
    if (w >> 23) == 0x125 and (w & 31) == 8:
        hw = (w >> 21) & 3
        imm = (w >> 5) & 0xFFFF
        return imm << (hw * 16)
    return None


def parse_elf_sections(data: bytes) -> list[tuple[str, int, int, int, int]]:
    e_shoff = struct.unpack_from("<Q", data, 40)[0]
    e_shentsize = struct.unpack_from("<H", data, 58)[0]
    e_shnum = struct.unpack_from("<H", data, 60)[0]
    e_shstrndx = struct.unpack_from("<H", data, 62)[0]

    def sh(i: int) -> tuple:
        off = e_shoff + i * e_shentsize
        return struct.unpack_from("<IIQQQQIIQQ", data, off)

    strtab = sh(e_shstrndx)
    names = data[strtab[4] : strtab[4] + strtab[5]]
    out = []
    for i in range(e_shnum):
        s = sh(i)
        end = names.find(b"\x00", s[0])
        name = names[s[0] : end].decode("ascii", "replace") if end >= 0 else ""
        if name:
            # name, type, addr, offset, size
            out.append((name, s[1], s[3], s[4], s[5]))
    return out


def extract(bin_path: Path, out_dir: Path, inc_path: Path | None) -> None:
    data = bin_path.read_bytes()
    out_dir.mkdir(parents=True, exist_ok=True)

    # --- condition-code table DAT_800000012e1c ---
    cc = data[va_off(0x800000012E1C) : va_off(0x800000012E1C) + 16]
    (out_dir / "cc_table.hex").write_text(cc.hex(" ") + "\n")
    lines = ["x86_cc\tx86_name\tarm_cc\tarm_name\n"]
    for i, b in enumerate(cc):
        lines.append(f"{i}\t{X86_CC[i]}\t{b}\t{ARM_CC[b]}\n")
    (out_dir / "cc_table.tsv").write_text("".join(lines))

    # --- Operand.cpp condition_code_to_string PTR_DAT_800000013240 ---
    clines = ["index\tname\tstring_va\n"]
    for i in range(16):
        p = u64(data, 0x800000013240 + i * 8)
        clines.append(f"{i}\t{cstr(data, p)}\t{p:#x}\n")
    (out_dir / "cc_names.tsv").write_text("".join(clines))

    # --- Operand.cpp segment_register_to_string (stack literals in decomp) ---
    segs = [
        (0x8000000086FC, "ES"),
        (0x800000008D10, "CS"),
        (0x80000000B789, "SS"),
        (0x80000000DF00, "DS"),
        (0x800000007327, "FS"),
        (0x80000000C2EF, "GS"),
    ]
    slines = ["index\tname\tstring_va\n"]
    for i, (va, expect) in enumerate(segs):
        got = cstr(data, va)
        if got != expect:
            raise SystemExit(f"segment[{i}] at {va:#x} is {got!r}, expected {expect!r}")
        slines.append(f"{i}\t{got}\t{va:#x}\n")
    (out_dir / "segments.tsv").write_text("".join(slines))

    # --- Register.cpp register_to_string pointer tables ---
    tables = [
        (0x800000012EA0, 16, 0, "g8"),
        (0x800000012F20, 4, 1, "gh"),
        (0x800000012F40, 16, 2, "g16"),
        (0x800000012FC0, 16, 3, "g32"),
        (0x800000013040, 16, 4, "g64"),
        (0x8000000130C0, 16, 5, "xmm"),
        (0x800000013140, 16, 10, "ymm"),
        (0x8000000131C0, 16, 6, "mm_then_st"),  # mm0-7, st0-7
        (0x800000013200, 8, 7, "st"),
    ]
    rlines = ["class\tindex\tname\tstring_va\n"]
    rlines.append("8\t0\trip\t0x8000000060eb\n")
    for addr, n, cls, _tag in tables:
        for i in range(n):
            p = u64(data, addr + i * 8)
            rlines.append(f"{cls}\t{i}\t{cstr(data, p)}\t{p:#x}\n")
    (out_dir / "register_names.tsv").write_text("".join(rlines))

    # --- ELF sections ---
    shlines = ["name\ttype\tva\tfile_off\tsize\n"]
    for name, typ, addr, off, size in parse_elf_sections(data):
        shlines.append(f"{name}\t{typ:#x}\t{addr:#x}\t{off:#x}\t{size:#x}\n")
    (out_dir / "elf_sections.tsv").write_text("".join(shlines))

    # --- syscall wrappers: MOVZ X8,#N ; SVC #0 ; CMN X0,#0xfff ---
    slines = ["va\tnr\tname\tkind\n"]
    other = ["va\tnr\tname\tnext_insn\n"]
    wrappers: list[tuple[int, int, str, str]] = []
    text_start = 0x800000026000
    text_end = text_start + 0x6C2A0
    # Also scan runtime_* sections (savedreg has one extra clone-like wrapper).
    scan_end = 0x800000097498  # end of runtime_syscall
    va = text_start
    while va + 12 <= scan_end:
        w0 = u32(data, va)
        w1 = u32(data, va + 4)
        nr = decode_movz_x8(w0)
        if nr is not None and w1 == SVC_0:
            w2 = u32(data, va + 8)
            name = AARCH64_NR.get(nr, f"nr_{nr}")
            if w2 == CMN_X0_FFF:
                kind = "raw" if u32(data, va + 16) == RET else "result"
                slines.append(f"{va:#x}\t{nr}\t{name}\t{kind}\n")
                wrappers.append((va, nr, name, kind))
            else:
                other.append(f"{va:#x}\t{nr}\t{name}\t{w2:08x}\n")
            va += 4
            continue
        va += 4
    (out_dir / "syscalls.tsv").write_text("".join(slines))
    (out_dir / "other_svc.tsv").write_text("".join(other))

    uniq = sorted({nr for _, nr, _, _ in wrappers})
    summary = [
        "# Host (AArch64) syscalls used by RosettaLinux wrappers\n\n",
        f"Recovered `{len(wrappers)}` `movz x8,#N; svc #0; cmn x0,#0xfff` sites, "
        f"`{len(uniq)}` distinct numbers.\n\n",
        "Each number has a **raw** wrapper (saturate to -1) and a **result** "
        "wrapper (`{value, is_error}` in x0/x1), except extras in `runtime_*`.\n\n",
        "| nr | name |\n|----|------|\n",
    ]
    for nr in uniq:
        summary.append(f"| {nr} | {AARCH64_NR.get(nr, '?')} |\n")
    (out_dir / "syscalls.md").write_text("".join(summary))

    if inc_path is not None:
        inc_path.parent.mkdir(parents=True, exist_ok=True)
        body = [
            "/* Generated by tools/extract_from_binary.py — do not edit. */\n",
            "/* Host AArch64 syscall numbers recovered from MOVZ X8 immediates. */\n",
            "static const struct {\n",
            "    u16 nr;\n",
            "    const char *name;\n",
            "} oah_host_syscall_table[] = {\n",
        ]
        for nr in uniq:
            body.append(f'    {{ {nr}, "{AARCH64_NR.get(nr, f"nr_{nr}")}" }},\n')
        body.append("};\n")
        body.append(
            f"#define OAH_HOST_SYSCALL_COUNT "
            f"{len(uniq)}\n"
        )
        inc_path.write_text("".join(body))

    print(
        f"cc={list(cc)} wrappers={len(wrappers)} unique_nr={len(uniq)} -> {out_dir}"
    )


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", type=Path)
    ap.add_argument("-o", "--out", type=Path, required=True)
    ap.add_argument("--inc", type=Path, default=None,
                    help="Write C initializer (host_syscall_table.inc)")
    args = ap.parse_args()
    extract(args.binary, args.out, args.inc)


if __name__ == "__main__":
    main()
