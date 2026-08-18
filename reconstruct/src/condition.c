#include "oah/condition.h"

bool oah_translate_condition_code(oah_x86_cc cc, oah_arm_cc *out)
{
    /* FUN_800000057bc8 Translator.h:0x264 / 0x266 */
    if ((u8)cc > 15) {
        return false;
    }
    if (((u8)cc & 0xfe) == 10) {
        /* P and NP must be special-cased (no direct ARM cond). */
        return false;
    }

    /* Derived from C = ~CF in FUN_800000040650. Original table is rodata. */
    static const oah_arm_cc table[16] = {
        OAH_ARM_VS, /* O  */
        OAH_ARM_VC, /* NO */
        OAH_ARM_CC, /* B  (CF=1, stored C=0) */
        OAH_ARM_CS, /* AE */
        OAH_ARM_EQ, /* E  */
        OAH_ARM_NE, /* NE */
        OAH_ARM_LS, /* BE */
        OAH_ARM_HI, /* A  */
        OAH_ARM_MI, /* S  */
        OAH_ARM_PL, /* NS */
        OAH_ARM_AL, /* P  — unreachable */
        OAH_ARM_AL, /* NP — unreachable */
        OAH_ARM_LT, /* L  */
        OAH_ARM_GE, /* GE */
        OAH_ARM_LE, /* LE */
        OAH_ARM_GT  /* G  */
    };
    *out = table[cc];
    return true;
}
