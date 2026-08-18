#include "oah/register.h"

#include <stdint.h>

#include "test_common.h"

int main(void)
{
    u8 n;

    CHECK(oah_gpr_to_num(0, &n) && n == 0);
    CHECK(oah_gpr_to_num(30, &n) && n == 30);
    CHECK(oah_gpr_to_num(OAH_GPR_XZR, &n) && n == 31);
    CHECK(!oah_gpr_to_num(OAH_GPR_SP, &n));

    CHECK(oah_gpr_to_num_sp(OAH_GPR_SP, &n) && n == 31);
    CHECK(oah_gpr_to_num_sp(0, &n) && n == 0);
    CHECK(!oah_gpr_to_num_sp(OAH_GPR_XZR, &n));

    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RAX), 8);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RCX), 9);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RDX), 10);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RBX), 11);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RSP), 12);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RBP), 13);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RSI), 14);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_RDI), 15);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R8), 7);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R9), 6);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R10), 5);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R11), 3);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R12), 2);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R13), 0);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R14), 1);
    CHECK_EQ_U64(oah_host_xn_for_x86_gpr(OAH_X86_R15), 4);

    CHECK_STREQ(oah_register_to_string(oah_reg_encode(0, 0)), "al");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(0, 15)), "r15b");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(1, 0)), "ah");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(1, 3)), "bh");
    CHECK(oah_register_to_string(oah_reg_encode(1, 4)) == 0);
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(2, 4)), "sp");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(3, 0)), "eax");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(4, 0)), "rax");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(0xd, 15)), "r15");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(5, 0)), "xmm0");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(9, 15)), "xmm15"); /* default */
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(6, 0)), "mm0");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(6, 8)), "st0");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(6, 15)), "st7");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(7, 3)), "st3");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(0xe, 7)), "st7");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(8, 0)), "rip");
    CHECK(oah_register_to_string(oah_reg_encode(8, 1)) == 0);
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(10, 0)), "ymm0");
    CHECK_STREQ(oah_register_to_string(oah_reg_encode(10, 15)), "ymm15");
    CHECK_EQ_U64((u64)(uintptr_t)oah_register_to_string(0xc0), 0xc0);

    CHECK_STREQ(oah_segment_register_to_string(0), "ES");
    CHECK_STREQ(oah_segment_register_to_string(1), "CS");
    CHECK_STREQ(oah_segment_register_to_string(5), "GS");
    CHECK(oah_segment_register_to_string(6) == 0);

    return test_report("register");
}
