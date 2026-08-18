#ifndef OAH_FLAGS_H
#define OAH_FLAGS_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ARM NZCV occupy CPSR bits 31-28.
 * orig: ThreadState.h:0x37 set_nzcv
 *   "(nzcv & ~arm::kCpsrNZCVMask) == 0"
 * decomp: FUN_80000004077c checks (nzcv & 0x03ffffff) == 0
 * so kCpsrNZCVMask == 0xfc000000. */
#define OAH_CPSR_NZCV_MASK 0xfc000000u
#define OAH_CPSR_N (1u << 31)
#define OAH_CPSR_Z (1u << 30)
#define OAH_CPSR_C (1u << 29)
#define OAH_CPSR_V (1u << 28)

#define OAH_RFLAGS_CF (1ull << 0)
#define OAH_RFLAGS_PF (1ull << 2)
#define OAH_RFLAGS_AF (1ull << 4)
#define OAH_RFLAGS_ZF (1ull << 6)
#define OAH_RFLAGS_SF (1ull << 7)
#define OAH_RFLAGS_TF (1ull << 8)
#define OAH_RFLAGS_DF (1ull << 10)
#define OAH_RFLAGS_OF (1ull << 11)
#define OAH_RFLAGS_RF (1ull << 16)
#define OAH_RFLAGS_AC (1ull << 18)

typedef struct {
    u32 nzcv;          /* bits 31-28 */
    u64 aux_flags;     /* packed PF/AF/DF/RF/AC — see oah_pack_aux_flags */
    u8  trap_flag;     /* RFLAGS.TF (bit 8) */
} oah_host_flags;

/* orig: ThreadState.h:0x37 set_nzcv. Returns false on invalid bits. */
bool oah_set_nzcv(u32 *cpsr, u32 nzcv);

/*
 * derived: FUN_800000040650
 *   NZCV = (RFLAGS & 0xc0) << 24        // SF→N, ZF→Z
 *         | (~RFLAGS & 1) << 29         // C = NOT CF
 *         | ((RFLAGS >> 11) & 1) << 28  // OF→V
 */
u32 oah_nzcv_from_rflags(u64 rflags);

/*
 * derived: FUN_800000040650 param_3[0x11]
 *   DF bit10 → bit1, ~PF bit2 → bit0, RF bit16 → bit2,
 *   AC bit18 → bit3, AF bit4 → bit4
 */
u64 oah_pack_aux_flags(u64 rflags);

void oah_host_flags_from_rflags(oah_host_flags *out, u64 rflags);

#ifdef __cplusplus
}
#endif

#endif /* OAH_FLAGS_H */
