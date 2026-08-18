# Original source files recovered from RosettaLinux decompilation

Evidence is assertion strings embedded in the binary (`file`, `line`, `function`, `condition`).

- Unique original files: **88**
- Assertion sites: **974**
- Decompiled functions: **727**
- Functions with at least one named assert: **250**

## `Translator.h` (137 sites, 10 functions)

- `operand_size_to_data_size` lines [553]
  - decomp: `FUN_800000055874 @ 0x800000055874`
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_80000008319c @ 0x80000008319c`
  - decomp: `FUN_80000008599c @ 0x80000008599c`
  - decomp: `FUN_8000000860a8 @ 0x8000000860a8`
- `operand_size_to_mem_size` lines [562]
  - decomp: `FUN_800000039528 @ 0x800000039528`
  - decomp: `FUN_800000056040 @ 0x800000056040`
  - decomp: `FUN_800000056408 @ 0x800000056408`
  - decomp: `FUN_800000056608 @ 0x800000056608`
  - decomp: `FUN_800000056a60 @ 0x800000056a60`
  - decomp: `FUN_800000056be8 @ 0x800000056be8`
  - decomp: `FUN_800000056dd4 @ 0x800000056dd4`
  - decomp: `FUN_800000079a98 @ 0x800000079a98`
  - decomp: `FUN_80000008599c @ 0x80000008599c`
  - decomp: `FUN_800000086a60 @ 0x800000086a60`
  - decomp: `FUN_800000087424 @ 0x800000087424`
  - decomp: `FUN_8000000876f0 @ 0x8000000876f0`
  - decomp: `FUN_800000087a7c @ 0x800000087a7c`
  - decomp: `FUN_80000008864c @ 0x80000008864c`
  - decomp: `FUN_800000088994 @ 0x800000088994`
- `operator[]` lines [130, 134]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_800000079d9c @ 0x800000079d9c`
  - decomp: `FUN_80000007a09c @ 0x80000007a09c`
  - decomp: `FUN_80000007a3a8 @ 0x80000007a3a8`
  - decomp: `FUN_80000007a6bc @ 0x80000007a6bc`
  - decomp: `FUN_80000007b2c4 @ 0x80000007b2c4`
  - decomp: `FUN_80000007bec4 @ 0x80000007bec4`
  - decomp: `FUN_80000007cb74 @ 0x80000007cb74`
  - decomp: `FUN_80000007cd58 @ 0x80000007cd58`
  - decomp: `FUN_80000007cf44 @ 0x80000007cf44`
  - decomp: `FUN_80000007e290 @ 0x80000007e290`
  - decomp: `FUN_80000007e3a4 @ 0x80000007e3a4`
  - decomp: `FUN_80000007e62c @ 0x80000007e62c`
  - decomp: `FUN_80000007e8b4 @ 0x80000007e8b4`
  - decomp: `FUN_80000007ea24 @ 0x80000007ea24`
  - decomp: `FUN_80000007eb94 @ 0x80000007eb94`
  - decomp: `FUN_80000007ed04 @ 0x80000007ed04`
  - decomp: `FUN_80000007ef48 @ 0x80000007ef48`
  - decomp: `FUN_80000007f0b8 @ 0x80000007f0b8`
  - decomp: `FUN_80000007f228 @ 0x80000007f228`
  - decomp: `FUN_80000007f404 @ 0x80000007f404`
  - decomp: `FUN_80000007f688 @ 0x80000007f688`
  - decomp: `FUN_80000007f868 @ 0x80000007f868`
  - decomp: `FUN_80000007fa48 @ 0x80000007fa48`
  - decomp: `FUN_80000007fc28 @ 0x80000007fc28`
  - decomp: `FUN_80000007fe08 @ 0x80000007fe08`
  - decomp: `FUN_800000080040 @ 0x800000080040`
  - decomp: `FUN_8000000801ec @ 0x8000000801ec`
  - decomp: `FUN_800000080424 @ 0x800000080424`
  - decomp: `FUN_8000000805d0 @ 0x8000000805d0`
  - decomp: `FUN_800000080740 @ 0x800000080740`
  - decomp: `FUN_800000080950 @ 0x800000080950`
  - decomp: `FUN_800000080ac0 @ 0x800000080ac0`
  - decomp: `FUN_800000080c30 @ 0x800000080c30`
  - decomp: `FUN_800000080da0 @ 0x800000080da0`
  - decomp: `FUN_800000080fe8 @ 0x800000080fe8`
  - decomp: `FUN_8000000811e0 @ 0x8000000811e0`
  - decomp: `FUN_800000081348 @ 0x800000081348`
  - decomp: `FUN_800000081724 @ 0x800000081724`
  - decomp: `FUN_800000081b00 @ 0x800000081b00`
  - decomp: `FUN_800000081d5c @ 0x800000081d5c`
  - decomp: `FUN_8000000822c8 @ 0x8000000822c8`
  - decomp: `FUN_8000000823b4 @ 0x8000000823b4`
  - decomp: `FUN_8000000824bc @ 0x8000000824bc`
  - decomp: `FUN_8000000828b8 @ 0x8000000828b8`
  - decomp: `FUN_800000082afc @ 0x800000082afc`
  - decomp: `FUN_800000087154 @ 0x800000087154`
  - decomp: `FUN_800000087a7c @ 0x800000087a7c`
  - decomp: `FUN_8000000884a8 @ 0x8000000884a8`
- `translate_avx_low` lines [735]
  - decomp: `FUN_800000081d10 @ 0x800000081d10`
- `translate_condition_code` lines [612, 614]
  - decomp: `FUN_800000057bc8 @ 0x800000057bc8`
