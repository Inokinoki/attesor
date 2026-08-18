#ifndef OAH_CONDITION_H
#define OAH_CONDITION_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * orig: Translator.h:0x264 / 0x266 translate_condition_code
 * decomp: FUN_800000057bc8
 *
 *   cc < 16
 *   cc != P && cc != NP   ("parity flag conditions must be special-cased")
 *   otherwise table lookup at DAT_800000012e1c (rodata, not in the C dump)
 *
 * The table below is DERIVED from FUN_800000040650 storing C = ~CF, which
 * makes x86 B/AE/BE/A map to ARM CC/CS/LS/HI. It is not copied from rodata.
 */
bool oah_translate_condition_code(oah_x86_cc cc, oah_arm_cc *out);

#ifdef __cplusplus
}
#endif

#endif /* OAH_CONDITION_H */
