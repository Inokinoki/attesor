#include "oah/arm_exec.h"

#include "oah/syscall_bridge.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

static u32 rd(u32 w) { return w & 31u; }
static u32 rn(u32 w) { return (w >> 5) & 31u; }
static u32 rm(u32 w) { return (w >> 16) & 31u; }

static u64 *xr(oah_arm_state *st, u32 n)
{
    return &st->x[n];
}

static s64 set_err(long rc)
{
    if (rc < 0) {
        return -(s64)errno;
    }
    return rc;
}

oah_interp_result oah_arm_exec(oah_arm_state *st, const u8 *code, u64 code_size,
                               oah_image *img, u64 max_insns)
{
    oah_interp_result r;
    u64 start = (u64)(uintptr_t)code;
    u64 end = start + code_size;

    r.status = OAH_INTERP_OK;
    r.exit_code = 0;
    r.insns = 0;
    st->pc = start;
    if (max_insns == 0) {
        max_insns = OAH_INTERP_MAX_INSNS;
    }

    while (r.insns < max_insns) {
        u32 w;
        u64 pc = st->pc;

        if (pc < start || pc + 4 > end) {
            /* RET to dispatcher: LR=0 or PC left the fragment. */
            r.status = OAH_INTERP_OK;
            return r;
        }
        memcpy(&w, (const void *)(uintptr_t)pc, 4);
        r.insns++;
        st->pc = pc + 4;

        if (w == 0xd503201fu) { /* NOP */
            continue;
        }
        if (w == 0xd65f03c0u) { /* RET X30 */
            if (st->x[30] == 0) {
                r.status = OAH_INTERP_OK;
                return r;
            }
            st->pc = st->x[30];
            continue;
        }
        if (w == 0xd4000001u) { /* SVC #0 */
            u32 nr = (u32)st->x[8];
            u64 a0 = st->x[0], a1 = st->x[1], a2 = st->x[2];
            u64 a3 = st->x[3], a4 = st->x[4], a5 = st->x[5];
            u32 host = oah_x86_syscall_to_host(nr);
            long rc;

            if (nr == OAH_X86_NR_EXIT || nr == OAH_X86_NR_EXIT_GROUP) {
                r.status = OAH_INTERP_EXIT;
                r.exit_code = (int)(a0 & 0xff);
                return r;
            }
            if (host == (u32)-1) {
                st->x[0] = (u64)(s64)-ENOSYS;
                continue;
            }
            if (nr == OAH_X86_NR_WRITE || nr == OAH_X86_NR_READ) {
                void *p = oah_guest_to_host(img, a1, a2 ? a2 : 1);
                if (a2 != 0 && !p) {
                    st->x[0] = (u64)(s64)-EFAULT;
                    continue;
                }
                errno = 0;
                rc = syscall((long)host, (long)a0, p, (long)a2);
                st->x[0] = (u64)set_err(rc);
                continue;
            }
            if (nr == OAH_X86_NR_BRK) {
                if (a0 == 0) {
                    st->x[0] = img ? img->brk : 0;
                } else if (img && a0 >= img->load_end) {
                    img->brk = a0;
                    st->x[0] = a0;
                } else {
                    st->x[0] = (u64)(s64)-ENOMEM;
                }
                continue;
            }
            errno = 0;
            rc = syscall((long)host, (long)a0, (long)a1, (long)a2,
                         (long)a3, (long)a4, (long)a5);
            st->x[0] = (u64)set_err(rc);
            continue;
        }

        /* MOVZ 64/32 */
        if ((w & 0xff800000u) == 0xd2800000u || (w & 0xff800000u) == 0x52800000u) {
            u32 hw = (w >> 21) & 3u;
            u64 imm = (u64)((w >> 5) & 0xffffu) << (hw * 16);
            bool is64 = (w & 0x80000000u) != 0;
            if (is64) {
                *xr(st, rd(w)) = imm;
            } else {
                *xr(st, rd(w)) = imm & 0xffffffffull;
            }
            continue;
        }
        /* MOVK */
        if ((w & 0xff800000u) == 0xf2800000u || (w & 0xff800000u) == 0x72800000u) {
            u32 hw = (w >> 21) & 3u;
            u64 imm = (u64)((w >> 5) & 0xffffu) << (hw * 16);
            u64 mask = 0xffffull << (hw * 16);
            bool is64 = (w & 0x80000000u) != 0;
            u64 cur = *xr(st, rd(w));
            if (!is64) {
                cur &= 0xffffffffull;
                mask &= 0xffffffffull;
            }
            cur = (cur & ~mask) | (imm & mask);
            *xr(st, rd(w)) = is64 ? cur : (cur & 0xffffffffull);
            continue;
        }
        /* ORR Rd, XZR, Rm  (MOV) 64: aa0003e0 mask */
        if ((w & 0xffe0ffe0u) == 0xaa0003e0u) {
            *xr(st, rd(w)) = *xr(st, rm(w));
            continue;
        }
        if ((w & 0xffe0ffe0u) == 0x2a0003e0u) {
            *xr(st, rd(w)) = *xr(st, rm(w)) & 0xffffffffull;
            continue;
        }
        /* EOR */
        if ((w & 0xff200000u) == 0xca000000u) {
            *xr(st, rd(w)) = *xr(st, rn(w)) ^ *xr(st, rm(w));
            continue;
        }
        if ((w & 0xff200000u) == 0x4a000000u) {
            *xr(st, rd(w)) = (*xr(st, rn(w)) ^ *xr(st, rm(w))) & 0xffffffffull;
            continue;
        }
        /* ADD/SUB Xd, Xn, #imm12 */
        if ((w & 0xff000000u) == 0x91000000u || (w & 0xff000000u) == 0xd1000000u) {
            u64 imm = (w >> 10) & 0xfffu;
            u64 a = *xr(st, rn(w));
            u64 v = ((w >> 30) & 1u) ? a - imm : a + imm;
            *xr(st, rd(w)) = v;
            continue;
        }
        /* LDR/STR Xt, [Xn, #imm] */
        if ((w & 0xffc00000u) == 0xf9000000u || (w & 0xffc00000u) == 0xf9400000u) {
            u64 addr = *xr(st, rn(w)) + (((w >> 10) & 0xfffu) << 3);
            u64 *slot = (u64 *)oah_guest_to_host(img, addr, 8);
            if (!slot) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            if ((w & 0xffc00000u) == 0xf9000000u) {
                *slot = *xr(st, rd(w));
            } else {
                *xr(st, rd(w)) = *slot;
            }
            continue;
        }

        r.status = OAH_INTERP_DECODE;
        return r;
    }
    r.status = OAH_INTERP_LIMIT;
    return r;
}
