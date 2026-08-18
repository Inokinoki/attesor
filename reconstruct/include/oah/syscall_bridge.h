#ifndef OAH_SYSCALL_BRIDGE_H
#define OAH_SYSCALL_BRIDGE_H

#include "oah/elf_map.h"
#include "oah/guest.h"

#ifdef __cplusplus
extern "C" {
#endif

/* x86_64 Linux syscall numbers used by the vertical slice. */
#define OAH_X86_NR_READ        0
#define OAH_X86_NR_WRITE       1
#define OAH_X86_NR_CLOSE       3
#define OAH_X86_NR_BRK         12
#define OAH_X86_NR_EXIT        60
#define OAH_X86_NR_UNAME       63
#define OAH_X86_NR_ARCH_PRCTL  158
#define OAH_X86_NR_EXIT_GROUP  231
#define OAH_X86_NR_OPENAT      257
#define OAH_X86_NR_GETRANDOM   318

#define OAH_ARCH_SET_FS 0x1002
#define OAH_ARCH_GET_FS 0x1003

/*
 * derived: Linux asm-generic vs x86_64 unistd. The original remaps inside
 * runtime_syscall (0x8000000964b8); that trampoline shuffles runtime Xn
 * into AArch64 syscall ABI. We have not copied Apple's table; this is the
 * public Linux mapping for NRs we actually dispatch.
 *
 * Returns (u32)-1 if this host has no equivalent (e.g. arch_prctl on ARM).
 */
u32 oah_x86_syscall_to_host(u32 x86_nr);

typedef enum {
    OAH_SYS_CONTINUE = 0,
    OAH_SYS_EXIT     = 1,
    OAH_SYS_FATAL    = 2
} oah_sys_status;

/*
 * Perform guest SYSCALL. Args: rax=nr, rdi,rsi,rdx,r10,r8,r9.
 * Intel SYSCALL also writes rcx=next_rip, r11=rflags.
 * Result in rax (negative errno on failure, matching the raw wrappers).
 */
oah_sys_status oah_guest_syscall(oah_guest *g, oah_image *img);

#ifdef __cplusplus
}
#endif

#endif /* OAH_SYSCALL_BRIDGE_H */
