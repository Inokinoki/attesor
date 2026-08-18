#ifndef OAH_RESULT_H
#define OAH_RESULT_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Syscall result returned in x0+x1 (Ghidra `undefined1 [16]`).
 *
 * decomp: FUN_800000026cb0 and ~80 identical wrappers:
 *   value    = (ret > -4096) ? -ret : ret     // positive errno on error
 *   is_error = (unsigned)ret > 0xfffffffffffff000   // Linux -1..-4095
 *
 * The ELF (not the C dump) recovers the SVC number: `movz x8, #NR; svc #0`.
 * See oah/host_syscall.h for the NR → name map.
 *
 * Two packing styles sit back-to-back for each NR:
 *   raw    FUN_800000026b94: CMN; CSINV → saturate to -1
 *   result FUN_800000026cb0: CMN; CSNEG; CSINC → {abs(errno), is_error}
 */
typedef struct {
    u64 value;
    u64 is_error; /* 0 or 1 */
} oah_result;

#define OAH_LINUX_ERRNO_LIMIT 0xfffffffffffff000ULL /* -4096 as unsigned */

static inline bool oah_result_is_error(oah_result r)
{
    return r.is_error != 0;
}

/* Pack a raw Linux syscall return (negative errno on failure). */
oah_result oah_result_from_linux_ret(u64 raw);

/* Saturate a Linux syscall return to -1 on error (the "raw" wrappers). */
u64 oah_raw_from_linux_ret(u64 raw);

#ifdef __cplusplus
}
#endif

#endif /* OAH_RESULT_H */
