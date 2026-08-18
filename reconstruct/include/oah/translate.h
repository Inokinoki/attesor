#ifndef OAH_TRANSLATE_H
#define OAH_TRANSLATE_H

#include "oah/assembler_buffer.h"
#include "oah/elf_map.h"
#include "oah/guest.h"
#include "oah/interp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Translate one x86_64 block into ARM64 using the runtime GPR map
 * (FUN_800000040650) and a runtime_syscall shuffle + `svc #0`.
 *
 * The fragment always ends with RET (LR=0 means return to the dispatcher
 * when the C interpreter runs the fragment).
 *
 * flags:
 *   OAH_XLAT_HOST_SVC — rewrite x86_64 NR in X8 to the aarch64 NR before
 *   `svc #0`, for native/QEMU execution. Unit tests leave this off so
 *   oah_arm_exec still sees x86 numbers.
 */
#define OAH_XLAT_HOST_SVC 1u

bool oah_translate_block(oah_asm_buf *buf, oah_image *img, u64 rip, u64 *next_rip);
bool oah_translate_block_flags(oah_asm_buf *buf, oah_image *img, u64 rip,
                               u64 *next_rip, u32 flags);

/* Shuffle runtime Xn into Linux SVC ABI and `svc #0`, then restore. */
bool oah_emit_runtime_syscall(oah_asm_buf *buf);
/* Same shuffle, then x86 NR → aarch64 NR compares before `svc #0`. */
bool oah_emit_runtime_syscall_host(oah_asm_buf *buf);

/* Dispatcher: pack GPRs, translate+interpret fragments until guest exit. */
oah_interp_result oah_jit_run(oah_guest *g, oah_image *img, u64 max_insns);

/*
 * Same dispatcher, but mmap/BLR the fragment on aarch64. `svc` is real
 * (including exit). On other hosts this is the interpreter path.
 */
oah_interp_result oah_jit_run_native(oah_guest *g, oah_image *img, u64 max_insns);

#ifdef __cplusplus
}
#endif

#endif /* OAH_TRANSLATE_H */
