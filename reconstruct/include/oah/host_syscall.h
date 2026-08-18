#ifndef OAH_HOST_SYSCALL_H
#define OAH_HOST_SYSCALL_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host (AArch64) syscall numbers used by RosettaLinux libc-style wrappers.
 *
 * Ghidra collapsed every wrapper to `CallSupervisor(0)`, so the dump cannot
 * tell openat from memfd_create. The ELF has:
 *
 *   movz x8, #NR
 *   svc  #0
 *   cmn  x0, #0xfff
 *   ... raw (CSINV → -1) or result (CSNEG + CSINC → {value,is_error}) ...
 *
 * 76 distinct NRs, each with a raw and a result stub in `.text` (clone has
 * an extra result stub in `runtime_savedreg`).
 *
 * confidence: high (immediates copied from the binary)
 */
u32 oah_host_syscall_count(void);

/* Linux asm-generic name for an AArch64 NR that appears in a wrapper.
 * NULL if this translator never emits that NR as a libc wrapper. */
const char *oah_host_syscall_name(u32 nr);

/* True if a raw/result wrapper pair exists for this NR. */
bool oah_host_syscall_has_wrapper(u32 nr);

#ifdef __cplusplus
}
#endif

#endif /* OAH_HOST_SYSCALL_H */
