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

bool oah_emit_movz(oah_asm_buf *buf, bool is_64, oah_gpr rd, u32 hw, u32 imm16)
{
    u8 d;
    if (hw > 3 || !oah_gpr_to_num(rd, &d)) {
        return false;
    }
    oah_asm_emit32(buf, (is_64 ? 0xd2800000u : 0x52800000u) |
                            (hw & 3) << 21 | (imm16 & 0xffffu) << 5 | d);
    return true;
}

bool oah_emit_movk(oah_asm_buf *buf, bool is_64, oah_gpr rd, u32 hw, u32 imm16)
{
    u8 d;
    if (hw > 3 || !oah_gpr_to_num(rd, &d)) {
        return false;
    }
    oah_asm_emit32(buf, (is_64 ? 0xf2800000u : 0x72800000u) |
                            (hw & 3) << 21 | (imm16 & 0xffffu) << 5 | d);
    return true;
}

bool oah_emit_mov_reg(oah_asm_buf *buf, bool is_64, oah_gpr rd, oah_gpr rm)
{
    u8 d, m;
    if (!oah_gpr_to_num(rd, &d) || !oah_gpr_to_num(rm, &m)) {
        return false;
    }
    /* ORR Rd, XZR, Rm */
    oah_asm_emit32(buf, (is_64 ? 0xaa0003e0u : 0x2a0003e0u) | (u32)m << 16 | d);
    return true;
}

bool oah_emit_eor(oah_asm_buf *buf, bool is_64, oah_gpr rd, oah_gpr rn, oah_gpr rm)
{
    u8 d, n, m;
    if (!oah_gpr_to_num(rd, &d) || !oah_gpr_to_num(rn, &n) || !oah_gpr_to_num(rm, &m)) {
        return false;
    }
    oah_asm_emit32(buf, (is_64 ? 0xca000000u : 0x4a000000u) |
                            (u32)m << 16 | (u32)n << 5 | d);
    return true;
}

bool oah_emit_add_sub_imm(oah_asm_buf *buf, bool is_64, bool is_sub,
                          oah_gpr rd, oah_gpr rn, u32 imm12)
{
    u8 d, n;
    if (imm12 > 0xfff || !oah_gpr_to_num(rd, &d) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (is_64 ? 0x91000000u : 0x11000000u) |
                            (u32)is_sub << 30 | (imm12 & 0xfffu) << 10 |
                            (u32)n << 5 | d);
    return true;
}

bool oah_emit_ldr_str_uoff(oah_asm_buf *buf, bool is_store,
                           oah_gpr rt, oah_gpr rn, u32 imm_bytes)
{
    u8 t, n;
    if ((imm_bytes & 7) != 0 || imm_bytes > (0xfffu << 3) ||
        !oah_gpr_to_num(rt, &t) || !oah_gpr_to_num_sp(rn, &n)) {
        return false;
    }
    oah_asm_emit32(buf, (is_store ? 0xf9000000u : 0xf9400000u) |
                            (imm_bytes >> 3) << 10 | (u32)n << 5 | t);
    return true;
}

bool oah_emit_svc(oah_asm_buf *buf, u16 imm)
{
    oah_asm_emit32(buf, 0xd4000001u | ((u32)imm << 5));
    return true;
}

bool oah_emit_ret(oah_asm_buf *buf)
{
    oah_asm_emit32(buf, 0xd65f03c0u);
    return true;
}

bool oah_emit_mov_u64(oah_asm_buf *buf, oah_gpr rd, u64 val)
{
    u32 hw;
    if (!oah_emit_movz(buf, true, rd, 0, (u32)(val & 0xffffu))) {
        return false;
    }
    for (hw = 1; hw < 4; hw++) {
        u32 bits = (u32)((val >> (hw * 16)) & 0xffffu);
        if (bits != 0 && !oah_emit_movk(buf, true, rd, hw, bits)) {
            return false;
        }
    }
    return true;
}
