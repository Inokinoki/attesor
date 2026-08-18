#include "oah/arm_exec.h"
#include "oah/assembler.h"
#include "test_common.h"

#include <string.h>

int main(void)
{
    oah_arm_state st;
    oah_asm_buf buf;
    oah_interp_result r;

    memset(&st, 0, sizeof(st));
    st.x[0] = 1;
    st.x[8] = 7;
    oah_asm_buf_init(&buf);
    CHECK(oah_emit_movz(&buf, true, 8, 0, 42));
    CHECK(oah_emit_ret(&buf));
    r = oah_arm_exec_native(&st, buf.data, buf.size);
#ifdef __aarch64__
    CHECK_EQ_U64(r.status, OAH_INTERP_OK);
    CHECK_EQ_U64(st.x[8], 42);
    CHECK_EQ_U64(st.x[0], 1); /* trampoline must restore untouched GPRs */
#else
    CHECK_EQ_U64(r.status, OAH_INTERP_DECODE);
    CHECK_EQ_U64(st.x[8], 7);
#endif
    oah_asm_buf_destroy(&buf);
    return test_report("arm_native");
}
