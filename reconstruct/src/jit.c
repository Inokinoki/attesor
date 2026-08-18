#include "oah/translate.h"

#include "oah/arm_exec.h"
#include "oah/assembler.h"

#include <string.h>

oah_interp_result oah_jit_run(oah_guest *g, oah_image *img, u64 max_insns)
{
    oah_interp_result r, one;
    oah_arm_state st;
    u64 budget = max_insns ? max_insns : OAH_INTERP_MAX_INSNS;

    r.status = OAH_INTERP_OK;
    r.exit_code = 0;
    r.insns = 0;
    memset(&st, 0, sizeof(st));

    while (r.insns < budget) {
        oah_asm_buf buf;
        u64 next = g->rip;

        oah_asm_buf_init(&buf);
        oah_pack_runtime_x(st.x, g);
        st.x[30] = 0;
        if (!oah_translate_block(&buf, img, g->rip, &next)) {
            oah_asm_buf_destroy(&buf);
            r.status = OAH_INTERP_DECODE;
            return r;
        }
        one = oah_arm_exec(&st, buf.data, buf.size, img, budget - r.insns);
        oah_unpack_runtime_x(g, st.x);
        oah_asm_buf_destroy(&buf);
        r.insns += one.insns;

        if (one.status == OAH_INTERP_EXIT) {
            r.status = OAH_INTERP_EXIT;
            r.exit_code = one.exit_code;
            return r;
        }
        if (one.status != OAH_INTERP_OK) {
            r.status = one.status;
            return r;
        }
        g->rip = next;
        if (one.insns == 0) {
            r.status = OAH_INTERP_FAULT;
            return r;
        }
    }
    r.status = OAH_INTERP_LIMIT;
    return r;
}
