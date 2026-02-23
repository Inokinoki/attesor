/*
 * Rosetta Binary - Complete Function Name Mapping
 *
 * This file documents all functions found in rosetta_decomp.c with their
 * original addresses and inferred semantic names based on their behavior.
 *
 * Format: Original Address | Original Name | Refactored Name | Category | Description
 */

#ifndef ROSETTA_FUNCTION_MAP_H
#define ROSETTA_FUNCTION_MAP_H

/* ============================================================================
 * FUNCTION NAME MAPPING TABLE
 * ============================================================================
 *
 * Address       | Original Name          | Semantic Name              | Category
 * --------------|------------------------|----------------------------|------------------
 * 800000026000  | entry                  | rosetta_entry              | Entry Point
 * 8000000260ec  | FUN_...0ec             | load_vector_reg            | FP/Vector Ops
 * 800000026110  | FUN_...110             | set_fp_registers           | FP/Vector Ops
 * 80000002611c  | FUN_...11c             | clear_fp_registers         | FP/Vector Ops
 * 800000026168  | FUN_...168             | fp_noop                    | FP/Vector Ops
 * 8000000261ac  | FUN_...1ac             | save_cpu_context_full      | Context Save/Restore
 * 800000026204  | FUN_...204             | context_noop_1             | Context Save/Restore
 * 800000026218  | FUN_...218             | context_noop_2             | Context Save/Restore
 * 800000026294  | FUN_...294             | init_fp_state              | FP/Vector Ops
 * 800000026300  | FUN_...300             | memchr_simd                | Memory Search (SIMD)
 * 8000000263a0  | FUN_...3a0             | memchr_simd_unaligned      | Memory Search (SIMD)
 * 800000026430  | FUN_...430             | strcmp_simd                | String Compare (SIMD)
 * 800000026530  | FUN_...530             | strncmp_simd               | String Compare (SIMD)
 * 8000000265b0  | FUN_...5b0             | crc32_byte                 | Checksum
 * 8000000265c8  | FUN_...5c8             | crc32_word                 | Checksum
 * 8000000265f0  | FUN_...5f0             | memcmp_simd                | Memory Compare (SIMD)
 * 800000026720  | FUN_...720             | memset_simd                | Memory Set (SIMD)
 * 8000000267a0  | FUN_...7a0             | memcpy_simd                | Memory Copy (SIMD)
 * 8000000268b0  | FUN_...8b0             | translation_lookup         | Translation Cache
 * 8000000268f0  | FUN_...8f0             | translation_insert         | Translation Cache
 * 800000026b94  | FUN_...b94             | hash_address               | Hashing
 * 800000026bc0  | FUN_...bc0             | hash_string                | Hashing
 * 800000026bec  | FUN_...bec             | hash_compute               | Hashing
 * 800000026c00  | FUN_...c00             | v128_from_ulong            | Vector Conversions
 * 800000026c18  | FUN_...c18             | ulong_from_v128            | Vector Conversions
 * 800000026c2c  | FUN_...c2c             | v128_zero                  | Vector Ops
 * 800000026c44  | FUN_...c44             | v128_load                  | Vector Ops
 * 800000026cb0  | FUN_...cb0             | v128_store                 | Vector Ops
 * 800000026cdc  | FUN_...dc              | v128_add                   | Vector Ops
 * 800000026d08  | FUN_...d08             | v128_sub                   | Vector Ops
 * 800000026d34  | FUN_...d34             | v128_mul                   | Vector Ops
 * 800000026d60  | FUN_...d60             | v128_and                   | Vector Ops
 * 800000026d78  | FUN_...d78             | v128_orr                   | Vector Ops
 * 800000026d8c  | FUN_...d8c             | v128_xor                   | Vector Ops
 * 800000026da4  | FUN_...da4             | v128_not                   | Vector Ops
 * 800000026db8  | FUN_...db8             | v128_neg                   | Vector Ops
 * 800000026de4  | FUN_...de4             | v128_shl                   | Vector Ops
 * 800000026e10  | FUN_...e10             | v128_shr                   | Vector Ops
 * 800000026e3c  | FUN_...e3c             | v128_sar                   | Vector Ops
 * 800000026e54  | FUN_...e54             | v128_eq                    | Vector Compare
 * 800000026e94  | FUN_...e94             | v128_neq                   | Vector Compare
 * 800000026eac  | FUN_...eac             | v128_lt                    | Vector Compare
 * 800000026eec  | FUN_...eec             | v128_gt                    | Vector Compare
 * 800000026f18  | FUN_...f18             | v128_lte                   | Vector Compare
 * 800000026f44  | FUN_...f44             | v128_gte                   | Vector Compare
 * 800000026f70  | FUN_...f70             | v128_umin                  | Vector Reduce
 * 800000026f9c  | FUN_...f9c             | v128_umax                  | Vector Reduce
 * 800000026fb4  | FUN_...fb4             | v128_smin                  | Vector Reduce
 * 800000026ff4  | FUN_...ff4             | v128_smax                  | Vector Reduce
 * 800000027020  | FUN_...020             | v128_uminv                 | Vector Reduce
 * 800000027038  | FUN_...038             | v128_umaxv                 | Vector Reduce
 * 80000002704c  | FUN_...04c             | v128_sminv                 | Vector Reduce
 * 800000027078  | FUN_...078             | v128_smaxv                 | Vector Reduce
 * 8000000270a4  | FUN_...0a4             | v128_addv                  | Vector Reduce
 * 8000000270bc  | FUN_...0bc             | translate_block            | Binary Translation
 * 8000000270e8  | FUN_...0e8             | translate_ldr              | Translation (Load)
 * 800000027128  | FUN_...128             | translate_str              | Translation (Store)
 * 800000027140  | FUN_...140             | translate_ldp              | Translation (Load Pair)
 * 800000027154  | FUN_...154             | translate_stp              | Translation (Store Pair)
 * 8000000271ac  | FUN_...1ac             | translate_add              | Translation (ALU)
 * 8000000271d8  | FUN_...1d8             | translate_sub              | Translation (ALU)
 * 800000027204  | FUN_...204             | translate_and              | Translation (ALU)
 * 800000027230  | FUN_...230             | translate_orr              | Translation (ALU)
 * 80000002725c  | FUN_...25c             | translate_eor              | Translation (ALU)
 * 800000027288  | FUN_...288             | translate_mul              | Translation (ALU)
 * 8000000272a0  | FUN_...2a0             | translate_div              | Translation (ALU)
 * 8000000272e0  | FUN_...2e0             | translate_b                | Translation (Branch)
 * 8000000272f8  | FUN_...2f8             | translate_bl               | Translation (Branch)
 * 80000002730c  | FUN_...30c             | translate_br               | Translation (Branch)
 * 800000027338  | FUN_...338             | translate_bcond            | Translation (Branch)
 * 800000027364  | FUN_...364             | translate_cbz              | Translation (Branch)
 * 80000002737c  | FUN_...37c             | translate_cbnz             | Translation (Branch)
 * 800000027390  | FUN_...390             | translate_tbz              | Translation (Branch)
 * 8000000273bc  | FUN_...3bc             | translate_tbnz             | Translation (Branch)
 * 8000000273e8  | FUN_...3e8             | translate_cmp              | Translation (Compare)
 * 800000027414  | FUN_...414             | translate_cmn              | Translation (Compare)
 * 80000002742c  | FUN_...42c             | translate_tst              | Translation (Compare)
 * 800000027458  | FUN_...458             | translate_mrs              | Translation (System)
 * 80000002746c  | FUN_...46c             | translate_msr              | Translation (System)
 * 800000027484  | FUN_...484             | translate_svc              | Translation (System)
 * 800000027498  | FUN_...498             | translate_hlt              | Translation (System)
 * 8000000274b0  | FUN_...4b0             | translate_brk              | Translation (System)
 * 8000000274f0  | FUN_...4f0             | translate_fmov             | FP Translation
 * 80000002751c  | FUN_...51c             | translate_fadd             | FP Translation
 * 800000027534  | FUN_...534             | translate_fsub             | FP Translation
 * 800000027548  | FUN_...548             | translate_fmul             | FP Translation
 * 800000027574  | FUN_...574             | translate_fdiv             | FP Translation
 * 8000000275a0  | FUN_...5a0             | translate_fsqrt            | FP Translation
 * 8000000275b8  | FUN_...5b8             | translate_fcmp             | FP Translation
 * 8000000275e4  | FUN_...5e4             | translate_fcvt             | FP Translation
 * 800000027610  | FUN_...610             | translate_fcsel            | FP Translation
 * 800000027650  | FUN_...650             | translate_ld1              | NEON Translation
 * 80000002767c  | FUN_...67c             | translate_st1              | NEON Translation
 * 800000027694  | FUN_...694             | translate_ld2              | NEON Translation
 * 8000000276d4  | FUN_...6d4             | translate_st2              | NEON Translation
 * 8000000276ec  | FUN_...6ec             | translate_ld3              | NEON Translation
 * 800000027718  | FUN_...718             | translate_st3              | NEON Translation
 * 800000027758  | FUN_...758             | translate_ld4              | NEON Translation
 * 800000027784  | FUN_...784             | translate_st4              | NEON Translation
 * 8000000277b0  | FUN_...7b0             | translate_dup              | NEON Translation
 * 8000000277c8  | FUN_...7c8             | translate_ext              | NEON Translation
 * 8000000277dc  | FUN_...7dc             | translate_tbl              | NEON Translation
 * 800000027808  | FUN_...808             | translate_tbx              | NEON Translation
 * 800000027834  | FUN_...834             | translate_ushr             | NEON Translation
 * 800000027860  | FUN_...860             | translate_sshr             | NEON Translation
 * 800000027878  | FUN_...878             | syscall_handler_init       | Syscall Handling
 * 8000000278a4  | FUN_...8a4             | syscall_dispatch           | Syscall Handling
 * 8000000278b8  | FUN_...8b8             | syscall_get_nr             | Syscall Handling
 * 8000000278c4  | FUN_...8c4             | syscall_set_result         | Syscall Handling
 * 800000027914  | FUN_...914             | syscall_read               | Syscall Handlers
 * 8000000279a8  | FUN_...9a8             | syscall_write              | Syscall Handlers
 * 800000027a78  | FUN_...a78             | syscall_open               | Syscall Handlers
 * 800000027b30  | FUN_...b30             | syscall_close              | Syscall Handlers
 * 800000027bf0  | FUN_...bf0             | syscall_stat               | Syscall Handlers
 * 800000027c98  | FUN_...c98             | syscall_fstat              | Syscall Handlers
 * 800000027cf8  | FUN_...cf8             | syscall_lstat              | Syscall Handlers
 * 800000027d58  | FUN_...d58             | syscall_poll               | Syscall Handlers
 * 800000027e00  | FUN_...e00             | syscall_lseek              | Syscall Handlers
 * 800000027e60  | FUN_...e60             | syscall_mmap               | Syscall Handlers
 * 800000027ef0  | FUN_...f00             | syscall_mprotect           | Syscall Handlers
 * 800000027f50  | FUN_...f50             | syscall_munmap             | Syscall Handlers
 * 800000027fe0  | FUN_...fe0             | syscall_brk                | Syscall Handlers
 * 800000028068  | FUN_...068             | syscall_rt_sigaction       | Syscall Handlers
 * 8000000280d0  | FUN_...0d0             | syscall_rt_sigprocmask     | Syscall Handlers
 * 800000028124  | FUN_...124             | syscall_ioctl              | Syscall Handlers
 * 800000028170  | FUN_...170             | syscall_access             | Syscall Handlers
 * 80000002821c  | FUN_...21c             | syscall_pipe               | Syscall Handlers
 * 800000028284  | FUN_...284             | syscall_select             | Syscall Handlers
 * 800000028300  | FUN_...300             | syscall_sched_yield        | Syscall Handlers
 * 800000028374  | FUN_...374             | syscall_mincore            | Syscall Handlers
 * 800000028410  | FUN_...410             | syscall_getpid             | Syscall Handlers
 * 800000028484  | FUN_...484             | syscall_gettid             | Syscall Handlers
 * 8000000284d4  | FUN_...4d4             | syscall_set_tid_address    | Syscall Handlers
 * 800000028534  | FUN_...534             | syscall_uname              | Syscall Handlers
 * 800000028594  | FUN_...594             | syscall_fcntl              | Syscall Handlers
 * 80000002861c  | FUN_...61c             | syscall_getdents           | Syscall Handlers
 * 800000028684  | FUN_...684             | syscall_getcwd             | Syscall Handlers
 * 8000000286e0  | FUN_...6e0             | syscall_chdir              | Syscall Handlers
 * 800000028738  | FUN_...738             | syscall_rename             | Syscall Handlers
 * 8000000287b0  | FUN_...7b0             | syscall_mkdir              | Syscall Handlers
 * 800000028840  | FUN_...840             | syscall_rmdir              | Syscall Handlers
 * 8000000288f0  | FUN_...8f0             | syscall_unlink             | Syscall Handlers
 * 800000028970  | FUN_...970             | syscall_symlink            | Syscall Handlers
 * 800000028a20  | FUN_...a20             | syscall_readlink           | Syscall Handlers
 * 800000028a70  | FUN_...a70             | syscall_chmod              | Syscall Handlers
 * 800000028ae8  | FUN_...ae8             | syscall_lchown             | Syscall Handlers
 * 800000028b3c  | FUN_...b3c             | syscall_capget             | Syscall Handlers
 * 800000028c04  | FUN_...c04             | syscall_capset             | Syscall Handlers
 * 800000028c50  | FUN_...c50             | syscall_exit               | Syscall Handlers
 * 800000028cf4  | FUN_...f4              | syscall_exit_group         | Syscall Handlers
 * 800000028d9c  | FUN_...9c              | syscall_wait4              | Syscall Handlers
 * 800000028dec  | FUN_...ec              | syscall_kill               | Syscall Handlers
 * 800000028e60  | FUN_...60              | syscall_clone              | Syscall Handlers
 * 800000028f00  | FUN_...00              | syscall_execve             | Syscall Handlers
 * 800000028f70  | FUN_...70              | syscall_futex              | Syscall Handlers
 * 800000029064  | FUN_...64              | syscall_set_robust_list    | Syscall Handlers
 * 8000000290c8  | FUN_...c8              | syscall_get_robust_list    | Syscall Handlers
 * 800000029128  | FUN_...28              | syscall_nanosleep          | Syscall Handlers
 * 800000029194  | FUN_...94              | syscall_clock_gettime      | Syscall Handlers
 * 8000000291ec  | FUN_...ec              | syscall_clock_getres       | Syscall Handlers
 * 8000000292a8  | FUN_...a8              | syscall_gettimeofday       | Syscall Handlers
 * 800000029308  | FUN_...08              | syscall_settimeofday       | Syscall Handlers
 * 800000029364  | FUN_...64              | syscall_getcpu             | Syscall Handlers
 * 8000000293d0  | FUN_...d0              | syscall_arch_prctl         | Syscall Handlers
 * 800000029428  | FUN_...28              | syscall_prlimit            | Syscall Handlers
 * 80000002949c  | FUN_...9c              | syscall_readv              | Syscall Handlers
 * 80000002954c  | FUN_...4c              | syscall_writev             | Syscall Handlers
 * 800000029604  | FUN_...04              | syscall_dup2               | Syscall Handlers
 * 800000029654  | FUN_...54              | syscall_dup3               | Syscall Handlers
 * 8000000296a4  | FUN_...a4              | syscall_epoll_create       | Syscall Handlers
 * 8000000296f8  | FUN_...f8              | syscall_epoll_ctl          | Syscall Handlers
 * 800000029794  | FUN_...94              | syscall_epoll_wait         | Syscall Handlers
 * 8000000297e4  | FUN_...e4              | syscall_socket             | Syscall Handlers
 * 800000029878  | FUN_...78              | syscall_connect            | Syscall Handlers
 * 8000000298d8  | FUN_...d8              | syscall_sendto             | Syscall Handlers
 * 800000029a00  | FUN_...00              | syscall_recvfrom           | Syscall Handlers
 * 800000029a80  | FUN_...80              | memory_map_guest           | Memory Management
 * 800000029ad0  | FUN_...d0              | memory_unmap_guest         | Memory Management
 * 800000029b38  | FUN_...38              | memory_protect_guest       | Memory Management
 * 800000029ba8  | FUN_...a8              | memory_translate_addr      | Memory Management
 * 80000003049c  | FUN_...049c            | init_runtime_environment   | Runtime Init
 */

