#include "oah/register.h"

bool oah_gpr_to_num(oah_gpr g, u8 *out_num)
{
    /* Register.h:0x33 gpr_to_num — "gpr != Gpr::SP" */
    if (oah_gpr_is_sp(g)) {
        return false;
    }
    *out_num = (u8)(g & OAH_GPR_NUM_MASK);
    return true;
}

bool oah_gpr_to_num_sp(oah_gpr g, u8 *out_num)
{
    /* Register.h:0x38 gpr_to_num_sp — "gpr != Gpr::XZR" */
    if (oah_gpr_is_xzr(g)) {
        return false;
    }
    *out_num = (u8)(g & OAH_GPR_NUM_MASK);
    return true;
}

u8 oah_host_xn_for_x86_gpr(oah_x86_gpr g)
{
    /* FUN_800000040650:
     *   X8..X15 = memmove from guest+0 (RAX..RDI)
     *   X0 = guest+0x68 (R13), X1 = +0x70 (R14), X2 = +0x60 (R12),
     *   X3 = +0x58 (R11), X4 = +0x78 (R15), X5 = +0x50 (R10),
     *   X6 = +0x48 (R9),  X7 = +0x40 (R8)
     */
    static const u8 map[16] = {
        /* RAX RCX RDX RBX RSP RBP RSI RDI */
        8, 9, 10, 11, 12, 13, 14, 15,
        /* R8 R9 R10 R11 R12 R13 R14 R15 */
        7, 6, 5, 3, 2, 0, 1, 4
    };
    if ((unsigned)g >= 16) {
        return 0xff;
    }
    return map[g];
}
