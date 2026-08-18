#ifndef OAH_REGISTER_H
#define OAH_REGISTER_H

#include "oah/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host GPR tagging recovered from Register.h asserts in assembler emitters.
 *
 *   gpr_to_num    (Register.h:0x33): gpr != Gpr::SP   — SP encoding is 0x3f
 *   gpr_to_num_sp (Register.h:0x38): gpr != Gpr::XZR  — XZR encoding is 0x1f
 *
 * 0x3f == 0x1f | 0x20: the low 5 bits are the architectural register number
 * (31 = SP or ZR); bit 5 distinguishes SP from XZR.
 */
#define OAH_GPR_XZR  0x1fu
#define OAH_GPR_SP   0x3fu
#define OAH_GPR_NUM_MASK 0x1fu
#define OAH_GPR_SP_BIT   0x20u

typedef u8 oah_gpr; /* tagged: 0..30, XZR=0x1f, SP=0x3f */

static inline bool oah_gpr_is_sp(oah_gpr g)
{
    return g == OAH_GPR_SP;
}

static inline bool oah_gpr_is_xzr(oah_gpr g)
{
    return g == OAH_GPR_XZR;
}

/* decomp: inlined at every assembler emit that rejects SP.
 * orig: Register.h:0x33 gpr_to_num
 * Returns 5-bit number, or false if g is SP. */
bool oah_gpr_to_num(oah_gpr g, u8 *out_num);

/* decomp: inlined at every assembler emit that rejects XZR (base registers).
 * orig: Register.h:0x38 gpr_to_num_sp
 * Returns 5-bit number (31 means SP), or false if g is XZR. */
bool oah_gpr_to_num_sp(oah_gpr g, u8 *out_num);

/*
 * Guest x86_64 GPR file layout (Linux user_regs_struct order).
 * Recovered from FUN_800000040650 copies of param_4 offsets.
 */
typedef enum {
    OAH_X86_RAX = 0,
    OAH_X86_RCX = 1,
    OAH_X86_RDX = 2,
    OAH_X86_RBX = 3,
    OAH_X86_RSP = 4,
    OAH_X86_RBP = 5,
    OAH_X86_RSI = 6,
    OAH_X86_RDI = 7,
    OAH_X86_R8  = 8,
    OAH_X86_R9  = 9,
    OAH_X86_R10 = 10,
    OAH_X86_R11 = 11,
    OAH_X86_R12 = 12,
    OAH_X86_R13 = 13,
    OAH_X86_R14 = 14,
    OAH_X86_R15 = 15,
    OAH_X86_GPR_COUNT = 16
} oah_x86_gpr;

/* ARM Xn that holds a given x86 GPR in the runtime host state.
 * derived: FUN_800000040650 (X8-X15 ← RAX-RDI, X0-X7 ← R13,R14,R12,R11,R15,R10,R9,R8) */
u8 oah_host_xn_for_x86_gpr(oah_x86_gpr g);

/*
 * orig: Register.cpp:174 register_to_string
 * decomp: FUN_80000008d3d0
 *
 * Packed encoding: (class << 4) | index, with these class nibbles:
 *   0  g8     al..r15b
 *   1  gh     ah,ch,dh,bh   (index must be < 4)
 *   2  g16    ax..r15w
 *   3  g32    eax..r15d
 *   4, 0xd    g64   rax..r15
 *   6         mm0-7 then st0-7 (index 0-15)
 *   7, 0xe    st0-7
 *   8         rip (index must be 0)
 *   10        ymm0-15
 *   0xc       the value itself is already a C string pointer
 *   default   xmm0-15
 *
 * Original asserts and does not return on a bad high-byte / RIP index.
 * This reconstruction returns NULL instead.
 */
#define OAH_REG_CLASS(r) ((unsigned)(((u64)(r) >> 4) & 0xfu))
#define OAH_REG_INDEX(r) ((unsigned)((u64)(r) & 0xfu))

static inline u64 oah_reg_encode(unsigned cls, unsigned index)
{
    return ((u64)(cls & 0xfu) << 4) | (u64)(index & 0xfu);
}

const char *oah_register_to_string(u64 reg);

/*
 * orig: Operand.cpp:0x3d segment_register_to_string
 * decomp: FUN_80000002ac58 case 6
 * Names (uppercase, as in rodata): ES CS SS DS FS GS. NULL if index >= 6.
 */
const char *oah_segment_register_to_string(unsigned index);

#ifdef __cplusplus
}
#endif

#endif /* OAH_REGISTER_H */
