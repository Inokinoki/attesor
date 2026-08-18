#!/usr/bin/env python3
"""Extract a source map from Ghidra decompilation of RosettaLinux.

Parses rosetta_decomp.c for:
  - Function definitions (address + signature)
  - Assertion call sites (file, line, function, condition, message)
  - String literals that look like original source paths
  - Unique original C++/header files

This is Layer 0 of the reconstruction: inventory with evidence, not guessed names.
"""

from __future__ import annotations

import argparse
import csv
import re
from collections import Counter, defaultdict
from pathlib import Path

FUN_DEF_RE = re.compile(
    r"^(?P<ret>\S.*?)\s+(?P<name>FUN_[0-9a-f]+|entry)\s*\(",
)
ASSERT5_RE = re.compile(
    r'FUN_800000089a1c\(\s*"([^"]+)"\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*"([^"]+)"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"',
)
ASSERT4_RE = re.compile(
    r'FUN_800000089aa0\(\s*"([^"]+)"\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*"([^"]+)"\s*,\s*"([^"]*)"',
)
# Ghidra sometimes splits assertions across lines; also the first arg can be param_4
ASSERT5_FLEX_RE = re.compile(
    r'FUN_800000089a1c\(\s*(?:param_\d+,)?\s*"([^"]+\.(?:h|hpp|cpp|c|cc))"\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*,\s*(?:&DAT_[0-9a-f]+|"([^"]+)")',
)
STRING_FILE_RE = re.compile(
    r'"((?:[A-Za-z][A-Za-z0-9_]+(?:/[A-Za-z0-9_]+)*)?\.(?:h|hpp|cpp|c|cc|hpp))"'
)
EMIT32_RE = re.compile(r"FUN_8000000278c4\(")


def parse_int(tok: str) -> int:
    tok = tok.strip()
    return int(tok, 16) if tok.startswith("0x") or tok.startswith("0X") else int(tok)


def extract(decomp_path: Path) -> dict:
    text = decomp_path.read_text(errors="replace")
    lines = text.splitlines()

    functions = []
    current = None
    for i, line in enumerate(lines, start=1):
        m = FUN_DEF_RE.match(line.strip()) if line.strip().endswith("(") or "(" in line.strip() else None
        # Function defs in this dump are typically:
        #   void FUN_8000000278c4(long *param_1,undefined4 param_2)
        m = re.match(
            r"^(?P<ret>[\w \*\[\]]+?)\s+(?P<name>FUN_[0-9a-f]+|entry)\s*\((?P<args>.*)\)\s*$",
            line,
        )
        if m and not line.startswith(" ") and not line.startswith("\t"):
            if current is not None:
                current["end_line"] = i - 1
                functions.append(current)
            name = m.group("name")
            addr = 0
            if name.startswith("FUN_"):
                addr = int(name[4:], 16)
            current = {
                "name": name,
                "addr": addr,
                "ret": " ".join(m.group("ret").split()),
                "args": m.group("args"),
                "start_line": i,
                "end_line": None,
                "asserts": [],
                "emit32_calls": 0,
            }
        if current is not None and "FUN_8000000278c4(" in line:
            current["emit32_calls"] += line.count("FUN_8000000278c4(")

    if current is not None:
        current["end_line"] = len(lines)
        functions.append(current)

    # Map line -> function
    line_to_fn = {}
    for fn in functions:
        for ln in range(fn["start_line"], (fn["end_line"] or fn["start_line"]) + 1):
            line_to_fn[ln] = fn

    assertions = []
    # Join the file into a single string for multi-line asserts, but keep line numbers
    # by scanning a sliding window of 6 lines.
    for i in range(len(lines)):
        window = " ".join(lines[i : i + 6])
        window = re.sub(r"\s+", " ", window)
        m5 = ASSERT5_RE.search(window)
        m4 = ASSERT4_RE.search(window)
        mflex = None if (m5 or m4) else ASSERT5_FLEX_RE.search(window)
        rec = None
        if m5:
            rec = {
                "kind": "assert",
                "file": m5.group(1),
                "line": parse_int(m5.group(2)),
                "function": m5.group(3),
                "condition": m5.group(4),
                "message": m5.group(5),
            }
        elif m4:
            rec = {
                "kind": "assert_msg",
                "file": m4.group(1),
                "line": parse_int(m4.group(2)),
                "function": m4.group(3),
                "condition": "",
                "message": m4.group(4),
            }
        elif mflex and mflex.group(3):
            rec = {
                "kind": "assert_partial",
                "file": mflex.group(1),
                "line": parse_int(mflex.group(2)),
                "function": mflex.group(3),
                "condition": "",
                "message": "",
            }
        if rec:
            src_line = i + 1
            fn = line_to_fn.get(src_line)
            rec["decomp_line"] = src_line
            rec["decomp_func"] = fn["name"] if fn else ""
            rec["decomp_addr"] = f"0x{fn['addr']:x}" if fn and fn["addr"] else ""
            # Dedup: Ghidra windows overlap
            key = (
                rec["file"],
                rec["line"],
                rec["function"],
                rec["decomp_func"],
                rec["kind"],
            )
            rec["_key"] = key
            assertions.append(rec)

    # Deduplicate overlapping window matches
    seen = set()
    uniq_asserts = []
    for rec in assertions:
        if rec["_key"] in seen:
            continue
        seen.add(rec["_key"])
        rec.pop("_key")
        uniq_asserts.append(rec)
        if rec["decomp_func"]:
            for fn in functions:
                if fn["name"] == rec["decomp_func"]:
                    fn["asserts"].append(rec)
                    break

    source_files = Counter()
    for rec in uniq_asserts:
        source_files[rec["file"]] += 1
    for m in STRING_FILE_RE.finditer(text):
        source_files[m.group(1)] += 0  # ensure present

    return {
        "functions": functions,
        "assertions": uniq_asserts,
        "source_files": source_files,
    }


