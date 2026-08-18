#include "oah/flags.h"
#include "oah/result.h"
#include "oah/icache.h"
#include "test_common.h"

int main(void)
{
    u32 cpsr = 0;

    CHECK(oah_set_nzcv(&cpsr, OAH_CPSR_N | OAH_CPSR_Z));
    CHECK_EQ_U64(cpsr, OAH_CPSR_N | OAH_CPSR_Z);
    CHECK(!oah_set_nzcv(&cpsr, 1u)); /* bit 0 is outside NZCV mask */

    /* SF+ZF → N+Z; CF inverted → C=0; OF=0 → V=0 */
    CHECK_EQ_U64(oah_nzcv_from_rflags(OAH_RFLAGS_SF | OAH_RFLAGS_ZF),
                 OAH_CPSR_N | OAH_CPSR_Z | OAH_CPSR_C);
    /* CF=1 → C=0 (inverted) */
    CHECK_EQ_U64(oah_nzcv_from_rflags(OAH_RFLAGS_CF) & OAH_CPSR_C, 0);
    /* CF=0 → C=1 */
    CHECK((oah_nzcv_from_rflags(0) & OAH_CPSR_C) != 0);
    /* OF → V */
    CHECK((oah_nzcv_from_rflags(OAH_RFLAGS_OF) & OAH_CPSR_V) != 0);

    {
        oah_host_flags hf;
        oah_host_flags_from_rflags(&hf, OAH_RFLAGS_TF | OAH_RFLAGS_DF | OAH_RFLAGS_AF);
        CHECK_EQ_U64(hf.trap_flag, 1);
        CHECK((hf.aux_flags & 2) != 0);  /* DF → bit 1 */
        CHECK((hf.aux_flags & 0x10) != 0); /* AF → bit 4 */
    }

    {
        oah_result ok = oah_result_from_linux_ret(7);
        CHECK(!oah_result_is_error(ok));
        CHECK_EQ_U64(ok.value, 7);

        oah_result err = oah_result_from_linux_ret((u64)-2); /* -ENOENT */
        CHECK(oah_result_is_error(err));
        CHECK_EQ_U64(err.value, 2);
    }

    CHECK_EQ_U64(oah_icache_line_count(0, 64), 1);
    CHECK_EQ_U64(oah_icache_line_count(0, 65), 2);
    CHECK_EQ_U64(oah_icache_line_count(32, 32), 1);
    CHECK_EQ_U64(oah_icache_line_count(32, 33), 2);

    return test_report("flags_result_icache");
}
