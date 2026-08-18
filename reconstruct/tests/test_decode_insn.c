#include "oah/decode_insn.h"
#include "test_common.h"

int main(void)
{
    {
        u8 bytes[] = {0x90};
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_NOP);
        CHECK_EQ_U64(i.length, 1);
    }
    {
        u8 bytes[] = {0x0f, 0x05};
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_SYSCALL);
        CHECK_EQ_U64(i.length, 2);
    }
    {
        u8 bytes[] = {0xf3, 0x0f, 0x1e, 0xfa};
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_ENDBR64);
        CHECK_EQ_U64(i.length, 4);
    }
    {
        u8 bytes[] = {0xb8, 0x01, 0x00, 0x00, 0x00}; /* mov eax, 1 */
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_MOV_IMM);
        CHECK_EQ_U64(i.dst, 0);
        CHECK_EQ_U64(i.opsize, 4);
        CHECK_EQ_U64(i.imm, 1);
        CHECK_EQ_U64(i.length, 5);
    }
    {
        u8 bytes[] = {0xbf, 0x01, 0x00, 0x00, 0x00}; /* mov edi, 1 */
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.dst, 7);
        CHECK_EQ_U64(i.imm, 1);
    }
    {
        u8 bytes[] = {0x31, 0xff}; /* xor edi, edi */
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_XOR_RR);
        CHECK_EQ_U64(i.dst, 7);
        CHECK_EQ_U64(i.src, 7);
        CHECK_EQ_U64(i.opsize, 4);
    }
    {
        /* lea rsi, [rip+6] */
        u8 bytes[] = {0x48, 0x8d, 0x35, 0x06, 0x00, 0x00, 0x00};
        oah_insn i;
        CHECK(oah_decode_insn(bytes, bytes + sizeof(bytes), &i));
        CHECK_EQ_U64(i.op, OAH_OP_LEA_RIP);
        CHECK_EQ_U64(i.dst, 6); /* rsi */
        CHECK_EQ_U64((u64)(s32)i.disp, 6);
        CHECK_EQ_U64(i.length, 7);
    }
    {
        u8 bytes[] = {0xc3};
        oah_insn i;
        CHECK(!oah_decode_insn(bytes, bytes, &i)); /* empty */
        CHECK(oah_decode_insn(bytes, bytes + 1, &i) && i.op == OAH_OP_RET);
    }
    {
        u8 bytes[] = {0x06}; /* 32-bit push es — invalid in long mode */
        oah_insn i;
        CHECK(!oah_decode_insn(bytes, bytes + 1, &i));
    }

    return test_report("decode_insn");
}
