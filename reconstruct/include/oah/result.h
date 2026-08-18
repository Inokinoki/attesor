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
 * Ghidra lost the SVC immediate (x8), so individual syscall stubs cannot be
 * told apart from the dump body. Only the Result packing is recoverable here.
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

#ifdef __cplusplus
}
#endif

#endif /* OAH_RESULT_H */
