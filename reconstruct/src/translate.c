#include "oah/translate.h"

#include "oah/assembler.h"
#include "oah/decode_insn.h"
#include "oah/register.h"

static oah_gpr xn(oah_x86_gpr g)
{
    return (oah_gpr)oah_host_xn_for_x86_gpr(g);
}

static bool emit_mov_imm(oah_asm_buf *buf, oah_x86_gpr g, u8 opsize, u64 imm)
{
    oah_gpr d = xn(g);
    if (opsize == 8) {
        return oah_emit_mov_u64(buf, d, imm);
    }
    /* 32-bit write zero-extends: MOVZ/MOVK Wd */
    if (!oah_emit_movz(buf, false, d, 0, (u32)(imm & 0xffffu))) {
        return false;
    }
    if ((imm >> 16) & 0xffffu) {
        return oah_emit_movk(buf, false, d, 1, (u32)((imm >> 16) & 0xffffu));
    }
    return true;
}

bool oah_translate_block(oah_asm_buf *buf, oah_image *img, u64 rip, u64 *next_rip)
{
    return oah_translate_block_flags(buf, img, rip, next_rip, 0);
}

bool oah_translate_block_flags(oah_asm_buf *buf, oah_image *img, u64 rip,
                               u64 *next_rip, u32 flags)
{
    unsigned n = 0;
    u64 cur = rip;
    int host_svc = (flags & OAH_XLAT_HOST_SVC) != 0;

    if (next_rip) {
        *next_rip = rip;
    }

    while (n < 64) {
        const u8 *pc = oah_guest_to_host(img, cur, 1);
        const u8 *end;
        oah_insn in;
        u64 after;
        bool is_64;
        oah_gpr rsp;

        if (!pc) {
            if (n == 0) {
                return false;
            }
            break;
        }
        end = pc + OAH_X86_MAX_INSN;
        if (!oah_decode_insn(pc, end, &in)) {
            if (n == 0) {
                return false;
            }
            break;
        }
        after = cur + in.length;
        is_64 = in.opsize == 8;
        rsp = xn(OAH_X86_RSP);

        switch (in.op) {
        case OAH_OP_NOP:
        case OAH_OP_ENDBR64:
            break;
        case OAH_OP_MOV_IMM:
            if (!emit_mov_imm(buf, (oah_x86_gpr)in.dst, in.opsize, in.imm)) {
                return false;
            }
            break;
        case OAH_OP_XOR_RR:
            if (!oah_emit_eor(buf, is_64, xn((oah_x86_gpr)in.dst),
                              xn((oah_x86_gpr)in.dst), xn((oah_x86_gpr)in.src))) {
                return false;
            }
            break;
        case OAH_OP_MOV_RR:
            if (!oah_emit_mov_reg(buf, is_64, xn((oah_x86_gpr)in.dst),
                                  xn((oah_x86_gpr)in.src))) {
                return false;
            }
            break;
        case OAH_OP_LEA_RIP:
            if (!oah_emit_mov_u64(buf, xn((oah_x86_gpr)in.dst),
                                  after + (u64)(s64)in.disp)) {
                return false;
            }
            break;
        case OAH_OP_PUSH:
            if (!oah_emit_add_sub_imm(buf, true, true, rsp, rsp, 8) ||
                !oah_emit_ldr_str_uoff(buf, true, xn((oah_x86_gpr)in.dst), rsp, 0)) {
                return false;
            }
            break;
        case OAH_OP_POP:
            if (!oah_emit_ldr_str_uoff(buf, false, xn((oah_x86_gpr)in.dst), rsp, 0) ||
                !oah_emit_add_sub_imm(buf, true, false, rsp, rsp, 8)) {
                return false;
            }
            break;
        case OAH_OP_SYSCALL:
            if (host_svc) {
                if (!oah_emit_runtime_syscall_host(buf)) {
                    return false;
                }
            } else if (!oah_emit_runtime_syscall(buf)) {
                return false;
            }
            break;
        case OAH_OP_JMP_REL:
            cur = after + (u64)(s64)in.disp;
            if (next_rip) {
                *next_rip = cur;
            }
            return oah_emit_ret(buf);
        case OAH_OP_CALL_REL:
        case OAH_OP_RET:
            return false;
        default:
            return false;
        }

        cur = after;
        n++;
        if (next_rip) {
            *next_rip = cur;
        }

        if (in.op == OAH_OP_SYSCALL) {
            /* Keep going; exit is detected when SVC runs. */
        }
    }
    return oah_emit_ret(buf);
}
