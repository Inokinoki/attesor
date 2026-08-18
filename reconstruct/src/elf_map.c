#include "oah/elf_map.h"

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static u32 prot_from_ph(u32 flags)
{
    u32 p = 0;
    if (flags & PF_R) {
        p |= PROT_READ;
    }
    if (flags & PF_W) {
        p |= PROT_WRITE;
    }
    if (flags & PF_X) {
        p |= PROT_READ; /* keep readable for the interpreter */
    }
    return p ? p : PROT_READ;
}

static u64 page_down(u64 x)
{
    return x & ~0xfffULL;
}

static u64 page_up(u64 x)
{
    return (x + 0xfffULL) & ~0xfffULL;
}

void oah_unmap_elf(oah_image *img)
{
    unsigned i;
    if (!img) {
        return;
    }
    for (i = 0; i < img->nseg; i++) {
        if (img->segs[i].host && img->segs[i].size) {
            munmap(img->segs[i].host, (size_t)img->segs[i].size);
        }
    }
    if (img->fd >= 0) {
        close(img->fd);
    }
    memset(img, 0, sizeof(*img));
    img->fd = -1;
}

void *oah_guest_to_host(const oah_image *img, u64 gva, u64 len)
{
    unsigned i;
    if (!img || len == 0) {
        return 0;
    }
    if (gva + len < gva) {
        return 0;
    }
    for (i = 0; i < img->nseg; i++) {
        u64 s = img->segs[i].gva;
        u64 e = s + img->segs[i].size;
        if (gva >= s && gva + len <= e) {
            return img->segs[i].host + (gva - s);
        }
    }
    return 0;
}

bool oah_map_elf(oah_image *img, const char *path)
{
    Elf64_Ehdr eh;
    Elf64_Phdr *ph = 0;
    ssize_t nread;
    unsigned i;
    int first_load = 1;
    u64 eh_ph_bytes;
    int map_flags = MAP_PRIVATE | MAP_FIXED;

#ifdef MAP_FIXED_NOREPLACE
    map_flags = MAP_PRIVATE | MAP_FIXED_NOREPLACE;
#endif

    memset(img, 0, sizeof(*img));
    img->fd = -1;

    img->fd = open(path, O_RDONLY);
    if (img->fd < 0) {
        return false;
    }

    nread = pread(img->fd, &eh, sizeof(eh), 0);
    if (nread != (ssize_t)sizeof(eh)) {
        goto fail;
    }
    if (memcmp(eh.e_ident, ELFMAG, SELFMAG) != 0 ||
        eh.e_ident[EI_CLASS] != ELFCLASS64 ||
        eh.e_ident[EI_DATA] != ELFDATA2LSB ||
        eh.e_machine != EM_X86_64 ||
        (eh.e_type != ET_EXEC && eh.e_type != ET_DYN) ||
        eh.e_phentsize != sizeof(Elf64_Phdr) ||
        eh.e_phnum == 0 || eh.e_phnum > 128) {
        goto fail;
    }

    eh_ph_bytes = (u64)eh.e_phoff + (u64)eh.e_phnum * sizeof(Elf64_Phdr);
    ph = (Elf64_Phdr *)malloc((size_t)eh.e_phnum * sizeof(Elf64_Phdr));
    if (!ph) {
        goto fail;
    }
    nread = pread(img->fd, ph, (size_t)eh.e_phnum * sizeof(Elf64_Phdr),
                  (off_t)eh.e_phoff);
    if (nread != (ssize_t)((size_t)eh.e_phnum * sizeof(Elf64_Phdr))) {
        goto fail;
    }

    img->entry = eh.e_entry;
    img->phnum = eh.e_phnum;
    img->phentsize = eh.e_phentsize;

    for (i = 0; i < eh.e_phnum; i++) {
        u64 vaddr, off, filesz, memsz, map_va, map_off, map_len, align_diff;
        u32 prot;
        void *p;
        oah_load_seg *s;

        if (ph[i].p_type != PT_LOAD) {
            continue;
        }
        if (img->nseg >= OAH_MAX_LOAD_SEGS) {
            goto fail;
        }

        vaddr = ph[i].p_vaddr;
        off = ph[i].p_offset;
        filesz = ph[i].p_filesz;
        memsz = ph[i].p_memsz;
        if (memsz < filesz) {
            goto fail;
        }

        if (first_load) {
            /* ElfMapper.cpp:399 "first load segment not at file offset 0" */
            if (off != 0) {
                goto fail;
            }
            /* ElfMapper.cpp:401 spans ELF + program headers */
            if (filesz < eh_ph_bytes) {
                goto fail;
            }
            img->phdr = vaddr + eh.e_phoff;
            first_load = 0;
        }

        align_diff = vaddr & 0xfff;
        if ((off & 0xfff) != align_diff) {
            goto fail;
        }
        map_va = page_down(vaddr);
        map_off = page_down(off);
        map_len = page_up((vaddr + memsz) - map_va);
        prot = prot_from_ph(ph[i].p_flags);
        /* Interpreter needs to read instructions; force READ. */
        prot |= PROT_READ;

        p = mmap((void *)(uintptr_t)map_va, (size_t)map_len, prot, map_flags,
                 img->fd, (off_t)map_off);
        if (p == MAP_FAILED) {
            goto fail;
        }
        if ((u64)(uintptr_t)p != map_va) {
            munmap(p, (size_t)map_len);
            goto fail;
        }

        /* Zero BSS tail inside the mapping. Need PROT_WRITE temporarily. */
        if (memsz > filesz) {
            u64 bss_gva = vaddr + filesz;
            u64 bss_len = memsz - filesz;
            u8 *bss = (u8 *)(uintptr_t)bss_gva;
            if (mprotect((void *)(uintptr_t)map_va, (size_t)map_len,
                         prot | PROT_WRITE) != 0) {
                munmap(p, (size_t)map_len);
                goto fail;
            }
            memset(bss, 0, (size_t)bss_len);
            if (mprotect((void *)(uintptr_t)map_va, (size_t)map_len, prot) != 0) {
                munmap(p, (size_t)map_len);
                goto fail;
            }
        }

        s = &img->segs[img->nseg++];
        s->gva = map_va;
        s->size = map_len;
        s->host = (u8 *)(uintptr_t)map_va;
        s->prot = prot;

        if (vaddr + memsz > img->load_end) {
            img->load_end = page_up(vaddr + memsz);
        }
    }

    free(ph);
    if (img->nseg == 0) {
        goto fail;
    }
    img->brk = img->load_end;
    return true;

fail:
    free(ph);
    oah_unmap_elf(img);
    return false;
}
