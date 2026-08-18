#include "oah/assembler.h"
#include "oah/syscall_bridge.h"

/* Temps X16-X17, callee-saved X19-X25: not used by the runtime GPR map. */
static bool movx(oah_asm_buf *buf, oah_gpr d, oah_gpr s)
{
    return oah_emit_mov_reg(buf, true, d, s);
}

/*
 * After the shuffle, X8 holds the x86_64 NR. Native `svc #0` on Linux
 * aarch64 needs the asm-generic number. Compare/branch so we do not
 * clobber the saved guest GPRs in X16/X19–X25.
 *
 * Encodings checked with aarch64-linux-gnu-as:
 *   cmp x8, #1     = f100051f
 *   b.ne .+12      = 54000061
 *   b .+4          = 14000001  (fall into the next insn)
 */
static bool emit_x86_nr_to_aarch64(oah_asm_buf *buf)
{
    static const struct {
        u32 x86;
        u32 arm;
    } map[] = {
        {OAH_X86_NR_READ, 63},
        {OAH_X86_NR_WRITE, 64},
        {OAH_X86_NR_CLOSE, 57},
        {OAH_X86_NR_BRK, 214},
        {OAH_X86_NR_EXIT, 93},
        {OAH_X86_NR_UNAME, 160},
        {OAH_X86_NR_EXIT_GROUP, 94},
        {OAH_X86_NR_OPENAT, 56},
        {OAH_X86_NR_GETRANDOM, 278},
    };
    unsigned i;
    unsigned n = (unsigned)(sizeof(map) / sizeof(map[0]));

    for (i = 0; i < n; i++) {
        u32 rest;
        if (map[i].x86 > 0xfffu || map[i].arm > 0xffffu) {
            return false;
        }
        /* cmp x8, #x86  — SUBS XZR, X8, #imm12 */
        oah_asm_emit32(buf, 0xf100001fu | (map[i].x86 << 10) | (8u << 5));
        /* b.ne #12 — skip movz and b */
        oah_asm_emit32(buf, 0x54000061u);
        if (!oah_emit_movz(buf, true, 8, 0, map[i].arm)) {
            return false;
        }
        /* remaining cases are 4 insns each; +1 lands on svc */
        rest = 1u + 4u * (n - 1u - i);
        oah_asm_emit32(buf, 0x14000000u | rest);
    }
    return true;
}

static bool emit_shuffle_svc(oah_asm_buf *buf, int remap_to_aarch64)
{
    /*
     * derived from FUN_800000040650 host map + Linux AArch64 SVC ABI.
     *
     * Incoming: X8=RAX … X15=RDI, X0=R13, X7=R8, X6=R9, X5=R10, X4=R15, …
     * Linux SVC: X8=nr, X0..X5 = rdi,rsi,rdx,r10,r8,r9
     *
     * Original `runtime_syscall` at 0x8000000964b8 starts with a similar
     * shuffle then `svc #0`. We do not copy those bytes; we emit the
     * mapping we actually recovered.
     */
    if (!movx(buf, 19, 1) ||  /* x19 = R14 */
        !movx(buf, 20, 2) ||  /* x20 = R12 */
        !movx(buf, 21, 3) ||  /* x21 = R11 */
        !movx(buf, 22, 4) ||  /* x22 = R15 */
        !movx(buf, 23, 5) ||  /* x23 = R10 */
        !movx(buf, 24, 6) ||  /* x24 = R9  */
        !movx(buf, 25, 7) ||  /* x25 = R8  */
        !movx(buf, 16, 0) ||  /* x16 = R13 */
        !movx(buf, 17, 8) ||  /* x17 = RAX/nr */
        !movx(buf, 0, 15) ||
        !movx(buf, 1, 14) ||
        !movx(buf, 2, 10) ||
        !movx(buf, 3, 23) ||
        !movx(buf, 4, 25) ||
        !movx(buf, 5, 24) ||
        !movx(buf, 8, 17)) {
        return false;
    }
    if (remap_to_aarch64 && !emit_x86_nr_to_aarch64(buf)) {
        return false;
    }
    if (!oah_emit_svc(buf, 0) ||
        !movx(buf, 8, 0) ||   /* RAX = result */
        !movx(buf, 0, 16) ||
        !movx(buf, 1, 19) ||
        !movx(buf, 2, 20) ||
        !movx(buf, 3, 21) ||
        !movx(buf, 4, 22) ||
        !movx(buf, 5, 23) ||
        !movx(buf, 6, 24) ||
        !movx(buf, 7, 25)) {
        return false;
    }
    return true;
}

bool oah_emit_runtime_syscall(oah_asm_buf *buf)
{
    return emit_shuffle_svc(buf, 0);
}

bool oah_emit_runtime_syscall_host(oah_asm_buf *buf)
{
    return emit_shuffle_svc(buf, 1);
}
