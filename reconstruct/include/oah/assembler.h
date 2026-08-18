#ifndef OAH_ASSEMBLER_H
#define OAH_ASSEMBLER_H

#include "oah/assembler_buffer.h"
#include "oah/register.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ARM64 NOP. Used as padding (0xd503201f) in TranslatorBase.hpp fixup loops. */
#define OAH_INSN_NOP 0xd503201fu

/*
 * Emitters below copy the bit formulas from the dump. They return false when
 * the original would have aborted (SP/XZR used in the wrong context, or an
 * AssemblerBase.hpp assert).
 */

/* decomp: FUN_8000000280d0
 * BR (link=0) / BLR (link=1): 0xd61f0000 | (link<<21) | (Rn<<5) */
bool oah_emit_br(oah_asm_buf *buf, bool link, oah_gpr rn);

/* decomp: FUN_800000027914
 * ADC/SBC family: 0x1a000000 / 0x9a000000 | (S<<29) | (op<<30) | Rm<<16|Rn<<5|Rd
 * is_64 selects 0x9a vs 0x1a (sf). op=1 is SBC. set_flags is bit 29. */
bool oah_emit_adc_sbc(oah_asm_buf *buf, bool is_64, bool is_sbc, bool set_flags,
                      oah_gpr rd, oah_gpr rn, oah_gpr rm);

/* orig: AssemblerBase.hpp:0x4d7 ldp_post
 * decomp: FUN_800000028970
 * size is S32 or S64. Immediate is implicit +2 elements (pair writeback). */
bool oah_emit_ldp_post(oah_asm_buf *buf, oah_mem_size size,
                       oah_gpr rt, oah_gpr rt2, oah_gpr rn);

/* orig: AssemblerBase.hpp:0x7f5 stp_pre
 * decomp: FUN_80000002954c */
bool oah_emit_stp_pre(oah_asm_buf *buf, oah_mem_size size,
                      oah_gpr rt, oah_gpr rt2, oah_gpr rn, u32 imm7);

/* orig: AssemblerBase.hpp:0x506 ldr_post  "Rt != Rn"
 * decomp: FUN_800000028c50 (Rn was constant-folded to X4 at this call site;
 *          the API takes Rn, matching the assert). */
bool oah_emit_ldr_post(oah_asm_buf *buf, oah_mem_size size,
                       oah_gpr rt, oah_gpr rn, s32 imm9);

/* orig: AssemblerBase.hpp:0x82c str_pre  "Rt != Rn"
 * decomp: FUN_8000000296f8 */
bool oah_emit_str_pre(oah_asm_buf *buf, oah_mem_size size,
                      oah_gpr rt, oah_gpr rn, s32 imm9);

/* orig: AssemblerBase.hpp:0x894 smov_64
 * decomp: FUN_8000000297e4  index must fit in 2 bits. */
bool oah_emit_smov_64(oah_asm_buf *buf, oah_gpr rd, oah_gpr vn, u32 index);

/* orig: AssemblerBase.hpp:0x982 ushr_vec  "immh != 0"
 * decomp: FUN_800000029a00 */
bool oah_emit_ushr_vec(oah_asm_buf *buf, u32 elem_size_log2,
                       oah_gpr vd, oah_gpr vn, s32 shift);

/* decomp: FUN_800000028c04 — LDR (literal), imm19 = 0, 64-bit. */
bool oah_emit_ldr_literal_pc(oah_asm_buf *buf, oah_gpr rt);

#ifdef __cplusplus
}
#endif

#endif /* OAH_ASSEMBLER_H */