- `translate_gpr` lines [567]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_80000008319c @ 0x80000008319c`
  - decomp: `FUN_80000008599c @ 0x80000008599c`
  - decomp: `FUN_8000000860a8 @ 0x8000000860a8`
  - decomp: `FUN_800000086edc @ 0x800000086edc`
  - decomp: `FUN_8000000880f4 @ 0x8000000880f4`
  - decomp: `FUN_80000008888c @ 0x80000008888c`
- `translate_vector_low` lines [741]
  - decomp: `FUN_800000055bac @ 0x800000055bac`
- `translate_vector_or_mm_low` lines [751]
  - decomp: `FUN_800000081ca0 @ 0x800000081ca0`
- `translate_xmm` lines [573]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_800000087424 @ 0x800000087424`
  - decomp: `FUN_8000000881b0 @ 0x8000000881b0`
- `translate_xmm_or_mm` lines [579]
  - decomp: `FUN_800000079960 @ 0x800000079960`

## `GDBServer.cpp` (58 sites, 16 functions)

- `GDBServer` lines [151, 186]
  - decomp: `FUN_800000047700 @ 0x800000047700`
- `add_pending_watchpoint` lines [921]
  - decomp: `FUN_800000048924 @ 0x800000048924`
- `check_if_thread_in_runtime` lines [334]
  - decomp: `FUN_800000047ce8 @ 0x800000047ce8`
- `compute_wp_start_addr` lines [950]
  - decomp: `FUN_800000048ac0 @ 0x800000048ac0`
- `convert_gdb_to_sys_signal` lines [294, 299]
  - decomp: `FUN_800000047bf0 @ 0x800000047bf0`
- `convert_to_gdb_signal` lines [234]
  - decomp: `FUN_800000047b84 @ 0x800000047b84`
- `halt_all_threads_except` lines [823, 828]
  - decomp: `FUN_800000048754 @ 0x800000048754`
- `handle_reportable_signal` lines [1098]
  - decomp: `FUN_800000048ac0 @ 0x800000048ac0`
- `handle_signal_stop_requested` lines [1033, 1053]
  - decomp: `FUN_800000048ac0 @ 0x800000048ac0`
- `handle_sigtrap_and_continue` lines [673, 709]
  - decomp: `FUN_800000048ac0 @ 0x800000048ac0`
- `handle_watchpoint` lines [998, 1014]
  - decomp: `FUN_800000048ac0 @ 0x800000048ac0`
- `operator()` lines [378, 406, 457, 462, 492, 506]
  - decomp: `FUN_800000047de4 @ 0x800000047de4`
- `report_watchpoint` lines [745, 751, 781, 793]
  - decomp: `FUN_800000048348 @ 0x800000048348`
- `restore_entry_point_instruction` lines [59, 63]
  - decomp: `FUN_800000047700 @ 0x800000047700`
- `thread_main` lines [1410]
  - decomp: `FUN_8000000496b0 @ 0x8000000496b0`
- `write_hex_string` lines [1479]
  - decomp: `FUN_800000049734 @ 0x800000049734`

## `Register.h` (53 sites, 2 functions)

