#include "oah/assembler.h"

bool oah_emit_br(oah_asm_buf *buf, bool link, oah_gpr rn)
{
    /* FUN_8000000280d0 */
    u8 n;
    if (!oah_gpr_to_num(rn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (u32)n << 5 | (u32)link << 21 | 0xd61f0000u);
    return true;
}

bool oah_emit_adc_sbc(oah_asm_buf *buf, bool is_64, bool is_sbc, bool set_flags,
                      oah_gpr rd, oah_gpr rn, oah_gpr rm)
{
    /* FUN_800000027914 */
    u8 d, n, m;
    if (!oah_gpr_to_num(rd, &d) || !oah_gpr_to_num(rn, &n) || !oah_gpr_to_num(rm, &m)) {
        return false;
    }
    u32 op = is_64 ? 0x9a000000u : 0x1a000000u;
    u32 s = set_flags ? 0x20000000u : 0;
    oah_asm_emit32(buf, op | (u32)is_sbc << 30 | s | (u32)m << 16 | (u32)n << 5 | d);
    return true;
}

bool oah_emit_ldp_post(oah_asm_buf *buf, oah_mem_size size,
                       oah_gpr rt, oah_gpr rt2, oah_gpr rn)
{
    /* FUN_800000028970 AssemblerBase.hpp:0x4d7
     * size == S32 (2) || S64 (3) */
    if ((size & 0xfe) != 2) {
        return false;
    }
    u8 t, t2, n;
    if (!oah_gpr_to_num(rt, &t) || !oah_gpr_to_num(rt2, &t2) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (u32)(size == OAH_MEM_S64) << 31 |
                            (u32)t2 << 10 | t | (u32)n << 5 | 0x28c10000u);
    return true;
}

bool oah_emit_stp_pre(oah_asm_buf *buf, oah_mem_size size,
                      oah_gpr rt, oah_gpr rt2, oah_gpr rn, u32 imm7)
{
    /* FUN_80000002954c AssemblerBase.hpp:0x7f5 */
    if ((size & 0xfe) != 2) {
        return false;
    }
    u8 t, t2, n;
    if (!oah_gpr_to_num(rt, &t) || !oah_gpr_to_num(rt2, &t2) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (imm7 & 0x7f) << 15 | (u32)(size == OAH_MEM_S64) << 31 |
                            (u32)t2 << 10 | t | (u32)n << 5 | 0x29800000u);
    return true;
}

bool oah_emit_ldr_post(oah_asm_buf *buf, oah_mem_size size,
                       oah_gpr rt, oah_gpr rn, s32 imm9)
{
    /* FUN_800000028c50 AssemblerBase.hpp:0x506 "Rt != Rn" */
    u8 t, n;
    if (!oah_gpr_to_num(rt, &t) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    if (t == n) {
        return false;
    }
    u32 opc = 0xc00000u;
    if (size != OAH_MEM_S128) {
        opc = 0x400000u;
    }
    /* 0x38000400 = LDR post-index; dump's 0x38000480 had Rn=X4 folded in. */
    oah_asm_emit32(buf, (t & 0x1f) | (u32)n << 5 | ((u32)imm9 & 0x1ff) << 12 |
                            (u32)size << 30 | opc | 0x38000400u);
    return true;
}

bool oah_emit_str_pre(oah_asm_buf *buf, oah_mem_size size,
                      oah_gpr rt, oah_gpr rn, s32 imm9)
{
    /* FUN_8000000296f8 AssemblerBase.hpp:0x82c "Rt != Rn" */
    u8 t, n;
    if (!oah_gpr_to_num(rt, &t) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    if (t == n) {
        return false;
    }
    /* 0x38000c00 = STR pre-index; dump's 0x38000c80 had Rn=X4 folded in. */
    oah_asm_emit32(buf, (u32)size << 30 | (t & 0x1f) | (u32)n << 5 |
                            ((u32)imm9 & 0x1ff) << 12 |
                            (u32)(size == OAH_MEM_S128) << 23 | 0x38000c00u);
    return true;
}

bool oah_emit_smov_64(oah_asm_buf *buf, oah_gpr rd, oah_gpr vn, u32 index)
{
    /* FUN_8000000297e4 AssemblerBase.hpp:0x894
     * "(index & 0b11) == index" */
    if ((index & 0xff) > 3) {
        return false;
    }
    u8 d, n;
    if (!oah_gpr_to_num(rd, &d) || !oah_gpr_to_num(vn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (index & 3) << 19 | d | (u32)n << 5 | 0x4e042c00u);
    return true;
}

bool oah_emit_ushr_vec(oah_asm_buf *buf, u32 elem_size_log2,
                       oah_gpr vd, oah_gpr vn, s32 shift)
{
    /* FUN_800000029a00 AssemblerBase.hpp:0x982 "immh != 0" */
    u32 uVar2 = 1u << (elem_size_log2 & 0x1f);
    u32 uVar1 = (uVar2 - 1) & (((u32)(-shift) & 0xf8u) >> 3);
    if (uVar1 == 0 && (uVar2 & 0xff) == 0) {
        return false;
    }
    u8 d, n;
    if (!oah_gpr_to_num(vd, &d) || !oah_gpr_to_num(vn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, d | (u32)n << 5 | ((u32)(-shift) & 7u) << 16 |
                            ((uVar1 | uVar2) & 0xf) << 19 | 0x6f000400u);
    return true;
}

bool oah_emit_ldr_literal_pc(oah_asm_buf *buf, oah_gpr rt)
{
    /* FUN_800000028c04 */
    u8 t;
    if (!oah_gpr_to_num(rt, &t)) {
        return false;
    }
    oah_asm_emit32(buf, t | 0x58000000u);
    return true;
}
