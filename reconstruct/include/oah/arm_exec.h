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
 * On this x86_64 CI the fragment cannot run natively; this is the stand-in
 * for jumping into RX memory on Apple Silicon.
 */
oah_interp_result oah_arm_exec(oah_arm_state *st, const u8 *code, u64 code_size,
                               oah_image *img, u64 max_insns);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ARM_EXEC_H */