- `gpr_to_num` lines [51]
  - decomp: `FUN_8000000278c4 @ 0x8000000278c4`
  - decomp: `FUN_800000027cf8 @ 0x800000027cf8`
  - decomp: `FUN_8000000280d0 @ 0x8000000280d0`
  - decomp: `FUN_80000002821c @ 0x80000002821c`
  - decomp: `FUN_800000028410 @ 0x800000028410`
  - decomp: `FUN_8000000284d4 @ 0x8000000284d4`
  - decomp: `FUN_80000002861c @ 0x80000002861c`
  - decomp: `FUN_800000028684 @ 0x800000028684`
  - decomp: `FUN_8000000286e0 @ 0x8000000286e0`
  - decomp: `FUN_800000028738 @ 0x800000028738`
  - decomp: `FUN_800000028970 @ 0x800000028970`
  - decomp: `FUN_800000028c04 @ 0x800000028c04`
  - decomp: `FUN_800000028c50 @ 0x800000028c50`
  - decomp: `FUN_800000028f70 @ 0x800000028f70`
  - decomp: `FUN_8000000290c8 @ 0x8000000290c8`
  - decomp: `FUN_800000029308 @ 0x800000029308`
  - decomp: `FUN_800000029364 @ 0x800000029364`
  - decomp: `FUN_8000000293d0 @ 0x8000000293d0`
  - decomp: `FUN_800000029428 @ 0x800000029428`
  - decomp: `FUN_8000000296f8 @ 0x8000000296f8`
  - decomp: `FUN_8000000297e4 @ 0x8000000297e4`
  - decomp: `FUN_800000029878 @ 0x800000029878`
  - decomp: `FUN_8000000298d8 @ 0x8000000298d8`
  - decomp: `FUN_800000029a80 @ 0x800000029a80`
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_800000082f34 @ 0x800000082f34`
  - decomp: `FUN_800000082fc8 @ 0x800000082fc8`
  - decomp: `FUN_800000086a60 @ 0x800000086a60`
  - decomp: `FUN_80000008864c @ 0x80000008864c`
- `gpr_to_num_sp` lines [56]
  - decomp: `FUN_8000000278c4 @ 0x8000000278c4`
  - decomp: `FUN_800000027cf8 @ 0x800000027cf8`
  - decomp: `FUN_8000000280d0 @ 0x8000000280d0`
  - decomp: `FUN_800000028738 @ 0x800000028738`
  - decomp: `FUN_800000028970 @ 0x800000028970`
  - decomp: `FUN_800000028c50 @ 0x800000028c50`
  - decomp: `FUN_8000000290c8 @ 0x8000000290c8`
  - decomp: `FUN_8000000296f8 @ 0x8000000296f8`
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `PacketHandlers.cpp` (49 sites, 19 functions)

- `continue_all_threads` lines [1548, 1564, 1603, 1613]
  - decomp: `FUN_8000000466e8 @ 0x8000000466e8`
- `convert_arm_neon_state64_to_user_fpsimd` lines [196]
  - decomp: `FUN_800000043528 @ 0x800000043528`
- `disable_watchpoint` lines [2015]
  - decomp: `FUN_800000047284 @ 0x800000047284`
- `enable_watchpoint` lines [1939, 1941]
  - decomp: `FUN_800000046f20 @ 0x800000046f20`
- `get_arm_fpr_state` lines [233]
  - decomp: `FUN_8000000433fc @ 0x8000000433fc`
- `get_arm_gpr_state` lines [216]
  - decomp: `FUN_80000004336c @ 0x80000004336c`
- `handle_H_packet` lines [307]
  - decomp: `FUN_800000043608 @ 0x800000043608`
- `handle_M_packet` lines [1472]
  - decomp: `FUN_800000046328 @ 0x800000046328`
- `handle_P_packet` lines [1164, 1199]
  - decomp: `FUN_800000045810 @ 0x800000045810`
- `handle_g_packet` lines [831]
  - decomp: `FUN_800000043ae8 @ 0x800000043ae8`
- `handle_last_signal_packet` lines [344]
  - decomp: `FUN_8000000437c4 @ 0x8000000437c4`
- `handle_memory_read_packet` lines [1258]
  - decomp: `FUN_800000046064 @ 0x800000046064`
- `handle_p_packet` lines [864]
  - decomp: `FUN_800000044548 @ 0x800000044548`
- `handle_vCont_packet` lines [1722, 1724]
  - decomp: `FUN_800000046a90 @ 0x800000046a90`
- `write_fpr_register` lines [1004]
  - decomp: `FUN_800000045810 @ 0x800000045810`
- `write_gpr_register` lines [906]
  - decomp: `FUN_800000045810 @ 0x800000045810`
- `x86_fpr_to_string` lines [712]
  - decomp: `FUN_800000044548 @ 0x800000044548`
- `x86_gpr_to_string` lines [620]
  - decomp: `FUN_800000044548 @ 0x800000044548`
- `x86_regstate_to_string` lines [560]
  - decomp: `FUN_800000043ae8 @ 0x800000043ae8`

## `Translator.cpp` (41 sites, 19 functions)

- `segment_reg_to_thread_context_offset` lines [715]
  - decomp: `FUN_800000056040 @ 0x800000056040`
  - decomp: `FUN_800000056608 @ 0x800000056608`
  - decomp: `FUN_800000057c50 @ 0x800000057c50`
- `translate_call_single_step` lines [604]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_crc32` lines [4384]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_far_call` lines [755, 771]
  - decomp: `FUN_800000056608 @ 0x800000056608`
- `translate_far_jmp` lines [1173, 1189]
  - decomp: `FUN_800000056040 @ 0x800000056040`
- `translate_far_ret` lines [877, 910]
  - decomp: `FUN_800000056dd4 @ 0x800000056dd4`
- `translate_far_ret_immediate` lines [1009]
  - decomp: `FUN_800000057120 @ 0x800000057120`
- `translate_indirect_jmp_dyld_stub` lines [1110]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_instruction` lines [4325, 4351]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_jcc` lines [1350]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_jcc_single_step` lines [1374]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_jcxz` lines [1506]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_jcxz_single_step` lines [1529]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_jmp` lines [1039]
  - decomp: `FUN_800000055d58 @ 0x800000055d58`
- `translate_l_segment` lines [5004]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_loop` lines [1628, 1660, 1682]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_loop_single_step` lines [1723, 1771]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `translate_mov_segment` lines [4908]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_terminator_single_step` lines [528]
  - decomp: `FUN_800000039528 @ 0x800000039528`

## `ThreadContextRegisterState.cpp` (38 sites, 4 functions)

- `guest_gpr_state_from_host_state` lines [259, 335, 369, 414, 435, 443, 478, 488, 515, 541, 547]
  - decomp: `FUN_8000000407cc @ 0x8000000407cc`
- `host_fpr_state_from_guest_state` lines [1021, 1053, 1080, 1087]
  - decomp: `FUN_800000045810 @ 0x800000045810`
- `host_gpr_state_from_guest_state` lines [880, 910, 938, 944, 948, 962, 986, 989, 993]
  - decomp: `FUN_800000041e4c @ 0x800000041e4c`