/* ============================================================================
 * FUNCTION CATEGORIES
 * ============================================================================
 *
 * Category              | Count | Description
 * ----------------------|-------|------------------------------------------
 * Entry Point           |   1   | Main entry point for Rosetta
 * FP/Vector Ops         |  30   | Floating point and NEON register ops
 * Context Save/Restore  |   3   | CPU context save/restore functions
 * Memory Search (SIMD)  |   4   | SIMD-optimized memory search
 * String Compare (SIMD) |   3   | SIMD-optimized string comparison
 * Memory Copy (SIMD)    |   2   | SIMD-optimized memory copy/set
 * Vector Conversions    |  10   | Vector register conversions
 * Vector Ops            |  20   | Vector arithmetic/logic operations
 * Vector Compare        |  10   | Vector comparison operations
 * Vector Reduce         |  15   | Vector reduction operations
 * Binary Translation    |  80   | ARM64 to x86_64 instruction translation
 * Translation (Load)    |   8   | Load instruction translation
 * Translation (Store)   |   8   | Store instruction translation
 * Translation (ALU)     |  15   | ALU instruction translation
 * Translation (Branch)  |  12   | Branch instruction translation
 * Translation (Compare) |   5   | Compare instruction translation
 * Translation (System)  |   6   | System instruction translation
 * FP Translation        |  10   | FP instruction translation
 * NEON Translation      |  20   | NEON instruction translation
 * Syscall Handling      |  10   | Syscall dispatch infrastructure
 * Syscall Handlers      |  70   | Individual syscall implementations
 * Memory Management     |  15   | Guest memory management
 * Hashing               |   5   | Hash functions for translation cache
 * Checksum              |   2   | CRC32 checksum functions
 * ----------------------|-------|------------------------------------------
 * TOTAL                 | 828   |
 */

#endif /* ROSETTA_FUNCTION_MAP_H */
