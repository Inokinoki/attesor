#include "oah/decode_insn.h"

#include <stdint.h>
#include <string.h>

static u8 peek(const oah_decoder *d)
{
    if (d->cursor >= d->end || (d->cursor - d->insn_start) >= OAH_X86_MAX_INSN) {
        return 0;
    }
    return *d->cursor;
}

static u8 take(oah_decoder *d)
{
    return (u8)oah_decoder_read_int(d, 0);
}

bool oah_decode_insn(const u8 *pc, const u8 *end, oah_insn *out)
{
    oah_decoder d;
    u8 rex = 0;
    u8 op, modrm, mod, reg, rm;
    int opsize;
    unsigned i;

    memset(out, 0, sizeof(*out));
    if (!pc || !end || pc >= end) {
        return false;
    }
    oah_decoder_init(&d, pc, (size_t)(end - pc));

    /* Prefixes. FUN_80000008d8c0: REX 0x40-0x4f; seg 0x26/2e/36/3e. */
    for (i = 0; i < OAH_X86_MAX_INSN; i++) {
        u8 b;
        if (d.error || d.cursor >= d.end) {
            return false;
        }
        b = peek(&d);
        if (b == 0x66 || b == 0x67 || b == 0xf0 || b == 0xf2 || b == 0xf3 ||
            b == 0x26 || b == 0x2e || b == 0x36 || b == 0x3e ||
            b == 0x64 || b == 0x65) {
            take(&d);
            continue;
        }
        if ((b & 0xf0) == 0x40) {
            rex = take(&d);
            break;
        }
        break;
    }

    opsize = (rex & 8) ? 8 : 4;
    if (d.error) {
        return false;
    }
    op = take(&d);
    if (d.error) {
        return false;
    }

    /* F3 0F 1E FA endbr64 — F3 already consumed as prefix. */
    if (op == 0x0f) {
        u8 op2 = take(&d);
        if (d.error) {
            return false;
        }
        if (op2 == 0x05) {
            out->op = OAH_OP_SYSCALL;
            goto done;
        }
        if (op2 == 0x1e && peek(&d) == 0xfa) {
            take(&d);
            out->op = OAH_OP_ENDBR64;
            goto done;
        }
        return false;
    }

    if (op == 0x90) {
        out->op = OAH_OP_NOP;
        goto done;
    }
    if (op == 0xc3) {
        out->op = OAH_OP_RET;
        goto done;
    }
    if (op >= 0x50 && op <= 0x57) {
        out->op = OAH_OP_PUSH;
        out->dst = (u8)((op - 0x50) | ((rex & 1) << 3));
        out->opsize = 8;
        goto done;
    }
    if (op >= 0x58 && op <= 0x5f) {
        out->op = OAH_OP_POP;
        out->dst = (u8)((op - 0x58) | ((rex & 1) << 3));
        out->opsize = 8;
        goto done;
    }
    if (op == 0xeb) {
        out->op = OAH_OP_JMP_REL;
        out->disp = (s32)(int8_t)take(&d);
        goto done;
    }
    if (op == 0xe9) {
        out->op = OAH_OP_JMP_REL;
        out->disp = (s32)oah_decoder_read_int(&d, 2);
        goto done;
    }
    if (op == 0xe8) {
        out->op = OAH_OP_CALL_REL;
        out->disp = (s32)oah_decoder_read_int(&d, 2);
        goto done;
    }
    if (op >= 0xb8 && op <= 0xbf) {
        out->op = OAH_OP_MOV_IMM;
        out->dst = (u8)((op - 0xb8) | ((rex & 1) << 3));
        out->opsize = (u8)opsize;
        if (opsize == 8) {
            out->imm = oah_decoder_read_int(&d, 3);
        } else {
            out->imm = oah_decoder_read_int(&d, 2); /* 32-bit, zero-extends */
        }
        goto done;
    }

    /* Need ModRM: 31 xor, 89/8B mov, 8D lea. Only mod=3 or RIP-relative. */
    if (op == 0x31 || op == 0x89 || op == 0x8b || op == 0x8d) {
        modrm = take(&d);
        if (d.error) {
            return false;
        }
        mod = (u8)(modrm >> 6);
        reg = (u8)(((modrm >> 3) & 7) | ((rex & 4) ? 8 : 0));
        rm = (u8)((modrm & 7) | ((rex & 1) ? 8 : 0));

        if (op == 0x8d) {
            /* LEA r, [rip+disp32]: mod=0, r/m=5 (unextended). */
            if (mod == 0 && (modrm & 7) == 5) {
                out->op = OAH_OP_LEA_RIP;
                out->dst = reg;
                out->opsize = (u8)opsize;
                out->disp = (s32)oah_decoder_read_int(&d, 2);
                goto done;
            }
            return false;
        }

        if (mod != 3) {
            return false;
        }
        if (op == 0x31) {
            out->op = OAH_OP_XOR_RR;
            out->dst = rm;
            out->src = reg;
            out->opsize = (u8)opsize;
            goto done;
        }
        if (op == 0x89) {
            out->op = OAH_OP_MOV_RR;
            out->dst = rm;
            out->src = reg;
            out->opsize = (u8)opsize;
            goto done;
        }
        /* 8B */
        out->op = OAH_OP_MOV_RR;
        out->dst = reg;
        out->src = rm;
        out->opsize = (u8)opsize;
        goto done;
    }

    return false;

done:
    if (d.error) {
        return false;
    }
    out->length = (u8)(d.cursor - d.insn_start);
    return out->length != 0;
}
