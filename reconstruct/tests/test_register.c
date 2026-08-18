#include "oah/register.h"
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

    return test_report("register");
}
