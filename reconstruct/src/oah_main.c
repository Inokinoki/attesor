#include "oah/elf_map.h"
#include "oah/guest.h"
#include "oah/interp.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static void usage(const char *argv0)
{
    fprintf(stderr, "usage: %s <x86_64-elf> [args...]\n", argv0);
}

static u64 setup_stack(oah_image *img, int argc, char **argv)
{
    const size_t stack_sz = 8u << 20;
    u8 *stack;
    u8 *p;
    u64 argv_gva[32];
    u64 *sp;
    int i;
    unsigned n;
    unsigned nwords;

    if (argc < 1 || argc > 31) {
        return 0;
    }

    stack = mmap(0, stack_sz, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED) {
        return 0;
    }
    if (img->nseg >= OAH_MAX_LOAD_SEGS) {
        munmap(stack, stack_sz);
        return 0;
    }
    img->segs[img->nseg].gva = (u64)(uintptr_t)stack;
    img->segs[img->nseg].size = stack_sz;
    img->segs[img->nseg].host = stack;
    img->segs[img->nseg].prot = PROT_READ | PROT_WRITE;
    img->nseg++;

    p = stack + stack_sz;
    for (i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;
        p -= len;
        memcpy(p, argv[i], len);
        argv_gva[i] = (u64)(uintptr_t)p;
    }
    p = (u8 *)((uintptr_t)p & ~15ull);

    /* argc, argv..., NULL, env NULL, 5 aux pairs, AT_NULL pair */
    nwords = (unsigned)argc + 1 + 2 + 12;
    p -= nwords * sizeof(u64);
    if (((uintptr_t)p & 15ull) != 0) {
        p -= 8;
    }

    sp = (u64 *)p;
    n = 0;
    sp[n++] = (u64)(unsigned)argc;
    for (i = 0; i < argc; i++) {
        sp[n++] = argv_gva[i];
    }
    sp[n++] = 0;
    sp[n++] = 0;
    sp[n++] = 3; /* AT_PHDR */
    sp[n++] = img->phdr;
    sp[n++] = 4; /* AT_PHENT */
    sp[n++] = img->phentsize;
    sp[n++] = 5; /* AT_PHNUM */
    sp[n++] = img->phnum;
    sp[n++] = 6; /* AT_PAGESZ */
    sp[n++] = 0x1000;
    sp[n++] = 9; /* AT_ENTRY */
    sp[n++] = img->entry;
    sp[n++] = 0; /* AT_NULL */
    sp[n++] = 0;
    (void)nwords;
    return (u64)(uintptr_t)p;
}

int main(int argc, char **argv)
{
    oah_image img;
    oah_guest guest;
    oah_interp_result run;
    u64 rsp;

    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    if (!oah_map_elf(&img, argv[1])) {
        fprintf(stderr, "oah: failed to map %s (need x86_64 ELF, first PT_LOAD at offset 0)\n",
                argv[1]);
        return 127;
    }

    rsp = setup_stack(&img, argc - 1, argv + 1);
    if (!rsp) {
        fprintf(stderr, "oah: failed to allocate guest stack\n");
        oah_unmap_elf(&img);
        return 127;
    }

    oah_guest_init(&guest, img.entry, rsp);
    run = oah_interp_run(&guest, &img, OAH_INTERP_MAX_INSNS);
    oah_unmap_elf(&img);

    if (run.status == OAH_INTERP_EXIT) {
        return run.exit_code;
    }
    fprintf(stderr, "oah: guest stopped (%d) after %llu insns rip=%llx\n",
            (int)run.status, (unsigned long long)run.insns,
            (unsigned long long)guest.rip);
    return 126;
}
