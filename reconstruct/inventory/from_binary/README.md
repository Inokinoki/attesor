# Tables copied from the RosettaLinux ELF

Regenerate (binary is gitignored):

```
make -C reconstruct inventory-binary
```

| File | Source in the ELF |
|------|-------------------|
| `cc_table.*` | `DAT_800000012e1c` (16 bytes) |
| `cc_names.tsv` | `PTR_DAT_800000013240` (`condition_code_to_string`) |
| `register_names.tsv` | `FUN_80000008d3d0` pointer tables |
| `segments.tsv` | `Operand.cpp` `segment_register_to_string` |
| `syscalls.tsv` | `movz x8,#N; svc #0; cmn x0,#0xfff` |
| `other_svc.tsv` | other `movz x8` + `svc #0` sites (getpid store, `runtime_*`) |
| `elf_sections.tsv` | section headers (`runtime_syscall`, `runtime_sigtramp`, …) |

The C copies of these tables live in `src/` so tests pass without the ELF.
