#include "oah/register.h"

#include <stdint.h>

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

const char *oah_register_to_string(u64 reg)
{
    /* FUN_80000008d3d0 Register.cpp:174
     * Pointer tables at PTR_DAT_800000012ea0 .. 0x13200. */
    static const char *const g8[16] = {
        "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
        "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
    };
    static const char *const gh[4] = { "ah", "ch", "dh", "bh" };
    static const char *const g16[16] = {
        "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
        "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
    };
    static const char *const g32[16] = {
        "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
        "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
    };
    static const char *const g64[16] = {
        "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };
    static const char *const xmm[16] = {
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    };
    static const char *const ymm[16] = {
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
        "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"
    };
    static const char *const mm[8] = {
        "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
    };
    static const char *const st[8] = {
        "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7"
    };

    unsigned cls = OAH_REG_CLASS(reg);
    unsigned idx = OAH_REG_INDEX(reg);

    switch (cls) {
    case 0:
        return g8[idx];
    case 1:
        if (idx > 3) {
            return 0; /* Register.cpp:174 "invalid high byte register" */
        }
        return gh[idx];
    case 2:
        return g16[idx];
    case 3:
        return g32[idx];
    case 4:
    case 0xd:
        return g64[idx];
    case 6:
        /* PTR_DAT_8000000131c0 is 16 slots: mm0-7, st0-7 */
        return (idx < 8) ? mm[idx] : st[idx - 8];
    case 7:
    case 0xe:
        if (idx > 7) {
            return 0;
        }
        return st[idx];
    case 8:
        if (idx != 0) {
            return 0; /* Register.cpp:200 "invalid register" */
        }
        return "rip";
    case 10:
        return ymm[idx];
    case 0xc:
        return (const char *)(uintptr_t)reg;
    default:
        return xmm[idx];
    }
}

const char *oah_segment_register_to_string(unsigned index)
{
    /* FUN_80000002ac58 Operand.cpp:0x3d */
    static const char *const names[6] = { "ES", "CS", "SS", "DS", "FS", "GS" };
    if (index > 5) {
        return 0;
    }
    return names[index];
}
