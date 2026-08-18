#include "oah/syscall_bridge.h"
#include "oah/linux_abi.h"

#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

/*
 * x86_64 → AArch64 asm-generic numbers for dispatched syscalls.
 * arch_prctl has no ARM equivalent (handled in-process).
 */
u32 oah_x86_syscall_to_host(u32 x86_nr)
{
#if defined(__aarch64__)
    switch (x86_nr) {
    case OAH_X86_NR_READ:       return 63;
    case OAH_X86_NR_WRITE:      return 64;
    case OAH_X86_NR_CLOSE:      return 57;
    case OAH_X86_NR_BRK:        return 214;
    case OAH_X86_NR_EXIT:       return 93;
    case OAH_X86_NR_UNAME:      return 160;
    case OAH_X86_NR_EXIT_GROUP: return 94;
    case OAH_X86_NR_OPENAT:     return 56;
    case OAH_X86_NR_GETRANDOM:  return 278;
    default:                    return (u32)-1;
    }
#else
    /* Host already uses the x86_64 numbering. */
    return x86_nr;
#endif
}

static u64 set_errno_ret(s64 rc)
{
    if (rc < 0) {
        return (u64)(s64)(-errno);
    }
    return (u64)rc;
}

oah_sys_status oah_guest_syscall(oah_guest *g, oah_image *img)
{
    u64 nr = g->gpr[OAH_X86_RAX];
    u64 a0 = g->gpr[OAH_X86_RDI];
    u64 a1 = g->gpr[OAH_X86_RSI];
    u64 a2 = g->gpr[OAH_X86_RDX];
    u64 a3 = g->gpr[OAH_X86_R10];
    u64 a4 = g->gpr[OAH_X86_R8];
    u64 a5 = g->gpr[OAH_X86_R9];
    u32 host_nr;
    long rc;

    /* SYSCALL: RCX = rip (return), R11 = RFLAGS. */
    g->gpr[OAH_X86_RCX] = g->rip;
    g->gpr[OAH_X86_R11] = g->rflags;

    switch (nr) {
    case OAH_X86_NR_EXIT:
    case OAH_X86_NR_EXIT_GROUP:
        return OAH_SYS_EXIT;

    case OAH_X86_NR_BRK:
        if (a0 == 0) {
            g->gpr[OAH_X86_RAX] = img ? img->brk : 0;
        } else if (img && a0 >= img->load_end) {
            img->brk = a0;
            g->gpr[OAH_X86_RAX] = a0;
        } else {
            g->gpr[OAH_X86_RAX] = (u64)(s64)-ENOMEM;
        }
        return OAH_SYS_CONTINUE;

    case OAH_X86_NR_ARCH_PRCTL:
        if (a0 == OAH_ARCH_SET_FS) {
            g->fs_base = a1;
            g->gpr[OAH_X86_RAX] = 0;
        } else if (a0 == OAH_ARCH_GET_FS) {
            u64 *out = (u64 *)oah_guest_to_host(img, a1, 8);
            if (!out) {
                g->gpr[OAH_X86_RAX] = (u64)(s64)-EFAULT;
            } else {
                *out = g->fs_base;
                g->gpr[OAH_X86_RAX] = 0;
            }
        } else {
            g->gpr[OAH_X86_RAX] = (u64)(s64)-EINVAL;
        }
        return OAH_SYS_CONTINUE;

    case OAH_X86_NR_WRITE:
    case OAH_X86_NR_READ: {
        void *buf = oah_guest_to_host(img, a1, a2 ? a2 : 1);
        if (a2 != 0 && !buf) {
            g->gpr[OAH_X86_RAX] = (u64)(s64)-EFAULT;
            return OAH_SYS_CONTINUE;
        }
        host_nr = oah_x86_syscall_to_host((u32)nr);
        errno = 0;
        rc = syscall((long)host_nr, (long)a0, buf, (long)a2);
        g->gpr[OAH_X86_RAX] = set_errno_ret(rc);
        return OAH_SYS_CONTINUE;
    }

    case OAH_X86_NR_UNAME: {
        struct utsname *un = oah_guest_to_host(img, a0, sizeof(*un));
        if (!un) {
            g->gpr[OAH_X86_RAX] = (u64)(s64)-EFAULT;
            return OAH_SYS_CONTINUE;
        }
        host_nr = oah_x86_syscall_to_host((u32)nr);
        errno = 0;
        rc = syscall((long)host_nr, un);
        g->gpr[OAH_X86_RAX] = set_errno_ret(rc);
        return OAH_SYS_CONTINUE;
    }

    case OAH_X86_NR_CLOSE:
    case OAH_X86_NR_GETRANDOM:
    case OAH_X86_NR_OPENAT:
        host_nr = oah_x86_syscall_to_host((u32)nr);
        if (host_nr == (u32)-1) {
            g->gpr[OAH_X86_RAX] = (u64)(s64)-ENOSYS;
            return OAH_SYS_CONTINUE;
        }
        if (nr == OAH_X86_NR_OPENAT) {
            const char *path = oah_guest_to_host(img, a1, 1);
            u32 flags = (u32)a2;
            if (!path) {
                g->gpr[OAH_X86_RAX] = (u64)(s64)-EFAULT;
                return OAH_SYS_CONTINUE;
            }
#if defined(__aarch64__)
            flags = oah_translate_open_flags(flags);
#endif
            errno = 0;
            rc = syscall((long)host_nr, (long)a0, path, (long)flags, (long)a3);
        } else if (nr == OAH_X86_NR_GETRANDOM) {
            void *buf = oah_guest_to_host(img, a0, a1 ? a1 : 1);
            if (a1 != 0 && !buf) {
                g->gpr[OAH_X86_RAX] = (u64)(s64)-EFAULT;
                return OAH_SYS_CONTINUE;
            }
            errno = 0;
            rc = syscall((long)host_nr, buf, (long)a1, (long)a2);
        } else {
            errno = 0;
            rc = syscall((long)host_nr, (long)a0, (long)a1, (long)a2,
                         (long)a3, (long)a4, (long)a5);
        }
        g->gpr[OAH_X86_RAX] = set_errno_ret(rc);
        return OAH_SYS_CONTINUE;

    default:
        g->gpr[OAH_X86_RAX] = (u64)(s64)-ENOSYS;
        return OAH_SYS_CONTINUE;
    }
}