- `move_to_instruction_boundary` lines [694]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`

## `TranslatorHelpers.cpp` (36 sites, 21 functions)

- `allocate_temporary_fpr_num` lines [165]
  - decomp: `FUN_800000085604 @ 0x800000085604`
- `allocate_temporary_gpr_num` lines [110]
  - decomp: `FUN_800000085460 @ 0x800000085460`
- `compute_operand_address` lines [1511, 1559]
  - decomp: `FUN_8000000860a8 @ 0x8000000860a8`
- `copy_xmm_state` lines [289]
  - decomp: `FUN_800000085704 @ 0x800000085704`
- `dst_gpr_for_operand` lines [1188]
  - decomp: `FUN_8000000880f4 @ 0x8000000880f4`
- `dst_gpr_or_xzr` lines [1207]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `free_temporary_gpr` lines [396]
  - decomp: `FUN_80000008593c @ 0x80000008593c`
- `read_and_invalidate_vector_dst` lines [993, 1018]
  - decomp: `FUN_800000087db0 @ 0x800000087db0`
- `read_gpr` lines [710, 732]
  - decomp: `FUN_800000086edc @ 0x800000086edc`
- `read_operand_gpr` lines [672]
  - decomp: `FUN_800000086a60 @ 0x800000086a60`
- `read_operand_vector` lines [775, 806]
  - decomp: `FUN_800000087154 @ 0x800000087154`
- `read_operand_xmm_helper` lines [1085]
  - decomp: `FUN_800000087424 @ 0x800000087424`
- `read_operand_xmm_or_mm_helper` lines [1138]
  - decomp: `FUN_8000000876f0 @ 0x8000000876f0`
- `reg_data_for_xmm_scalar_fp` lines [1241]
  - decomp: `FUN_8000000881b0 @ 0x8000000881b0`
- `set_xmm_state_internal` lines [224]
  - decomp: `FUN_8000000854c8 @ 0x8000000854c8`
- `translate_and_invalidate_vector_low` lines [979]
  - decomp: `FUN_800000087d58 @ 0x800000087d58`
- `translate_and_invalidate_xmm_dst` lines [1213]
  - decomp: `FUN_8000000881b0 @ 0x8000000881b0`
- `translate_and_invalidate_xmm_or_mm_dst` lines [1227]
  - decomp: `FUN_800000087efc @ 0x800000087efc`
- `write_non_reg_operand_fpr` lines [1441]
  - decomp: `FUN_800000088994 @ 0x800000088994`
- `write_non_reg_operand_vector` lines [973]
  - decomp: `FUN_800000087a7c @ 0x800000087a7c`
- `write_operand_gpr` lines [1374]
  - decomp: `FUN_80000008864c @ 0x80000008864c`

## `VMAllocationTracker.cpp` (33 sites, 11 functions)

- `allocate_node` lines [649]
  - decomp: `FUN_80000008b114 @ 0x80000008b114`
- `calculate_slab_allocator_size` lines [268, 284]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `change_permissions` lines [684]
  - decomp: `FUN_80000008ceec @ 0x80000008ceec`
- `cull_nodes_in_interval` lines [563]
  - decomp: `FUN_80000008c844 @ 0x80000008c844`
- `find_allocation_or_nearest_right` lines [82]
  - decomp: `FUN_80000008c844 @ 0x80000008c844`
  - decomp: `FUN_80000008cc7c @ 0x80000008cc7c`
- `find_free_space_internal` lines [240, 242]
  - decomp: `FUN_80000008afb0 @ 0x80000008afb0`
- `refill_slab_allocator` lines [659]
  - decomp: `FUN_80000008b114 @ 0x80000008b114`
- `remap` lines [705, 708, 711, 716]
  - decomp: `FUN_80000002f0f0 @ 0x80000002f0f0`
- `remove` lines [509]
  - decomp: `FUN_80000008cc7c @ 0x80000008cc7c`
- `remove_shared_mem` lines [735, 745]
  - decomp: `FUN_80000003007c @ 0x80000003007c`
- `remove_shm_mapping_entry` lines [670]
  - decomp: `FUN_80000008ce48 @ 0x80000008ce48`

## `Decoder.cpp` (28 sites, 10 functions)

- `decode_dependant_operand` lines [550, 553, 556]
  - decomp: `FUN_80000008d8c0 @ 0x80000008d8c0`
- `decode_gpr` lines [965, 992]
  - decomp: `FUN_80000008e7b8 @ 0x80000008e7b8`
- `decode_opcode` lines [326]
  - decomp: `FUN_80000008d8c0 @ 0x80000008d8c0`
- `decode_operand` lines [878]
  - decomp: `FUN_80000008e060 @ 0x80000008e060`
- `decode_operand_mem_16` lines [1170]
  - decomp: `FUN_80000008e6f4 @ 0x80000008e6f4`
  - decomp: `FUN_80000008e88c @ 0x80000008e88c`
- `decode_operand_register` lines [947]
  - decomp: `FUN_80000008d8c0 @ 0x80000008d8c0`
- `decode_xmm` lines [1002, 1016]
  - decomp: `FUN_80000008ed8c @ 0x80000008ed8c`
- `effects_for_instruction` lines [636, 669, 918, 933, 979]
  - decomp: `FUN_800000029ba8 @ 0x800000029ba8`
- `read_int` lines [86]
  - decomp: `FUN_80000008d500 @ 0x80000008d500`
- `walk_map_select` lines [1460]
  - decomp: `FUN_80000008d8c0 @ 0x80000008d8c0`

## `ArmInterpreter.cpp` (27 sites, 6 functions)

- `fpr_operand` lines [402]
  - decomp: `FUN_80000004d328 @ 0x80000004d328`
- `gpr_operand` lines [386]
  - decomp: `FUN_80000004d2b4 @ 0x80000004d2b4`
- `interpret_branch_reg` lines [106, 128]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `interpret_loadlit` lines [167]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `interpret_systemmove` lines [197]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `jit_instruction` lines [424, 465, 585, 713, 718, 723, 1051, 1088, 1099, 1103]
  - decomp: `FUN_80000004d38c @ 0x80000004d38c`

## `StateRecovery.cpp` (27 sites, 4 functions)

- `check_cfg` lines [270, 282, 293, 306]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `determine_loop_properties` lines [157, 202, 206, 215, 216, 221, 222]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `determine_state_recovery_action` lines [487, 500, 578]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`
- `determine_state_recovery_action_forward_branches` lines [338]
  - decomp: `FUN_80000007de84 @ 0x80000007de84`

## `TranslatorX87.cpp` (25 sites, 11 functions)

- `arithmetic_descriptor` lines [105]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `load_arithmetic_operand` lines [204]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fcom_fucom` lines [328, 334, 352, 372]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fcomi_fucomi` lines [386, 408]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_ffree` lines [423]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fist` lines [492]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fld` lines [508, 510, 512, 552]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fldconst` lines [894]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fst` lines [799]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fst_stack` lines [818]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_fxch` lines [944]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslationCacheJit.cpp` (23 sites, 7 functions)

- `handle_jit_breakpoint` lines [189, 209]
  - decomp: `FUN_80000002e700 @ 0x80000002e700`
- `iterate_fragments_in_x86_interval_holding_lock` lines [125]
  - decomp: `FUN_800000033920 @ 0x800000033920`
- `jit_translation_for_x86_address` lines [628, 634]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `operator()` lines [104, 506]
  - decomp: `FUN_800000034b64 @ 0x800000034b64`
  - decomp: `FUN_80000003e06c @ 0x80000003e06c`