def write_outputs(data: dict, out_dir: Path) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)

    src_path = out_dir / "source_files.tsv"
    with src_path.open("w") as f:
        f.write("file\tassert_sites\n")
        for name, count in sorted(data["source_files"].items(), key=lambda kv: (-kv[1], kv[0])):
            f.write(f"{name}\t{count}\n")

    assert_path = out_dir / "assertions.tsv"
    fields = [
        "file",
        "line",
        "function",
        "condition",
        "message",
        "kind",
        "decomp_func",
        "decomp_addr",
        "decomp_line",
    ]
    with assert_path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t")
        w.writeheader()
        for rec in data["assertions"]:
            w.writerow(rec)

    fn_path = out_dir / "functions.tsv"
    with fn_path.open("w") as f:
        f.write(
            "addr\tname\tstart_line\tend_line\tsize_lines\temit32_calls\tnamed_from_assert\tassert_functions\tret\n"
        )
        for fn in data["functions"]:
            names = sorted({a["function"] for a in fn["asserts"] if a.get("function")})
            f.write(
                "0x{addr:x}\t{name}\t{start}\t{end}\t{size}\t{emit}\t{named}\t{af}\t{ret}\n".format(
                    addr=fn["addr"],
                    name=fn["name"],
                    start=fn["start_line"],
                    end=fn["end_line"] or "",
                    size=(fn["end_line"] or fn["start_line"]) - fn["start_line"] + 1,
                    emit=fn["emit32_calls"],
                    named="yes" if names else "no",
                    af=",".join(names),
                    ret=fn["ret"],
                )
            )

    named_path = out_dir / "named_functions.tsv"
    with named_path.open("w") as f:
        f.write("addr\tdecomp_func\toriginal_function\toriginal_file\toriginal_line\tevidence\n")
        for fn in data["functions"]:
            if not fn["asserts"]:
                continue
            # Pick the most common asserted function name inside this FUN_*
            counts = Counter(a["function"] for a in fn["asserts"] if a.get("function"))
            if not counts:
                continue
            original, _ = counts.most_common(1)[0]
            files = Counter(a["file"] for a in fn["asserts"])
            lines = Counter(a["line"] for a in fn["asserts"] if a["function"] == original)
            orig_file = files.most_common(1)[0][0]
            orig_line = lines.most_common(1)[0][0] if lines else 0
            f.write(
                f"0x{fn['addr']:x}\t{fn['name']}\t{original}\t{orig_file}\t{orig_line}\t"
                f"{len(fn['asserts'])} assert site(s)\n"
            )

    # Functions-by-original-file grouping
    by_file = defaultdict(list)
    for rec in data["assertions"]:
        by_file[rec["file"]].append(rec)
    grouped = out_dir / "by_original_file.md"
    with grouped.open("w") as f:
        f.write("# Original source files recovered from RosettaLinux decompilation\n\n")
        f.write(
            "Evidence is assertion strings embedded in the binary "
            "(`file`, `line`, `function`, `condition`).\n\n"
        )
        f.write(f"- Unique original files: **{len(data['source_files'])}**\n")
        f.write(f"- Assertion sites: **{len(data['assertions'])}**\n")
        f.write(f"- Decompiled functions: **{len(data['functions'])}**\n")
        named = sum(1 for fn in data["functions"] if fn["asserts"])
        f.write(f"- Functions with at least one named assert: **{named}**\n\n")
        for fname in sorted(by_file, key=lambda n: (-len(by_file[n]), n)):
            recs = by_file[fname]
            funcs = sorted({r["function"] for r in recs if r["function"]})
            f.write(f"## `{fname}` ({len(recs)} sites, {len(funcs)} functions)\n\n")
            for func in funcs:
                lines = sorted({r["line"] for r in recs if r["function"] == func})
                decomp = sorted(
                    {
                        f"{r['decomp_func']} @ {r['decomp_addr']}"
                        for r in recs
                        if r["function"] == func and r["decomp_func"]
                    }
                )
                f.write(f"- `{func}` lines {lines}\n")
                for d in decomp:
                    f.write(f"  - decomp: `{d}`\n")
            f.write("\n")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("decomp", type=Path)
    ap.add_argument("-o", "--out", type=Path, required=True)
    args = ap.parse_args()
    data = extract(args.decomp)
    write_outputs(data, args.out)
    print(
        f"functions={len(data['functions'])} asserts={len(data['assertions'])} "
        f"files={len(data['source_files'])} -> {args.out}"
    )


if __name__ == "__main__":
    main()
