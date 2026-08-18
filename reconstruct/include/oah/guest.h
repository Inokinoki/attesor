#ifndef OAH_GUEST_H
#define OAH_GUEST_H

#include "oah/register.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Guest x86_64 integer state. GPR order matches Linux user_regs_struct
 * (RAX..R15) recovered from FUN_800000040650.
 *
 * The original runtime then copies these into host Xn (see
 * oah_host_xn_for_x86_gpr). The interpreter keeps them in this file and
 * only applies that map when a JIT exists.
 */
typedef struct {
    u64 gpr[OAH_X86_GPR_COUNT];
    u64 rip;
    u64 rflags;
    u64 fs_base;
    u64 gs_base;
} oah_guest;

/* Linux userspace default: bit 1 reserved-1, bit 9 IF. */
#define OAH_RFLAGS_INIT 0x202ull

void oah_guest_init(oah_guest *g, u64 entry, u64 rsp);

#ifdef __cplusplus
}
#endif

#endif /* OAH_GUEST_H */
