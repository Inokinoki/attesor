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
 * The fragment always ends with RET (LR=0 means return to the dispatcher).
 */
bool oah_translate_block(oah_asm_buf *buf, oah_image *img, u64 rip, u64 *next_rip);

/* Shuffle runtime Xn into Linux SVC ABI and `svc #0`, then restore. */
bool oah_emit_runtime_syscall(oah_asm_buf *buf);

/* Dispatcher: pack GPRs, translate+execute fragments until guest exit. */
oah_interp_result oah_jit_run(oah_guest *g, oah_image *img, u64 max_insns);

#ifdef __cplusplus
}
#endif

#endif /* OAH_TRANSLATE_H */
