#!/usr/bin/env python3
"""Write a tiny static x86_64 ET_EXEC that write(1,"hello\\n") then exit(0).

First PT_LOAD is at file offset 0 (ElfMapper.cpp:399).
"""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

EHDR_SIZE = 64
PHDR_SIZE = 56
BASE = 0x400000
MSG = b"hello\n"


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("out", type=Path)
    args = ap.parse_args()

    # Headers then code then message. Code starts immediately after 1 PHDR.
    code_off = EHDR_SIZE + PHDR_SIZE
    # lea rsi, [rip+disp] is 7 bytes; disp is from the insn after LEA.
    #
    #   mov eax, 1
    #   mov edi, 1
    #   lea rsi, [rip+rel]
    #   mov edx, len
    #   syscall
    #   mov eax, 60
    #   xor edi, edi
    #   syscall
    #   msg
    pre_lea = bytes.fromhex("b801000000 bf01000000")  # 10 bytes
    lea = b"\x48\x8d\x35" + b"\x00\x00\x00\x00"  # 7, disp patched
    post_lea = bytes.fromhex("ba06000000 0f05 b83c000000 31ff 0f05")
    after_lea = len(post_lea)
    disp = after_lea  # msg sits right after post_lea
    lea = b"\x48\x8d\x35" + struct.pack("<i", disp)
    code = pre_lea + lea + post_lea + MSG
    file_size = code_off + len(code)
    entry = BASE + code_off

    eh = struct.pack(
        "<16sHHIQQQIHHHHHH",
        b"\x7fELF\x02\x01\x01" + b"\x00" * 9,
        2,  # ET_EXEC
        62,  # EM_X86_64
        1,
        entry,
        64,  # e_phoff
        0,
        0,
        EHDR_SIZE,
        PHDR_SIZE,
        1,  # e_phnum
        0,
        0,
        0,
    )
    assert len(eh) == EHDR_SIZE

    ph = struct.pack(
        "<IIQQQQQQ",
        1,  # PT_LOAD
        5,  # PF_R|PF_X
        0,  # p_offset
        BASE,
        BASE,
        file_size,
        file_size,
        0x1000,
    )
    assert len(ph) == PHDR_SIZE

    args.out.write_bytes(eh + ph + code)


if __name__ == "__main__":
    main()
