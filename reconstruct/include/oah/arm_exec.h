#ifndef OAH_ARM_EXEC_H
#define OAH_ARM_EXEC_H

#include "oah/elf_map.h"
#include "oah/interp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64 x[32];
    u64 pc;
} oah_arm_state;

#define OAH_ARM_XSP 31

/*
 * Execute a straight-line ARM64 fragment produced by the translator.
 * Implements the insns we emit (MOVZ/K, ORR, EOR, ADD/SUB imm, LDR/STR,
 * SVC #0, RET). SVC #0 uses the Linux AArch64 argument layout (after the
 * emitted runtime_syscall shuffle). Exit/exit_group are intercepted so we
 * do not kill the host.
 *
 * Used by unit tests on every host. `oah` on aarch64 (including qemu-user)
 * jumps into RX memory via oah_arm_exec_native instead.
 */
oah_interp_result oah_arm_exec(oah_arm_state *st, const u8 *code, u64 code_size,
                               oah_image *img, u64 max_insns);

/*
 * mmap the fragment RX and BLR into it. Loads/stores X0–X15 around the call.
 * RET in the fragment returns here. A live `svc` (exit/exit_group) terminates
 * the process — that is intentional for `oah`, not for unit tests.
 *
 * On non-aarch64 hosts returns OAH_INTERP_DECODE without executing.
 */
oah_interp_result oah_arm_exec_native(oah_arm_state *st, const u8 *code,
                                      u64 code_size);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ARM_EXEC_H */
