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
 *   otherwise table lookup at DAT_800000012e1c
 *
 * The 16-byte table is copied from the ELF (.rodata at VA 0x800000012e1c):
 *   06 07 03 02 00 01 09 08 04 05 06 07 0b 0a 0d 0c
 * i.e. O→VS, NO→VC, B→CC, AE→CS, E→EQ, NE→NE, BE→LS, A→HI,
 *      S→MI, NS→PL, P→VS, NP→VC, L→LT, GE→GE, LE→LE, G→GT.
 *
 * P/NP slots are VS/VC (same as O/NO). The translator still rejects those
 * codes before the lookup; the bytes are kept so a binary dump can memcmp.
 */
extern const u8 oah_cc_arm_table[16];

bool oah_translate_condition_code(oah_x86_cc cc, oah_arm_cc *out);

/*
 * orig: Operand.cpp:0x2c condition_code_to_string
 * decomp: FUN_80000002ac58 case 5, table PTR_DAT_800000013240
 * Names: o,no,b,ae,e,ne,be,a,s,ns,p,np,l,ge,le,g
 * Returns NULL if cc >= 16 (original asserts).
 */
const char *oah_condition_code_to_string(oah_x86_cc cc);

#ifdef __cplusplus
}
#endif

#endif /* OAH_CONDITION_H */
