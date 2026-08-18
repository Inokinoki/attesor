#include "oah/assembler.h"
#include "test_common.h"

#include <string.h>

static u32 last(oah_asm_buf *b)
{
    u32 insn;
    CHECK(b->size >= 4);
    memcpy(&insn, b->data + b->size - 4, 4);
    return insn;
}

int main(void)
{
    oah_asm_buf buf;
    oah_asm_buf_init(&buf);

    oah_asm_emit32(&buf, OAH_INSN_NOP);
    CHECK_EQ_U64(last(&buf), 0xd503201fu);

    CHECK(oah_emit_br(&buf, false, 16)); /* BR X16 */
    CHECK_EQ_U64(last(&buf), 0xd61f0200u); /* Rn=16 at bits 5-9: 16<<5 = 0x200 */

    CHECK(oah_emit_br(&buf, true, 30)); /* BLR X30 */
    CHECK_EQ_U64(last(&buf), 0xd63f03c0u);

    CHECK(!oah_emit_br(&buf, false, OAH_GPR_SP));

    CHECK(oah_emit_adc_sbc(&buf, false, false, false, 0, 1, 2));
    CHECK_EQ_U64(last(&buf), 0x1a020020u); /* ADC W0, W1, W2 */

    CHECK(oah_emit_adc_sbc(&buf, true, false, false, 0, 1, 2));
    CHECK_EQ_U64(last(&buf), 0x9a020020u); /* ADC X0, X1, X2 */

    CHECK(oah_emit_ldp_post(&buf, OAH_MEM_S64, 0, 1, OAH_GPR_SP));
    /* sf=1, Rt=0, Rt2=1, Rn=31, base 0x28c10000 */
    CHECK_EQ_U64(last(&buf), 0xa8c107e0u);

    CHECK(!oah_emit_ldp_post(&buf, OAH_MEM_S8, 0, 1, 2));

    CHECK(oah_emit_ldr_post(&buf, OAH_MEM_S64, 0, 5, 8));
    CHECK(!oah_emit_ldr_post(&buf, OAH_MEM_S64, 5, 5, 8)); /* Rt == Rn */

    CHECK(oah_emit_str_pre(&buf, OAH_MEM_S64, 0, OAH_GPR_SP, 0));
    /* STR X0, [SP, #0]!  size=64, Rn=31, Rt=0, pre-index */
    CHECK_EQ_U64(last(&buf), 0xf8000fe0u);

    CHECK(oah_emit_smov_64(&buf, 0, 1, 0));
    CHECK_EQ_U64(last(&buf), 0x4e042c20u);
    CHECK(!oah_emit_smov_64(&buf, 0, 1, 4));

    CHECK(oah_emit_ldr_literal_pc(&buf, 2));
    CHECK_EQ_U64(last(&buf), 0x58000002u);

    /* Grow path: emit more than 0x4000 bytes (initial cap after first grow). */
    oah_asm_buf_destroy(&buf);
    oah_asm_buf_init(&buf);
    {
        u32 i;
        for (i = 0; i < 5000; i++) {
            oah_asm_emit32(&buf, OAH_INSN_NOP);
        }
        CHECK_EQ_U64(buf.size, 5000u * 4);
        CHECK(buf.capacity >= buf.size);
    }

    CHECK(oah_emit_movz(&buf, true, 8, 0, 56));
    CHECK_EQ_U64(last(&buf), 0xd2800708u); /* wrappers: movz x8, #56 */
    CHECK(oah_emit_svc(&buf, 0));
    CHECK_EQ_U64(last(&buf), 0xd4000001u);
    CHECK(oah_emit_movz(&buf, false, 8, 0, 1));
    CHECK_EQ_U64(last(&buf), 0x52800028u); /* movz w8, #1  (RAX) */

    oah_asm_buf_destroy(&buf);
    return test_report("assembler");
}