- `remove_arm_tree_only_translation` lines [458, 466, 468]
  - decomp: `FUN_8000000349d8 @ 0x8000000349d8`
- `remove_fragment` lines [349]
  - decomp: `FUN_800000033e60 @ 0x800000033e60`
- `swap_permissions` lines [278]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`

## `Rounding.cpp` (22 sites, 3 functions)

- `round_internal_rs` lines [62, 73]
  - decomp: `FUN_80000004a2bc @ 0x80000004a2bc`
- `round_to_float80_pc_rs` lines [175, 218, 234]
  - decomp: `FUN_80000004a2bc @ 0x80000004a2bc`
- `round_to_float80_rs` lines [84, 103, 118]
  - decomp: `FUN_800000049c4c @ 0x800000049c4c`
  - decomp: `FUN_80000004a2bc @ 0x80000004a2bc`

## `ThreadContextFcntl.cpp` (21 sites, 7 functions)

- `create_fake_cpuinfo_file` lines [333, 341]
  - decomp: `FUN_80000002bf60 @ 0x80000002bf60`
- `create_tempfile` lines [85, 88]
  - decomp: `FUN_80000002c750 @ 0x80000002c750`
- `fcntl` lines [463]
  - decomp: `FUN_80000002cc20 @ 0x80000002cc20`
- `is_rosetta_process` lines [160, 178, 179]
  - decomp: `FUN_80000002bf60 @ 0x80000002bf60`
- `number_of_processors` lines [300, 311]
  - decomp: `FUN_80000002bf60 @ 0x80000002bf60`
- `open_auxv_tmp` lines [256]
  - decomp: `FUN_80000002bf60 @ 0x80000002bf60`
- `was_rosetta_process_invoked_directly` lines [142]
  - decomp: `FUN_80000002c824 @ 0x80000002c824`

## `ThreadContextSignals.cpp` (18 sites, 6 functions)

- `arm_fpr_state_from_extended_state` lines [349]
  - decomp: `FUN_800000042a20 @ 0x800000042a20`
- `deliver_signal` lines [452, 517]
  - decomp: `FUN_800000042a20 @ 0x800000042a20`
- `init_signal_mask_from_kernel` lines [255, 279]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `rt_sigprocmask` lines [287]
  - decomp: `FUN_8000000423f8 @ 0x8000000423f8`
- `rt_sigreturn` lines [370]
  - decomp: `FUN_8000000427c0 @ 0x8000000427c0`
- `sa_sigaction_handler` lines [784, 785, 790]
  - decomp: `FUN_800000042f44 @ 0x800000042f44`

## `CodeFragmentMetadata.cpp` (14 sites, 3 functions)

- `instruction_extents_for_arm_address` lines [53, 66]
  - decomp: `FUN_80000004c9cc @ 0x80000004c9cc`
- `push_back` lines [90, 92, 97]
  - decomp: `FUN_80000004ce68 @ 0x80000004ce68`
- `resolve_branch_target` lines [20, 24]
  - decomp: `FUN_80000004c9cc @ 0x80000004c9cc`

## `AssemblerBase.hpp` (13 sites, 8 functions)

- `fp_arith_vec` lines [943]
  - decomp: `FUN_800000080fe8 @ 0x800000080fe8`
- `ldp_post` lines [1239]
  - decomp: `FUN_800000028970 @ 0x800000028970`
- `ldr_post` lines [1286]
  - decomp: `FUN_800000028c50 @ 0x800000028c50`
- `shl_vec` lines [1859]
  - decomp: `FUN_800000029428 @ 0x800000029428`
- `smov_64` lines [2196]
  - decomp: `FUN_8000000297e4 @ 0x8000000297e4`
- `stp_pre` lines [2037]
  - decomp: `FUN_800000029428 @ 0x800000029428`
- `str_pre` lines [2092]
  - decomp: `FUN_8000000296f8 @ 0x8000000296f8`
- `ushr_vec` lines [2434]
  - decomp: `FUN_800000029a00 @ 0x800000029a00`

## `Operand.h` (13 sites, 1 functions)

- `operand_size` lines [116]
  - decomp: `FUN_800000055874 @ 0x800000055874`
  - decomp: `FUN_800000056040 @ 0x800000056040`
  - decomp: `FUN_800000056408 @ 0x800000056408`
  - decomp: `FUN_800000056608 @ 0x800000056608`
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
  - decomp: `FUN_800000079a98 @ 0x800000079a98`
  - decomp: `FUN_80000008319c @ 0x80000008319c`

## `TranslatorBase.hpp` (13 sites, 6 functions)

- `add_branch_island_if_necessary` lines [375]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `add_internal_fixup` lines [494, 531]
  - decomp: `FUN_80000003be94 @ 0x80000003be94`
- `fixup_last_instruction_branch_target` lines [476]
  - decomp: `FUN_8000000578b8 @ 0x8000000578b8`
- `rollback` lines [90]
  - decomp: `FUN_80000003c1a4 @ 0x80000003c1a4`
- `translate` lines [167]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `translate_block` lines [244]
  - decomp: `FUN_800000039528 @ 0x800000039528`

## `ThreadContext.cpp` (12 sites, 6 functions)

- `branch_slot_miss` lines [232]
  - decomp: `FUN_800000033860 @ 0x800000033860`
- `request_evict` lines [275]
  - decomp: `FUN_800000033a68 @ 0x800000033a68`
- `request_suspend` lines [242]
  - decomp: `FUN_80000002dbf8 @ 0x80000002dbf8`
- `request_suspend_no_signal` lines [218]
  - decomp: `FUN_80000002e360 @ 0x80000002e360`
- `resume` lines [312]
  - decomp: `FUN_80000002e428 @ 0x80000002e428`
- `translate_indirect_branch` lines [223]
  - decomp: `FUN_800000033814 @ 0x800000033814`

## `TranslatorSseMovesShuffles.cpp` (12 sites, 8 functions)

- `translate_extractps` lines [286]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_maskmov` lines [384]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_mov_vector` lines [460]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_mov_xmm` lines [423]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_movd_movq` lines [490]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_movs` lines [660]
  - decomp: `FUN_800000081d5c @ 0x800000081d5c`
