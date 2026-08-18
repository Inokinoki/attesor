#ifndef OAH_DECODE_INSN_H
#define OAH_DECODE_INSN_H

#include "oah/decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OAH_OP_INVALID = 0,
    OAH_OP_NOP,
    OAH_OP_ENDBR64,
    OAH_OP_SYSCALL,
    OAH_OP_RET,
    OAH_OP_MOV_IMM,     /* rST = imm; opsize 4 zero-extends, 8 is imm64 */
    OAH_OP_XOR_RR,      /* rST ^= rSRC, opsize */
    OAH_OP_MOV_RR,      /* rST = rSRC */
    OAH_OP_LEA_RIP,     /* rST = rip_after + disp32 */
    OAH_OP_PUSH,
    OAH_OP_POP,
    OAH_OP_JMP_REL,
    OAH_OP_CALL_REL
} oah_op;

typedef struct {
    oah_op op;
    u8  length;
    u8  opsize; /* 1, 2, 4, or 8 */
    u8  dst;    /* gpr 0..15 */
    u8  src;
    u64 imm;
    s32 disp;
} oah_insn;

/*
 * Long-mode decoder for the nolibc vertical slice.
 * Prefix handling follows FUN_80000008d8c0 (REX 4x, seg 26/2e/36/3e).
 * Uses Decoder.cpp read_int for immediates (15-byte limit).
 */
bool oah_decode_insn(const u8 *pc, const u8 *end, oah_insn *out);

#ifdef __cplusplus
}
#endif

#endif /* OAH_DECODE_INSN_H */
