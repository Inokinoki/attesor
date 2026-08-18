#include "oah/result.h"

oah_result oah_result_from_linux_ret(u64 raw)
{
    /* FUN_800000026cb0 (and the identical wrappers around it):
     *   CallSupervisor(0);
     *   uVar1 = -param_1;
     *   if (0xfffffffffffff000 >= param_1) uVar1 = param_1;
     *   auVar2[8] = 0xfffffffffffff000 < param_1;
     *   auVar2._0_8_ = uVar1;
     */
    oah_result r;
    if (raw > OAH_LINUX_ERRNO_LIMIT) {
        r.value = (u64)(-(s64)raw);
        r.is_error = 1;
    } else {
        r.value = raw;
        r.is_error = 0;
    }
    return r;
}
