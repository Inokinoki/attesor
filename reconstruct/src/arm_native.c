#include "oah/arm_exec.h"

#include "oah/icache.h"

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __aarch64__
/*
 * AAPCS: x0 = ctx (u64[32]), x1 = fragment. Fragment RET returns here.
 * Guest RSP lives in X12 (runtime map), not host SP. Shuffle temps are
 * X16–X17 and X19–X25; keep the ctx pointer on the trampoline stack.
 *
 * Written as module-level asm — GCC for aarch64 ignores `naked`.
 */
__asm__(
    ".text\n"
    ".align 2\n"
    ".type oah_arm_native_enter, %function\n"
    ".global oah_arm_native_enter\n"
    "oah_arm_native_enter:\n"
    "    sub sp, sp, #128\n"
    "    stp x29, x30, [sp]\n"
    "    stp x18, x19, [sp, #16]\n"
    "    stp x20, x21, [sp, #32]\n"
    "    stp x22, x23, [sp, #48]\n"
    "    stp x24, x25, [sp, #64]\n"
    "    stp x26, x27, [sp, #80]\n"
    "    stp x28, xzr, [sp, #96]\n"
    "    str x0, [sp, #112]\n"
    "    mov x16, x0\n"
    "    mov x17, x1\n"
    "    ldp x0, x1, [x16, #0]\n"
    "    ldp x2, x3, [x16, #16]\n"
    "    ldp x4, x5, [x16, #32]\n"
    "    ldp x6, x7, [x16, #48]\n"
    "    ldp x8, x9, [x16, #64]\n"
    "    ldp x10, x11, [x16, #80]\n"
    "    ldp x12, x13, [x16, #96]\n"
    "    ldp x14, x15, [x16, #112]\n"
    "    blr x17\n"
    "    ldr x16, [sp, #112]\n"
    "    stp x0, x1, [x16, #0]\n"
    "    stp x2, x3, [x16, #16]\n"
    "    stp x4, x5, [x16, #32]\n"
    "    stp x6, x7, [x16, #48]\n"
    "    stp x8, x9, [x16, #64]\n"
    "    stp x10, x11, [x16, #80]\n"
    "    stp x12, x13, [x16, #96]\n"
    "    stp x14, x15, [x16, #112]\n"
    "    ldp x29, x30, [sp]\n"
    "    ldp x18, x19, [sp, #16]\n"
    "    ldp x20, x21, [sp, #32]\n"
    "    ldp x22, x23, [sp, #48]\n"
    "    ldp x24, x25, [sp, #64]\n"
    "    ldp x26, x27, [sp, #80]\n"
    "    ldp x28, xzr, [sp, #96]\n"
    "    add sp, sp, #128\n"
    "    ret\n"
    ".size oah_arm_native_enter, .-oah_arm_native_enter\n"
);

void oah_arm_native_enter(u64 *x, const void *code);
#endif

oah_interp_result oah_arm_exec_native(oah_arm_state *st, const u8 *code,
                                      u64 code_size)
{
    oah_interp_result r;

    r.status = OAH_INTERP_DECODE;
    r.exit_code = 0;
    r.insns = 0;

#ifndef __aarch64__
    (void)st;
    (void)code;
    (void)code_size;
    return r;
#else
    size_t page;
    size_t sz;
    void *p;

    if (!st || !code || code_size == 0 || (code_size & 3u) != 0) {
        r.status = OAH_INTERP_FAULT;
        return r;
    }

    page = (size_t)sysconf(_SC_PAGESIZE);
    if (page < 4096) {
        page = 4096;
    }
    sz = ((size_t)code_size + page - 1) & ~(page - 1);
    p = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        r.status = OAH_INTERP_FAULT;
        return r;
    }
    memcpy(p, code, (size_t)code_size);
    if (mprotect(p, sz, PROT_READ | PROT_EXEC) != 0) {
        munmap(p, sz);
        r.status = OAH_INTERP_FAULT;
        return r;
    }
    oah_icache_invalidate(p, code_size);
    __builtin___clear_cache((char *)p, (char *)p + (size_t)code_size);

    oah_arm_native_enter(st->x, p);

    munmap(p, sz);
    r.status = OAH_INTERP_OK;
    r.insns = code_size / 4;
    return r;
#endif
}
