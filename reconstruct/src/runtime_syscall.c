#include "oah/assembler.h"

/* Temps X16-X17, callee-saved X19-X25: not used by the runtime GPR map. */
static bool movx(oah_asm_buf *buf, oah_gpr d, oah_gpr s)
{
    return oah_emit_mov_reg(buf, true, d, s);
}

bool oah_emit_runtime_syscall(oah_asm_buf *buf)
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
        !movx(buf, 8, 17) ||
        !oah_emit_svc(buf, 0) ||
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
