#include "oah/flags.h"

bool oah_set_nzcv(u32 *cpsr, u32 nzcv)
{
    /* FUN_80000004077c ThreadState.h:0x37 */
    if ((nzcv & 0x03ffffffu) != 0) {
        return false;
    }
    *cpsr = (*cpsr & 0x03ffffffu) | nzcv;
    return true;
}

u32 oah_nzcv_from_rflags(u64 rflags)
{
    /* FUN_800000040650 */
    return ((u32)rflags & 0xc0u) << 24 |
           ((u32)~rflags & 1u) << 29 |
           (((u32)rflags >> 11) & 1u) << 28;
}

u64 oah_pack_aux_flags(u64 rflags)
{
    /* FUN_800000040650 param_3[0x11] */
    return (rflags >> 9 & 2) |
           (~rflags >> 2 & 1) |
           (rflags >> 16 & 4) |
           (rflags >> 18 & 8) |
           (rflags >> 4 & 0x10);
}

void oah_host_flags_from_rflags(oah_host_flags *out, u64 rflags)
{
    out->nzcv = oah_nzcv_from_rflags(rflags);
    out->aux_flags = oah_pack_aux_flags(rflags);
    out->trap_flag = (u8)((rflags >> 8) & 1);
}
