#include "oah/condition.h"

/* high: DAT_800000012e1c */
const u8 oah_cc_arm_table[16] = {
    6, 7, 3, 2, 0, 1, 9, 8, 4, 5, 6, 7, 11, 10, 13, 12
};

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

    *out = (oah_arm_cc)oah_cc_arm_table[cc];
    return true;
}

const char *oah_condition_code_to_string(oah_x86_cc cc)
{
    /* FUN_80000002ac58 Operand.cpp:0x2c — PTR_DAT_800000013240 */
    static const char *const names[16] = {
        "o", "no", "b", "ae", "e", "ne", "be", "a",
        "s", "ns", "p", "np", "l", "ge", "le", "g"
    };
    if ((unsigned)cc > 15) {
        return 0;
    }
    return names[cc];
}