- `translate_shuffle_from_plan` lines [1275]
  - decomp: `FUN_800000081fdc @ 0x800000081fdc`
- `translate_vmaskmov` lines [1616]
  - decomp: `FUN_8000000824bc @ 0x8000000824bc`

## `ThreadContextRuntimeSignals.cpp` (11 sites, 4 functions)

- `esr_context_from_extended_state` lines [33]
  - decomp: `FUN_80000002f0f0 @ 0x80000002f0f0`
- `handle_runtime_breakpoints` lines [123]
  - decomp: `FUN_80000002e700 @ 0x80000002e700`
- `runtime_signal_handler` lines [494, 501, 507]
  - decomp: `FUN_80000002f0f0 @ 0x80000002f0f0`
- `translate_sigtrap` lines [371]
  - decomp: `FUN_80000002edb8 @ 0x80000002edb8`

## `DeltaCodedOffsetMap.cpp` (10 sites, 3 functions)

- `DeltaCodedOffsetMapWriter` lines [526]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `entry_for_arm_offset` lines [133]
  - decomp: `FUN_80000008a8ac @ 0x80000008a8ac`
- `find_instruction_ranges` lines [297, 299, 338]
  - decomp: `FUN_80000004c9cc @ 0x80000004c9cc`

## `RuntimeLibraryBridgeInternal.cpp` (10 sites, 3 functions)

- `apply_fixups` lines [169, 179, 187]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `instruction_offsets` lines [140]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `translate` lines [114]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`

## `ProcMapsParser.cpp` (9 sites, 3 functions)

- `init_vm_allocation_tracker_from_procmaps` lines [139]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `parse_proc_maps_line` lines [42, 46, 50]
  - decomp: `FUN_80000008d0b4 @ 0x80000008d0b4`
- `procmaps_info_for_address` lines [75]
  - decomp: `FUN_80000008d1e8 @ 0x80000008d1e8`

## `Thread.cpp` (9 sites, 4 functions)

- `is_thread_alive` lines [18]
  - decomp: `FUN_80000002db70 @ 0x80000002db70`
- `is_valid_x86_thread` lines [159]
  - decomp: `FUN_8000000331fc @ 0x8000000331fc`
- `register_thread_context` lines [28, 30]
  - decomp: `FUN_80000002df8c @ 0x80000002df8c`
- `suspend_all_threads` lines [50]
  - decomp: `FUN_80000002dbf8 @ 0x80000002dbf8`

## `FstIeee.cpp` (8 sites, 1 functions)

- `convert_to_ieee` lines [103, 118]
  - decomp: `FUN_80000004efa8 @ 0x80000004efa8`
  - decomp: `FUN_80000004f2f4 @ 0x80000004f2f4`

## `TranslatorAes.cpp` (8 sites, 4 functions)

- `translate_aesdec` lines [29]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_aesdeclast` lines [75]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_aesenc` lines [392]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_aesenclast` lines [439]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslationCacheAot.cpp` (7 sites, 2 functions)

- `operator()` lines [146, 176, 244]
  - decomp: `FUN_80000002d50c @ 0x80000002d50c`
  - decomp: `FUN_80000002d5ec @ 0x80000002d5ec`
  - decomp: `FUN_80000002e700 @ 0x80000002e700`
- `remove_aot_fragments` lines [199, 215]
  - decomp: `FUN_800000033e60 @ 0x800000033e60`

## `AssemblerBuffer.h` (6 sites, 2 functions)

- `append_zero_fill` lines [42, 44]
  - decomp: `FUN_800000039528 @ 0x800000039528`
- `set_size` lines [53, 54]
  - decomp: `FUN_80000003c1a4 @ 0x80000003c1a4`

## `BranchTargetFinder.cpp` (6 sites, 3 functions)

- `process_jcc` lines [126]
  - decomp: `FUN_80000003e37c @ 0x80000003e37c`
- `process_jcxz` lines [135]
  - decomp: `FUN_80000003e37c @ 0x80000003e37c`
- `process_loop` lines [184]
  - decomp: `FUN_80000003e37c @ 0x80000003e37c`

## `Division.cpp` (6 sites, 1 functions)

- `divide_rs` lines [56, 61, 67, 73]
  - decomp: `FUN_80000004a2bc @ 0x80000004a2bc`

## `IndirectBranchHashMap.cpp` (6 sites, 3 functions)

- `Version` lines [65]
  - decomp: `FUN_80000004c724 @ 0x80000004c724`
- `insert_if_new` lines [210]
  - decomp: `FUN_80000004c724 @ 0x80000004c724`
- `remove_holding_lock` lines [139]
  - decomp: `FUN_80000004c33c @ 0x80000004c33c`

## `Operand.cpp` (6 sites, 2 functions)

- `condition_code_to_string` lines [44]
  - decomp: `FUN_80000002ac58 @ 0x80000002ac58`
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
- `segment_register_to_string` lines [61]
  - decomp: `FUN_80000002ac58 @ 0x80000002ac58`

## `Repatch.cpp` (6 sites, 3 functions)

- `repatch_branch_14` lines [59]
  - decomp: `FUN_80000003be94 @ 0x80000003be94`
- `repatch_branch_19` lines [46]
  - decomp: `FUN_80000003be94 @ 0x80000003be94`
- `repatch_branch_26` lines [33]
  - decomp: `FUN_800000029ad0 @ 0x800000029ad0`

## `ThreadContextVm.cpp` (6 sites, 3 functions)

- `io_destroy` lines [412]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `io_setup` lines [392]
  - decomp: `FUN_800000030314 @ 0x800000030314`
- `mremap` lines [223]
  - decomp: `FUN_80000002f0f0 @ 0x80000002f0f0`

## `TranslatorBitTest.cpp` (6 sites, 4 functions)

- `imm_info_for_high_index_mask` lines [108]
  - decomp: `FUN_800000080ed8 @ 0x800000080ed8`
- `imm_info_for_low_index_mask` lines [80]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_bt` lines [211, 299]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_bt_atomic` lines [348, 389]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorMulDiv.cpp` (6 sites, 3 functions)

