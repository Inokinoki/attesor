#ifndef OAH_BITMASK_IMM_H
#define OAH_BITMASK_IMM_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u8 N;    /* bit 22 of logical-immediate encoding */
    u8 imms;
    u8 immr;
} oah_bitmask_imm;

/*
 * orig: AssemblerHelpers.cpp:0xab is_bitmask_immediate
 * decomp: FUN_80000002a6bc
 *   param_1==1 → 64-bit, else 32-bit
 *   out[0]=N, out[1]=imms, out[2]=immr  (byte packing in dump)
 *
 * 0 and all-ones are not valid bitmask immediates.
 */
bool oah_is_bitmask_immediate(bool is_64, u64 value, oah_bitmask_imm *out);

#ifdef __cplusplus
}
#endif

#endif /* OAH_BITMASK_IMM_H */
