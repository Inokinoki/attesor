#include "oah/decoder.h"
#include "oah/condition.h"
#include "oah/bitmask_imm.h"
#include "test_common.h"

int main(void)
{
    /* read_int little-endian, 15-byte limit */
    {
        u8 bytes[] = {0x78, 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab, 0x90};
        oah_decoder d;
        oah_decoder_init(&d, bytes, sizeof(bytes));
        CHECK_EQ_U64(oah_decoder_read_int(&d, 0), 0x78);
        CHECK_EQ_U64(oah_decoder_read_int(&d, 1), 0x3456);
        CHECK(!d.error);
        oah_decoder_init(&d, bytes, sizeof(bytes));
        CHECK_EQ_U64(oah_decoder_read_int(&d, 2), 0x12345678u);
        oah_decoder_init(&d, bytes, sizeof(bytes));
        CHECK_EQ_U64(oah_decoder_read_int(&d, 3), 0x90abcdef12345678ULL);
        CHECK(!d.error);
        oah_decoder_init(&d, bytes, sizeof(bytes));
        (void)oah_decoder_read_int(&d, 99);
        CHECK(d.error);
    }
    {
        u8 long_insn[16];
        oah_decoder d;
        unsigned i;
        for (i = 0; i < 16; i++) {
            long_insn[i] = (u8)i;
        }
        oah_decoder_init(&d, long_insn, sizeof(long_insn));
        for (i = 0; i < 15; i++) {
            CHECK_EQ_U64(oah_decoder_read_int(&d, 0), i);
            CHECK(!d.error);
        }
        (void)oah_decoder_read_int(&d, 0); /* 16th byte: cursor-start == 15 */
        CHECK(d.error);
    }

    {
        oah_arm_cc arm;
        CHECK(oah_translate_condition_code(OAH_CC_E, &arm) && arm == OAH_ARM_EQ);
        CHECK(oah_translate_condition_code(OAH_CC_O, &arm) && arm == OAH_ARM_VS);
        CHECK(oah_translate_condition_code(OAH_CC_B, &arm) && arm == OAH_ARM_CC);
        CHECK(oah_translate_condition_code(OAH_CC_AE, &arm) && arm == OAH_ARM_CS);
        CHECK(!oah_translate_condition_code(OAH_CC_P, &arm));
        CHECK(!oah_translate_condition_code(OAH_CC_NP, &arm));
        CHECK(!oah_translate_condition_code((oah_x86_cc)16, &arm));
    }

    {
        oah_bitmask_imm imm;
        CHECK(!oah_is_bitmask_immediate(true, 0, &imm));
        CHECK(!oah_is_bitmask_immediate(true, ~0ULL, &imm));
        CHECK(oah_is_bitmask_immediate(true, 1, &imm));
        CHECK(oah_is_bitmask_immediate(false, 0xffu, &imm));
        CHECK(oah_is_bitmask_immediate(true, 0x5555555555555555ULL, &imm));
        CHECK(oah_is_bitmask_immediate(true, 0x00000000ffffffffULL, &imm));
        CHECK(!oah_is_bitmask_immediate(true, 0x0000000000000101ULL, &imm));
        /* 0x101 is two isolated bits spanning more than a simple run in 8-bit
         * elements? 0x101 = 9-bit pattern 1........1 — not a rotated run of
         * consecutive ones in 16/32/64. Should be invalid. */
    }

    return test_report("decoder_condition_bitmask");
}