- `high_reg_is_liveout` lines [54]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_div` lines [375]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_mul` lines [452, 483, 530, 585]
  - decomp: `FUN_80000008319c @ 0x80000008319c`

## `TranslatorSseConversions.cpp` (6 sites, 5 functions)

- `translate_cvtpd2i` lines [55, 158]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_cvtps2i` lines [212]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_cvts2i` lines [314]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_roundp` lines [527]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_rounds` lines [478]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `AssemblerHelpers.cpp` (5 sites, 3 functions)

- `brk_inst_to_kind` lines [184]
  - decomp: `FUN_80000002edb8 @ 0x80000002edb8`
- `is_bitmask_immediate` lines [171]
  - decomp: `FUN_80000002a6bc @ 0x80000002a6bc`
- `shift_to_mem_size` lines [14]
  - decomp: `FUN_80000008599c @ 0x80000008599c`

## `BestFitAllocator.h` (5 sites, 1 functions)

- `allocate_from_node` lines [247, 248, 252]
  - decomp: `FUN_80000003c9b8 @ 0x80000003c9b8`

## `BuilderBase.h` (5 sites, 3 functions)

- `block_for_offset` lines [550]
  - decomp: `FUN_80000003880c @ 0x80000003880c`
- `finish_block` lines [670]
  - decomp: `FUN_8000000386fc @ 0x8000000386fc`
- `process_jcxz` lines [405]
  - decomp: `FUN_80000003e37c @ 0x80000003e37c`

## `ThreadContextSyscalls.cpp` (5 sites, 3 functions)

- `alarm` lines [467]
  - decomp: `FUN_800000032984 @ 0x800000032984`
- `before_clone_thread` lines [91]
  - decomp: `FUN_800000032040 @ 0x800000032040`
- `clone_process` lines [179]
  - decomp: `FUN_8000000321b0 @ 0x8000000321b0`

## `Vdso.cpp` (5 sites, 5 functions)

- `get_vdso_dynsym_shdr` lines [45]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `get_vdso_elf_header` lines [18]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `get_vdso_load_offset` lines [31]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `get_vdso_strtab` lines [59]
  - decomp: `FUN_8000000303ec @ 0x8000000303ec`
- `initialize_function_table_entry` lines [84]
  - decomp: `FUN_800000031e04 @ 0x800000031e04`

## `ElfMapper.cpp` (4 sites, 1 functions)

- `map_elf` lines [399, 401]
  - decomp: `FUN_8000000900a0 @ 0x8000000900a0`

## `ExecutableHeap.cpp` (4 sites, 2 functions)

- `allocate` lines [119]
  - decomp: `FUN_80000003c87c @ 0x80000003c87c`
- `runtime_routines_for_address` lines [150]
  - decomp: `FUN_80000003d514 @ 0x80000003d514`

## `GuardBits.cpp` (4 sites, 2 functions)

- `left_shift_128_s` lines [83]
  - decomp: `FUN_8000000497b4 @ 0x8000000497b4`
- `right_shift_64_rs` lines [65]
  - decomp: `FUN_80000004ae70 @ 0x80000004ae70`

## `InitStack.cpp` (4 sites, 1 functions)

- `get_argv_skip_for_other_rosetta` lines [262, 268]
  - decomp: `FUN_80000002bf60 @ 0x80000002bf60`

## `IrBuilder_x86.h` (4 sites, 2 functions)

- `branch_target_from_operand` lines [104]
  - decomp: `FUN_800000038b38 @ 0x800000038b38`
- `process_call` lines [73]
  - decomp: `FUN_80000003e37c @ 0x80000003e37c`

## `OffsetPairArray.cpp` (4 sites, 2 functions)

- `find_instruction_ranges` lines [107]
  - decomp: `FUN_80000004c9cc @ 0x80000004c9cc`
- `find_x86_entry` lines [81]
  - decomp: `FUN_80000008ae80 @ 0x80000008ae80`

## `Signals.cpp` (4 sites, 2 functions)

- `kernel_sigismember` lines [85]
  - decomp: `FUN_800000054e14 @ 0x800000054e14`
- `sigaddset` lines [55]
  - decomp: `FUN_800000042a20 @ 0x800000042a20`

## `TransactionalList.h` (4 sites, 1 functions)

- `move_vector` lines [27]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`
  - decomp: `FUN_80000003bcf0 @ 0x80000003bcf0`

## `TranslatorSseFpComparisons.cpp` (4 sites, 4 functions)

- `translate_cmpp` lines [28]
  - decomp: `FUN_800000083ac0 @ 0x800000083ac0`
- `translate_cmps` lines [369]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_comis` lines [734]
  - decomp: `FUN_800000085144 @ 0x800000085144`
- `translate_vtestp` lines [761]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorSseInteger.cpp` (4 sites, 4 functions)

- `translate_pmovsxb` lines [510]
  - decomp: `FUN_80000007fe08 @ 0x80000007fe08`
- `translate_pmovsxw` lines [538]
  - decomp: `FUN_800000080040 @ 0x800000080040`
- `translate_pmovzxb` lines [585]
  - decomp: `FUN_8000000801ec @ 0x8000000801ec`
- `translate_pmovzxw` lines [613]
  - decomp: `FUN_800000080424 @ 0x800000080424`

## `TwoLevelOffsetMap.cpp` (4 sites, 1 functions)

- `TwoLevelOffsetMapWriter` lines [230, 231]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`

