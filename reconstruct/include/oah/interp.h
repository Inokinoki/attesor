#ifndef OAH_INTERP_H
#define OAH_INTERP_H

#include "oah/elf_map.h"
#include "oah/guest.h"
#include "oah/syscall_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OAH_INTERP_OK = 0,
    OAH_INTERP_EXIT,
    OAH_INTERP_DECODE,
    OAH_INTERP_FAULT,
    OAH_INTERP_LIMIT
} oah_interp_status;

typedef struct {
    oah_interp_status status;
    int exit_code;
    u64 insns;
} oah_interp_result;

#define OAH_INTERP_MAX_INSNS 1000000ull

/* Fetch-decode-execute until SYSCALL-exit, decode error, or limit. */
oah_interp_result oah_interp_run(oah_guest *g, oah_image *img, u64 max_insns);

#ifdef __cplusplus
}
#endif

#endif /* OAH_INTERP_H */
