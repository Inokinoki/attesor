#include "oah/interp.h"
#include "oah/decode_insn.h"

static u64 mask_for(u8 opsize)
{
    if (opsize == 1) {
        return 0xffull;
    }
    if (opsize == 2) {
        return 0xffffull;
    }
    if (opsize == 4) {
        return 0xffffffffull;
    }
    return ~0ull;
}

static void write_gpr(oah_guest *g, u8 r, u8 opsize, u64 val)
{
    if (opsize == 4) {
        g->gpr[r] = val & 0xffffffffull; /* 32-bit writes zero-extend */
    } else if (opsize == 8) {
        g->gpr[r] = val;
    } else {
        g->gpr[r] = (g->gpr[r] & ~mask_for(opsize)) | (val & mask_for(opsize));
    }
}

static bool push64(oah_guest *g, oah_image *img, u64 val)
{
    u64 rsp = g->gpr[OAH_X86_RSP] - 8;
    u64 *slot = (u64 *)oah_guest_to_host(img, rsp, 8);
    if (!slot) {
        return false;
    }
    *slot = val;
    g->gpr[OAH_X86_RSP] = rsp;
    return true;
}

static bool pop64(oah_guest *g, oah_image *img, u8 dst)
{
    u64 *slot = (u64 *)oah_guest_to_host(img, g->gpr[OAH_X86_RSP], 8);
    if (!slot) {
        return false;
    }
    g->gpr[dst] = *slot;
    g->gpr[OAH_X86_RSP] += 8;
    return true;
}

oah_interp_result oah_interp_run(oah_guest *g, oah_image *img, u64 max_insns)
{
    oah_interp_result r;
    r.status = OAH_INTERP_OK;
    r.exit_code = 0;
    r.insns = 0;

    if (max_insns == 0) {
        max_insns = OAH_INTERP_MAX_INSNS;
    }

    while (r.insns < max_insns) {
        const u8 *pc;
        const u8 *end;
        oah_insn insn;
        u64 next;
        oah_sys_status ss;

        pc = (const u8 *)oah_guest_to_host(img, g->rip, 1);
        if (!pc) {
            r.status = OAH_INTERP_FAULT;
            return r;
        }
        /* Allow decode up to 15 bytes or end of that segment. */
        end = pc + OAH_X86_MAX_INSN;
        {
            unsigned i;
            for (i = 0; i < img->nseg; i++) {
                u64 s = img->segs[i].gva;
                u64 e = s + img->segs[i].size;
                if (g->rip >= s && g->rip < e) {
                    u64 remain = e - g->rip;
                    if (remain < OAH_X86_MAX_INSN) {
                        end = pc + remain;
                    }
                    break;
                }
            }
        }

        if (!oah_decode_insn(pc, end, &insn)) {
            r.status = OAH_INTERP_DECODE;
            return r;
        }

        next = g->rip + insn.length;
        r.insns++;

        switch (insn.op) {
        case OAH_OP_NOP:
        case OAH_OP_ENDBR64:
            g->rip = next;
            break;
        case OAH_OP_MOV_IMM:
            write_gpr(g, insn.dst, insn.opsize, insn.imm);
            g->rip = next;
            break;
        case OAH_OP_XOR_RR: {
            u64 v = g->gpr[insn.dst] ^ g->gpr[insn.src];
            write_gpr(g, insn.dst, insn.opsize, v);
            g->rip = next;
            break;
        }
        case OAH_OP_MOV_RR:
            write_gpr(g, insn.dst, insn.opsize, g->gpr[insn.src]);
            g->rip = next;
            break;
        case OAH_OP_LEA_RIP:
            write_gpr(g, insn.dst, 8, next + (u64)(s64)insn.disp);
            g->rip = next;
            break;
        case OAH_OP_PUSH:
            if (!push64(g, img, g->gpr[insn.dst])) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            g->rip = next;
            break;
        case OAH_OP_POP:
            if (!pop64(g, img, insn.dst)) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            g->rip = next;
            break;
        case OAH_OP_JMP_REL:
            g->rip = next + (u64)(s64)insn.disp;
            break;
        case OAH_OP_CALL_REL:
            if (!push64(g, img, next)) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            g->rip = next + (u64)(s64)insn.disp;
            break;
        case OAH_OP_RET: {
            u64 *slot = (u64 *)oah_guest_to_host(img, g->gpr[OAH_X86_RSP], 8);
            if (!slot) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            g->rip = *slot;
            g->gpr[OAH_X86_RSP] += 8;
            break;
        }
        case OAH_OP_SYSCALL:
            g->rip = next;
            ss = oah_guest_syscall(g, img);
            if (ss == OAH_SYS_EXIT) {
                r.status = OAH_INTERP_EXIT;
                r.exit_code = (int)(g->gpr[OAH_X86_RDI] & 0xff);
                return r;
            }
            if (ss == OAH_SYS_FATAL) {
                r.status = OAH_INTERP_FAULT;
                return r;
            }
            break;
        default:
            r.status = OAH_INTERP_DECODE;
            return r;
        }
    }

    r.status = OAH_INTERP_LIMIT;
    return r;
}