## `FixedArray.h` (3 sites, 1 functions)

- `push_back` lines [29]
  - decomp: `FUN_800000046a34 @ 0x800000046a34`
  - decomp: `FUN_800000047ce8 @ 0x800000047ce8`
  - decomp: `FUN_800000048a04 @ 0x800000048a04`

## `RedBlackTree.h` (3 sites, 1 functions)

- `find_parent_for_insertion_no_duplicates` lines [214]
  - decomp: `FUN_80000002cce4 @ 0x80000002cce4`
  - decomp: `FUN_80000002d140 @ 0x80000002d140`
  - decomp: `FUN_80000008b230 @ 0x80000008b230`

## `Register.cpp` (3 sites, 1 functions)

- `register_to_string` lines [174, 200]
  - decomp: `FUN_80000008d3d0 @ 0x80000008d3d0`

## `TranslatorRotations.cpp` (3 sites, 3 functions)

- `translate_rol` lines [434]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_ror` lines [542]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_ror_imm` lines [620]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorSseStringOps.cpp` (3 sites, 3 functions)

- `translate_pcmpestri` lines [30]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_pcmpestrm` lines [86]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_pcmpistri` lines [170]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `AdditionMultiplication.cpp` (2 sites, 1 functions)

- `fma_rs` lines [17]
  - decomp: `FUN_8000000497b4 @ 0x8000000497b4`

## `ArmInterpreterRuntime.cpp` (2 sites, 1 functions)

- `interpret` lines [22]
  - decomp: `FUN_800000040f24 @ 0x800000040f24`

## `Fprem.cpp` (2 sites, 1 functions)

- `fprem` lines [178]
  - decomp: `FUN_800000049c4c @ 0x800000049c4c`

## `Fsqrt.cpp` (2 sites, 1 functions)

- `sqrt_rs` lines [56]
  - decomp: `FUN_800000052748 @ 0x800000052748`

## `ImageInfo.cpp` (2 sites, 1 functions)

- `create_from_header` lines [71]
  - decomp: `FUN_800000090a0c @ 0x800000090a0c`

## `InternalFloat.cpp` (2 sites, 1 functions)

- `unpack_float80` lines [13]
  - decomp: `FUN_800000049c4c @ 0x800000049c4c`

## `MmapTracker.cpp` (2 sites, 1 functions)

- `mmap_guest_errno` lines [169]
  - decomp: `FUN_800000055158 @ 0x800000055158`

## `Opcode.cpp` (2 sites, 1 functions)

- `opcode_to_string` lines [680]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`

## `SlabAllocator.cpp` (2 sites, 1 functions)

- `allocate_free_entry` lines [105]
  - decomp: `FUN_80000008ac84 @ 0x80000008ac84`

## `TaggedPointer.h` (2 sites, 1 functions)

- `set_pointer` lines [33]
  - decomp: `FUN_80000003498c @ 0x80000003498c`

## `ThreadState.h` (2 sites, 1 functions)

- `set_nzcv` lines [55]
  - decomp: `FUN_80000004077c @ 0x80000004077c`

## `TranslationCache.cpp` (2 sites, 1 functions)

- `find_translation_in_tree_x86` lines [311]
  - decomp: `FUN_800000037be4 @ 0x800000037be4`

## `TranslationCacheAbi.cpp` (2 sites, 1 functions)

- `abi_info_for_runtime_routines_address` lines [105]
  - decomp: `FUN_80000003d648 @ 0x80000003d648`

## `TranslatorArithmetic.cpp` (2 sites, 1 functions)

- `translate_cmpxchg16b` lines [380]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorHelpers.hpp` (2 sites, 1 functions)

- `read_literal_no_xzr` lines [136]
  - decomp: `FUN_8000000575f8 @ 0x8000000575f8`

## `TranslatorShifts.cpp` (2 sites, 2 functions)

- `translate_shld` lines [327]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_shrd` lines [546]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorSseShifts.cpp` (2 sites, 2 functions)

- `translate_psll` lines [84]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`
- `translate_psra` lines [186]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `UnfairLock.cpp` (2 sites, 1 functions)

- `unfair_lock_unlock_slow` lines [121]
  - decomp: `FUN_800000055374 @ 0x800000055374`

## `x87.cpp` (2 sites, 2 functions)

- `x87_fcom_ST` lines [423]
  - decomp: `FUN_80000004f90c @ 0x80000004f90c`
- `x87_fucom` lines [476]
  - decomp: `FUN_80000004fc50 @ 0x80000004fc50`

## `BasicBlock.cpp` (1 sites, 1 functions)

- `print` lines [295]
  - decomp: `FUN_800000034da8 @ 0x800000034da8`

## `Fixup.cpp` (1 sites, 1 functions)

- `fixup_absolute` lines [74]
  - decomp: `FUN_80000002a500 @ 0x80000002a500`

## `Mutex.cpp` (1 sites, 1 functions)

- `mutex_unlock` lines [69]
  - decomp: `FUN_800000054ef4 @ 0x800000054ef4`

## `Segmentation.cpp` (1 sites, 1 functions)

- `mov_segment` lines [480]
  - decomp: `FUN_80000008f0fc @ 0x80000008f0fc`

## `TranslatorFlags.cpp` (1 sites, 1 functions)

- `generate_z_flag` lines [115]
  - decomp: `FUN_800000082d18 @ 0x800000082d18`

## `TranslatorLogical.cpp` (1 sites, 1 functions)

- `translate_xor` lines [378]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

## `TranslatorStringOps.cpp` (1 sites, 1 functions)

- `translate_movs_string` lines [153]
  - decomp: `FUN_800000057c8c @ 0x800000057c8c`

