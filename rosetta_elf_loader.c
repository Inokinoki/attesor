/* ============================================================================
 * Rosetta ELF Binary Loader - Implementation
 * ============================================================================
 *
 * This module implements loading and parsing of x86_64 ELF binaries
 * for translation by the Rosetta binary translator.
 *
 * This is the Linux counterpart to rosetta_macho_loader.c (macOS)
 * ============================================================================ */

#include "rosetta_elf_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

/* ============================================================================
 * ELF Binary Cache Implementation
 * ============================================================================ */

/* Global cache instance */
static rosetta_elf_cache_t elf_cache = {
    .count = 0,
    .enabled = 1,
    .hits = 0,
    .misses = 0
};

/**
 * Initialize ELF binary cache
 */
void rosetta_elf_cache_init(void)
{
    memset(&elf_cache, 0, sizeof(elf_cache));
    elf_cache.enabled = 1;
}

/**
 * Clean up ELF binary cache
 */
void rosetta_elf_cache_cleanup(void)
{
    for (int i = 0; i < 8; i++) {
        if (elf_cache.entries[i].valid) {
            if (elf_cache.entries[i].filename) {
                free(elf_cache.entries[i].filename);
            }
            if (elf_cache.entries[i].binary) {
                rosetta_elf_unload(elf_cache.entries[i].binary);
            }
            elf_cache.entries[i].valid = 0;
        }
    }
    elf_cache.count = 0;
}

/**
 * Enable or disable ELF binary cache
 */
void rosetta_elf_cache_set_enabled(int enabled)
{
    elf_cache.enabled = enabled;
}

/**
 * Get cache statistics
 */
void rosetta_elf_cache_stats(uint64_t *hits, uint64_t *misses)
{
    if (hits) *hits = elf_cache.hits;
    if (misses) *misses = elf_cache.misses;
}

/**
 * Clear ELF binary cache
 */
void rosetta_elf_cache_clear(void)
{
    rosetta_elf_cache_cleanup();
}

/**
 * Find cache entry by filename
 */
static rosetta_elf_cache_entry_t *cache_find(const char *filename)
{
    for (int i = 0; i < 8; i++) {
        if (elf_cache.entries[i].valid &&
            elf_cache.entries[i].filename &&
            strcmp(elf_cache.entries[i].filename, filename) == 0) {
            return &elf_cache.entries[i];
        }
    }
    return NULL;
}

/**
 * Validate cache entry against current file
 */
static int cache_validate(rosetta_elf_cache_entry_t *entry, const char *filename)
{
    struct stat st;
    if (stat(filename, &st) < 0) {
        return 0;  /* File doesn't exist or can't stat */
    }

    /* Check if file size or modification time changed */
    if ((uint64_t)st.st_size != entry->file_size ||
        st.st_mtime != entry->mtime) {
        return 0;  /* File changed */
    }

    return 1;  /* Cache entry is still valid */
}

/**
 * Add entry to cache (LRU eviction)
 */
static void cache_add(const char *filename, struct stat *st, rosetta_elf_binary_t *binary)
{
    /* Find empty slot or evict LRU entry */
    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!elf_cache.entries[i].valid) {
            slot = i;
            break;
        }
    }

    /* If no empty slot, evict the first entry (simple LRU) */
    if (slot < 0) {
        slot = 0;
        if (elf_cache.entries[slot].filename) {
            free(elf_cache.entries[slot].filename);
        }
        if (elf_cache.entries[slot].binary) {
            rosetta_elf_unload(elf_cache.entries[slot].binary);
        }
    }

    /* Add new entry */
    elf_cache.entries[slot].filename = strdup(filename);
    elf_cache.entries[slot].file_size = st->st_size;
    elf_cache.entries[slot].mtime = st->st_mtime;
    elf_cache.entries[slot].binary = binary;
    elf_cache.entries[slot].valid = 1;

    if (elf_cache.count < 8) {
        elf_cache.count++;
    }
}

/* ============================================================================
 * ELF Helper Macros
 * ============================================================================ */

#define ELF64_R_SYM(info)  (((info) >> 32) & 0xffffffff)
#define ELF64_R_TYPE(info) ((info) & 0xffffffff)
#define ELF64_ST_BIND(info)  (((info) >> 4) & 0xf)
#define ELF64_ST_TYPE(info)  ((info) & 0xf)
#define ELF64_ST_VISIBILITY(other) ((other) & 0x3)

/* ============================================================================
 * File Loading Helpers
 * ============================================================================ */

/**
 * Map a file into memory
 */
static uint8_t *map_file(const char *filename, size_t *out_size)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "Failed to stat %s: %s\n", filename, strerror(errno));
        close(fd);
        return NULL;
    }

    uint8_t *data = (uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap %s: %s\n", filename, strerror(errno));
        return NULL;
    }

    *out_size = st.st_size;
    return data;
}

/**
 * Unmap a file from memory
 */
static void unmap_file(uint8_t *data, size_t size)
{
    if (data && size > 0) {
        munmap(data, size);
    }
}

/* ============================================================================
 * ELF Validation Helpers
 * ============================================================================ */

/**
 * Validate ELF header
 */
static int validate_elf_header(elf64_header_t *header)
{
    /* Check magic number */
    if (memcmp(header->e_ident, ELF_MAGIC, 4) != 0) {
        fprintf(stderr, "Invalid ELF magic number\n");
        return -1;
    }

    /* Check 64-bit */
    if (header->e_ident[4] != ELFCLASS64) {
        fprintf(stderr, "Not a 64-bit ELF file (class=%d)\n",
                header->e_ident[4]);
        return -1;
    }

    /* Check little endian */
    if (header->e_ident[5] != ELFDATA2LSB) {
        fprintf(stderr, "Not a little-endian ELF file (data=%d)\n",
                header->e_ident[5]);
        return -1;
    }

    /* Check version */
    if (header->e_ident[6] != EV_CURRENT) {
        fprintf(stderr, "Invalid ELF version (%d)\n",
                header->e_ident[6]);
        return -1;
    }

    /* Check x86_64 architecture */
    if (header->e_machine != EM_X86_64) {
        fprintf(stderr, "Not an x86_64 ELF file (e_machine=%d)\n",
                header->e_machine);
        return -1;
    }

    /* Check executable type */
    if (header->e_type != ET_EXEC && header->e_type != ET_DYN) {
        fprintf(stderr, "Not an executable ELF file (e_type=%d)\n",
                header->e_type);
        return -1;
    }

    return 0;
}

/* ============================================================================
 * ELF Parsing Implementation
 * ============================================================================ */

int rosetta_elf_load(const char *filename, rosetta_elf_binary_t **out_binary)
{
    if (!filename || !out_binary) {
        fprintf(stderr, "Invalid arguments to rosetta_elf_load\n");
        return -1;
    }

    /* Check cache first if enabled */
    if (elf_cache.enabled) {
        rosetta_elf_cache_entry_t *entry = cache_find(filename);
        if (entry && cache_validate(entry, filename)) {
            /* Cache hit - return cached binary */
            elf_cache.hits++;
            *out_binary = entry->binary;
            return 0;
        }
        elf_cache.misses++;
    }

    /* Get file stats for cache validation */
    struct stat st;
    if (stat(filename, &st) < 0) {
        /* File doesn't exist, will fail in map_file */
        memset(&st, 0, sizeof(st));
    }

    /* Allocate binary structure */
    rosetta_elf_binary_t *binary = (rosetta_elf_binary_t *)calloc(
        1, sizeof(rosetta_elf_binary_t));
    if (!binary) {
        fprintf(stderr, "Failed to allocate binary structure\n");
        return -1;
    }

    /* Map file into memory */
    binary->file_data = map_file(filename, &binary->file_size);
    if (!binary->file_data) {
        free(binary);
        return -1;
    }

    binary->filename = strdup(filename);

    /* Parse ELF header */
    if (binary->file_size < sizeof(elf64_header_t)) {
        fprintf(stderr, "File too small for ELF header\n");
        goto error;
    }

    elf64_header_t *header = (elf64_header_t *)binary->file_data;
    binary->header = *header;

    /* Validate ELF header */
    if (validate_elf_header(&binary->header) < 0) {
        goto error;
    }

    /* Check if PIE */
    binary->is_pie = (binary->header.e_type == ET_DYN);

    /* Load program headers */
    if (binary->header.e_phoff > 0 && binary->header.e_phnum > 0) {
        binary->phdrs = (elf64_phdr_t *)(binary->file_data + binary->header.e_phoff);
        binary->phdr_count = binary->header.e_phnum;
    }

    /* Load section headers */
    if (binary->header.e_shoff > 0 && binary->header.e_shnum > 0) {
        binary->shdrs = (elf64_shdr_t *)(binary->file_data + binary->header.e_shoff);
        binary->shdr_count = binary->header.e_shnum;

        /* Load section header string table */
        if (binary->header.e_shstrndx < binary->shdr_count) {
            elf64_shdr_t *shdr = &binary->shdrs[binary->header.e_shstrndx];
            binary->shstrtab = (char *)(binary->file_data + shdr->sh_offset);
        }
    }

    /* First pass: count loadable segments and sections */
    uint32_t num_segments = 0;
    uint32_t num_sections = 0;

    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];
        if (phdr->p_type == PT_LOAD) {
            num_segments++;
        }
    }

    for (uint32_t i = 0; i < binary->shdr_count; i++) {
        elf64_shdr_t *shdr = &binary->shdrs[i];
        if (shdr->sh_type != SHT_NULL && shdr->sh_name > 0) {
            num_sections++;
        }
    }

    /* Allocate arrays */
    binary->segments = (rosetta_elf_segment_t *)calloc(
        num_segments, sizeof(rosetta_elf_segment_t));
    binary->sections = (rosetta_elf_section_t *)calloc(
        num_sections, sizeof(rosetta_elf_section_t));

    if (!binary->segments || !binary->sections) {
        fprintf(stderr, "Failed to allocate segment/section arrays\n");
        goto error;
    }

    binary->num_segments = num_segments;
    binary->num_sections = num_sections;

    /* Second pass: fill in segments and sections */
    uint32_t seg_idx = 0;
    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];
        if (phdr->p_type == PT_LOAD) {
            rosetta_elf_segment_t *seg = &binary->segments[seg_idx++];

            seg->guest_vaddr = phdr->p_vaddr;
            seg->file_offset = phdr->p_offset;
            seg->file_size = phdr->p_filesz;
            seg->mem_size = phdr->p_memsz;
            seg->prot = phdr->p_flags;
            seg->flags = 0;
            seg->host_addr = 0;  /* Will be set during mapping */
            seg->data = (phdr->p_filesz > 0) ?
                (binary->file_data + phdr->p_offset) : NULL;
        }
    }

    uint32_t sect_idx = 0;
    for (uint32_t i = 0; i < binary->shdr_count; i++) {
        elf64_shdr_t *shdr = &binary->shdrs[i];
        if (shdr->sh_type != SHT_NULL && shdr->sh_name > 0) {
            rosetta_elf_section_t *sect = &binary->sections[sect_idx++];

            if (binary->shstrtab) {
                strncpy(sect->name, binary->shstrtab + shdr->sh_name, 31);
                sect->name[31] = '\0';
            } else {
                sect->name[0] = '\0';
            }

            sect->shstrtab = binary->shstrtab;
            sect->guest_addr = shdr->sh_addr;
            sect->host_addr = 0;  /* Will be set during mapping */
            sect->size = shdr->sh_size;
            sect->type = shdr->sh_type;
            sect->flags = shdr->sh_flags;
            sect->link = shdr->sh_link;
            sect->info = shdr->sh_info;
            sect->addralign = shdr->sh_addralign;
            sect->data = (shdr->sh_type != SHT_NOBITS && shdr->sh_offset > 0) ?
                (binary->file_data + shdr->sh_offset) : NULL;
        }
    }

    /* Load dynamic section */
    for (uint32_t i = 0; i < binary->shdr_count; i++) {
        elf64_shdr_t *shdr = &binary->shdrs[i];
        if (shdr->sh_type == SHT_DYNAMIC) {
            binary->dynamic = (elf64_dyn_t *)(binary->file_data + shdr->sh_offset);
            binary->dynamic_count = shdr->sh_size / sizeof(elf64_dyn_t);
        }
    }

    /* Also check program headers for dynamic section */
    if (!binary->dynamic) {
        for (uint32_t i = 0; i < binary->phdr_count; i++) {
            elf64_phdr_t *phdr = &binary->phdrs[i];
            if (phdr->p_type == PT_DYNAMIC) {
                binary->dynamic = (elf64_dyn_t *)(binary->file_data + phdr->p_offset);
                binary->dynamic_count = phdr->p_memsz / sizeof(elf64_dyn_t);
            }
        }
    }

    /* Parse dynamic section for symbol tables and string tables */
    if (binary->dynamic) {
        for (uint32_t i = 0; i < binary->dynamic_count; i++) {
            elf64_dyn_t *dyn = &binary->dynamic[i];
            if (dyn->d_tag == DT_NULL) {
                break;
            }

            switch (dyn->d_tag) {
            case DT_SYMTAB:
                binary->dynsym = (elf64_sym_t *)(binary->file_data +
                    (dyn->d_un.d_ptr - binary->header.e_phoff));
                break;
            case DT_STRTAB:
                binary->dynstr = (char *)(binary->file_data +
                    (dyn->d_un.d_ptr - binary->header.e_phoff));
                break;
            case DT_STRSZ:
                binary->dynstr_size = dyn->d_un.d_val;
                break;
            }
        }
    }

    /* Load symbol tables from sections */
    for (uint32_t i = 0; i < binary->shdr_count; i++) {
        elf64_shdr_t *shdr = &binary->shdrs[i];

        if (shdr->sh_type == SHT_DYNSYM) {
            binary->dynsym = (elf64_sym_t *)(binary->file_data + shdr->sh_offset);
            binary->dynsym_count = shdr->sh_size / sizeof(elf64_sym_t);
        } else if (shdr->sh_type == SHT_SYMTAB) {
            binary->symtab = (elf64_sym_t *)(binary->file_data + shdr->sh_offset);
            binary->symtab_count = shdr->sh_size / sizeof(elf64_sym_t);
        } else if (shdr->sh_type == SHT_STRTAB) {
            if (strcmp(binary->shstrtab + shdr->sh_name, ".dynstr") == 0) {
                binary->dynstr = (char *)(binary->file_data + shdr->sh_offset);
                binary->dynstr_size = shdr->sh_size;
            } else if (strcmp(binary->shstrtab + shdr->sh_name, ".strtab") == 0) {
                binary->strtab = (char *)(binary->file_data + shdr->sh_offset);
                binary->strtab_size = shdr->sh_size;
            }
        }
    }

    /* Check for interpreter (dynamic linker) */
    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];
        if (phdr->p_type == PT_INTERP) {
            binary->interp = (char *)(binary->file_data + phdr->p_offset);
            break;
        }
    }

    binary->is_static = (binary->interp == NULL);

    /* Initialize symbol hash table for performance */
    /* Check for DT_HASH or DT_GNU_HASH in dynamic section */
    if (binary->dynamic) {
        for (uint32_t i = 0; i < binary->dynamic_count; i++) {
            elf64_dyn_t *dyn = &binary->dynamic[i];
            if (dyn->d_tag == DT_NULL) {
                break;
            }

            if (dyn->d_tag == DT_HASH) {
                /* Initialize SYSV hash table */
                uint32_t *hash_data = (uint32_t *)(binary->file_data +
                    (dyn->d_un.d_ptr - binary->header.e_phoff));

                binary->symbol_hash.nbuckets = hash_data[0];
                binary->symbol_hash.nchains = hash_data[1];
                binary->symbol_hash.buckets = &hash_data[2];
                binary->symbol_hash.chains = &hash_data[2 + binary->symbol_hash.nbuckets];
                binary->symbol_hash.initialized = 1;
                break;
            } else if (dyn->d_tag == DT_GNU_HASH) {
                /* GNU hash table is more complex - for now skip it */
                /* TODO: Implement GNU_HASH support */
                break;
            }
        }
    }

    /* Initialize symbol cache */
    memset(binary->symbol_cache, 0, sizeof(binary->symbol_cache));

    /* Determine base address */
    uint64_t min_vaddr = UINT64_MAX;
    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];
        if (phdr->p_type == PT_LOAD && phdr->p_vaddr < min_vaddr) {
            min_vaddr = phdr->p_vaddr;
        }
    }

    /* For PIE binaries, base address is typically 0 (will be ASLR'd) */
    binary->base_address = binary->is_pie ? 0 : min_vaddr;

    /* Entry point */
    binary->entry_point = binary->header.e_entry;
    if (binary->is_pie && binary->entry_point != 0) {
        /* For PIE, entry point is relative to base */
        /* We'll adjust this during actual loading */
    }

    binary->is_loaded = 1;

    /* Add to cache if enabled */
    if (elf_cache.enabled) {
        cache_add(filename, &st, binary);
    }

    *out_binary = binary;
    return 0;

error:
    rosetta_elf_unload(binary);
    return -1;
}

void rosetta_elf_unload(rosetta_elf_binary_t *binary)
{
    if (!binary) {
        return;
    }

    /* Check if binary is in cache - don't actually free it */
    if (elf_cache.enabled) {
        for (int i = 0; i < 8; i++) {
            if (elf_cache.entries[i].valid &&
                elf_cache.entries[i].binary == binary) {
                /* Binary is cached, don't free it */
                return;
            }
        }
    }

    if (binary->filename) {
        free(binary->filename);
    }

    if (binary->file_data) {
        unmap_file(binary->file_data, binary->file_size);
    }

    if (binary->segments) {
        /* Unmap any loaded segments */
        for (uint32_t i = 0; i < binary->num_segments; i++) {
            rosetta_elf_segment_t *seg = &binary->segments[i];
            if (seg->host_addr != 0 && seg->mem_size > 0) {
                munmap((void *)seg->host_addr, seg->mem_size);
            }
        }
        free(binary->segments);
    }

    if (binary->sections) {
        free(binary->sections);
    }

    if (binary->relas) {
        free(binary->relas);
    }

    /* Clean up symbol cache */
    for (int i = 0; i < 16; i++) {
        if (binary->symbol_cache[i].name) {
            free(binary->symbol_cache[i].name);
            binary->symbol_cache[i].name = NULL;
            binary->symbol_cache[i].valid = 0;
        }
    }

    free(binary);
}

int rosetta_elf_validate_x86_64(rosetta_elf_binary_t *binary)
{
    if (!binary) {
        return 0;
    }

    if (validate_elf_header(&binary->header) < 0) {
        return 0;
    }

    return 1;
}

uint64_t rosetta_elf_get_entry_point(rosetta_elf_binary_t *binary)
{
    if (!binary || !binary->is_loaded) {
        return 0;
    }

    if (binary->is_pie) {
        /* For PIE, return relative offset */
        return binary->entry_point;
    }

    return binary->entry_point;
}

rosetta_elf_section_t *rosetta_elf_get_section(rosetta_elf_binary_t *binary,
                                               const char *name)
{
    if (!binary || !name) {
        return NULL;
    }

    for (uint32_t i = 0; i < binary->num_sections; i++) {
        rosetta_elf_section_t *sect = &binary->sections[i];
        if (strcmp(sect->name, name) == 0) {
            return sect;
        }
    }

    return NULL;
}

/**
 * Simple hash function for symbol names
 */
static uint32_t hash_symbol_name(const char *name)
{
    uint32_t h = 0;
    uint32_t g;

    while (*name) {
        h = (h << 4) + (*name++);
        if ((g = h & 0xf0000000) != 0) {
            h ^= g >> 24;
        }
        h &= ~g;
    }

    return h;
}

/**
 * Simple implementation without cache for debugging
 */
uint64_t rosetta_elf_lookup_symbol(rosetta_elf_binary_t *binary,
                                   const char *name)
{
    if (!binary || !name) {
        return 0;
    }

    /* Use hash table if available (fast path) */
    if (binary->symbol_hash.initialized &&
        binary->dynsym && binary->dynstr) {

        uint32_t hash = hash_symbol_name(name) % binary->symbol_hash.nbuckets;
        uint32_t idx = binary->symbol_hash.buckets[hash];

        while (idx != 0 && idx < binary->dynsym_count) {
            elf64_sym_t *sym = &binary->dynsym[idx];
            if (sym->st_name > 0 && sym->st_name < binary->dynstr_size) {
                const char *sym_name = binary->dynstr + sym->st_name;
                if (strcmp(sym_name, name) == 0) {
                    return sym->st_value;
                }
            }

            if (idx < binary->symbol_hash.nchains) {
                idx = binary->symbol_hash.chains[idx];
            } else {
                break;
            }
        }
    }

    /* Search dynamic symbol table */
    if (binary->dynsym && binary->dynstr) {
        for (uint32_t i = 0; i < binary->dynsym_count; i++) {
            elf64_sym_t *sym = &binary->dynsym[i];
            if (sym->st_name > 0 && sym->st_name < binary->dynstr_size) {
                const char *sym_name = binary->dynstr + sym->st_name;
                if (strcmp(sym_name, name) == 0) {
                    return sym->st_value;
                }
            }
        }
    }

    /* Search regular symbol table */
    if (binary->symtab && binary->strtab) {
        for (uint32_t i = 0; i < binary->symtab_count; i++) {
            elf64_sym_t *sym = &binary->symtab[i];
            if (sym->st_name > 0 && sym->st_name < binary->strtab_size) {
                const char *sym_name = binary->strtab + sym->st_name;
                if (strcmp(sym_name, name) == 0) {
                    return sym->st_value;
                }
            }
        }
    }

    return 0;
}

#if 0  /* Disabled for debugging - cache implementation has memory issues */
/**
 * Optimized symbol lookup with hash table and LRU cache
 */
uint64_t rosetta_elf_lookup_symbol(rosetta_elf_binary_t *binary,
                                   const char *name)
{
    if (!binary || !name) {
        return 0;
    }

    /* Check LRU cache first (fastest path) */
    for (int i = 0; i < 16; i++) {
        if (binary->symbol_cache[i].valid &&
            binary->symbol_cache[i].name &&
            strcmp(binary->symbol_cache[i].name, name) == 0) {
            /* Cache hit - update LRU */
            uint64_t value = binary->symbol_cache[i].value;

            /* Move to front (simple LRU) - but only if not already at front */
            if (i > 0) {
                /* Save the entry we want to move */
                char *temp_name = binary->symbol_cache[i].name;
                uint64_t temp_value = binary->symbol_cache[i].value;
                int temp_valid = binary->symbol_cache[i].valid;

                /* Shift entries down, but be careful not to lose pointers */
                for (int j = i; j > 0; j--) {
                    binary->symbol_cache[j].name = binary->symbol_cache[j - 1].name;
                    binary->symbol_cache[j].value = binary->symbol_cache[j - 1].value;
                    binary->symbol_cache[j].valid = binary->symbol_cache[j - 1].valid;
                }

                /* Place moved entry at front */
                binary->symbol_cache[0].name = temp_name;
                binary->symbol_cache[0].value = temp_value;
                binary->symbol_cache[0].valid = temp_valid;
            }

            return value;
        }
    }

    uint64_t result = 0;

    /* Use hash table if available (fast path) */
    if (binary->symbol_hash.initialized &&
        binary->dynsym && binary->dynstr) {

        uint32_t hash = hash_symbol_name(name) % binary->symbol_hash.nbuckets;
        uint32_t idx = binary->symbol_hash.buckets[hash];

        while (idx != 0 && idx < binary->dynsym_count) {
            elf64_sym_t *sym = &binary->dynsym[idx];
            if (sym->st_name > 0 && sym->st_name < binary->dynstr_size) {
                const char *sym_name = binary->dynstr + sym->st_name;
                if (strcmp(sym_name, name) == 0) {
                    result = sym->st_value;
                    break;
                }
            }

            if (idx < binary->symbol_hash.nchains) {
                idx = binary->symbol_hash.chains[idx];
            } else {
                break;
            }
        }
    }

    /* Fall back to linear search if hash table not available or symbol not found */
    if (result == 0) {
        /* Search dynamic symbol table */
        if (binary->dynsym && binary->dynstr) {
            for (uint32_t i = 0; i < binary->dynsym_count; i++) {
                elf64_sym_t *sym = &binary->dynsym[i];
                if (sym->st_name > 0 && sym->st_name < binary->dynstr_size) {
                    const char *sym_name = binary->dynstr + sym->st_name;
                    if (strcmp(sym_name, name) == 0) {
                        result = sym->st_value;
                        break;
                    }
                }
            }
        }

        /* Search regular symbol table */
        if (result == 0 && binary->symtab && binary->strtab) {
            for (uint32_t i = 0; i < binary->symtab_count; i++) {
                elf64_sym_t *sym = &binary->symtab[i];
                if (sym->st_name > 0 && sym->st_name < binary->strtab_size) {
                    const char *sym_name = binary->strtab + sym->st_name;
                    if (strcmp(sym_name, name) == 0) {
                        result = sym->st_value;
                        break;
                    }
                }
            }
        }
    }

    /* Update LRU cache if we found a symbol */
    if (result != 0) {
        /* Shift cache entries to make room - preserve ownership of pointers */
        for (int i = 15; i > 0; i--) {
            /* Just copy pointers, not free them */
            binary->symbol_cache[i].name = binary->symbol_cache[i - 1].name;
            binary->symbol_cache[i].value = binary->symbol_cache[i - 1].value;
            binary->symbol_cache[i].valid = binary->symbol_cache[i - 1].valid;
        }

        /* The last entry now has a duplicate pointer - free it to avoid leak */
        if (binary->symbol_cache[15].name) {
            free(binary->symbol_cache[15].name);
            binary->symbol_cache[15].name = NULL;
            binary->symbol_cache[15].valid = 0;
        }

        /* Store in cache[0] - free old name first if it exists */
        if (binary->symbol_cache[0].name) {
            free(binary->symbol_cache[0].name);
            binary->symbol_cache[0].name = NULL;
        }

        binary->symbol_cache[0].name = strdup(name);
        if (binary->symbol_cache[0].name) {  /* Only set if strdup succeeded */
            binary->symbol_cache[0].value = result;
            binary->symbol_cache[0].valid = 1;
        } else {
            /* If strdup failed, invalidate the cache entry */
            binary->symbol_cache[0].valid = 0;
        }
    }

    return result;
}
#endif  /* Disabled for debugging */

int rosetta_elf_map_segments(rosetta_elf_binary_t *binary)
{
    if (!binary || !binary->is_loaded) {
        fprintf(stderr, "Invalid binary for segment mapping\n");
        return -1;
    }

    /* Map all loadable segments */
    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];

        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        /* Calculate protection flags */
        int prot = 0;
        if (phdr->p_flags & PF_R) prot |= PROT_READ;
        if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
        if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

        /* Calculate alignment */
        size_t align = phdr->p_align;
        if (align < 0x1000) align = 0x1000;  /* Minimum page size */

        /* Allocate memory for segment */
        void *addr = mmap(NULL, phdr->p_memsz,
                          prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (addr == MAP_FAILED) {
            fprintf(stderr, "Failed to mmap segment: %s\n", strerror(errno));
            return -1;
        }

        /* Copy segment data if any */
        if (phdr->p_filesz > 0) {
            memcpy(addr, binary->file_data + phdr->p_offset, phdr->p_filesz);
        }

        /* Zero out BSS */
        if (phdr->p_memsz > phdr->p_filesz) {
            memset((uint8_t *)addr + phdr->p_filesz, 0,
                   phdr->p_memsz - phdr->p_filesz);
        }

        /* Update segment info */
        for (uint32_t j = 0; j < binary->num_segments; j++) {
            rosetta_elf_segment_t *seg = &binary->segments[j];
            if (seg->guest_vaddr == phdr->p_vaddr) {
                seg->host_addr = (uint64_t)addr;
                break;
            }
        }

        /* Update section host addresses */
        for (uint32_t j = 0; j < binary->num_sections; j++) {
            rosetta_elf_section_t *sect = &binary->sections[j];
            if (sect->guest_addr >= phdr->p_vaddr &&
                sect->guest_addr < phdr->p_vaddr + phdr->p_memsz) {
                sect->host_addr = (uint64_t)addr +
                    (sect->guest_addr - phdr->p_vaddr);
            }
        }
    }

    return 0;
}

int rosetta_elf_relocate(rosetta_elf_binary_t *binary)
{
    if (!binary || !binary->is_loaded) {
        fprintf(stderr, "Invalid binary for relocation\n");
        return -1;
    }

    /* Find relocation sections */
    for (uint32_t i = 0; i < binary->shdr_count; i++) {
        elf64_shdr_t *shdr = &binary->shdrs[i];

        if (shdr->sh_type != SHT_RELA) {
            continue;
        }

        /* Get relocation entries */
        elf64_rela_t *relas = (elf64_rela_t *)(binary->file_data + shdr->sh_offset);
        uint32_t rela_count = shdr->sh_size / sizeof(elf64_rela_t);

        /* Apply each relocation */
        for (uint32_t j = 0; j < rela_count; j++) {
            elf64_rela_t *rela = &relas[j];

            uint32_t sym_idx = ELF64_R_SYM(rela->r_info);
            uint32_t type = ELF64_R_TYPE(rela->r_info);

            /* Get target address */
            uint64_t *target = NULL;
            for (uint32_t k = 0; k < binary->num_sections; k++) {
                rosetta_elf_section_t *sect = &binary->sections[k];
                if (sect->host_addr != 0 &&
                    rela->r_offset >= sect->guest_addr &&
                    rela->r_offset < sect->guest_addr + sect->size) {
                    target = (uint64_t *)(sect->host_addr +
                        (rela->r_offset - sect->guest_addr));
                    break;
                }
            }

            if (!target) {
                fprintf(stderr, "Could not find target for relocation at 0x%lx\n",
                        rela->r_offset);
                continue;
            }

            /* Handle relocation types */
            switch (type) {
            case R_X86_64_NONE:
                /* No relocation needed */
                break;

            case R_X86_64_64:
                /* Direct 64-bit relocation */
                if (sym_idx == 0) {
                    *target = binary->base_address + rela->r_addend;
                } else {
                    /* TODO: Look up symbol value */
                    *target = rela->r_addend;
                }
                break;

            case R_X86_64_RELATIVE:
                /* Adjust by program base */
                *target = binary->base_address + rela->r_addend;
                break;

            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
                /* TODO: Look up symbol in global offset table */
                break;

            case R_X86_64_PC32:
                /* PC-relative 32-bit */
                /* TODO: Implement PC-relative relocations */
                break;

            default:
                fprintf(stderr, "Unsupported relocation type: %u\n", type);
                break;
            }
        }
    }

    return 0;
}

const char *rosetta_elf_get_interpreter(rosetta_elf_binary_t *binary)
{
    if (!binary) {
        return NULL;
    }
    return binary->interp;
}

void rosetta_elf_print_info(rosetta_elf_binary_t *binary)
{
    if (!binary) {
        return;
    }

    printf("=== ELF Binary Information ===\n");
    printf("File: %s\n", binary->filename);
    printf("Size: %zu bytes\n", binary->file_size);
    printf("Entry Point: 0x%016lx\n", binary->entry_point);
    printf("Base Address: 0x%016lx\n", binary->base_address);
    printf("PIE: %s\n", binary->is_pie ? "Yes" : "No");
    printf("Static: %s\n", binary->is_static ? "Yes" : "No");
    if (binary->interp) {
        printf("Interpreter: %s\n", binary->interp);
    }
    printf("\n");

    printf("=== Program Headers ===\n");
    for (uint32_t i = 0; i < binary->phdr_count; i++) {
        elf64_phdr_t *phdr = &binary->phdrs[i];
        const char *type_str = "UNKNOWN";
        switch (phdr->p_type) {
        case PT_NULL:    type_str = "NULL";    break;
        case PT_LOAD:    type_str = "LOAD";    break;
        case PT_DYNAMIC: type_str = "DYNAMIC"; break;
        case PT_INTERP:  type_str = "INTERP";  break;
        case PT_NOTE:    type_str = "NOTE";    break;
        case PT_PHDR:    type_str = "PHDR";    break;
        case PT_GNU_EH_FRAME: type_str = "GNU_EH_FRAME"; break;
        case PT_GNU_STACK:   type_str = "GNU_STACK";   break;
        case PT_GNU_RELRO:   type_str = "GNU_RELRO";   break;
        }

        printf("  [%2d] Type: %-12s ", i, type_str);
        if (phdr->p_type == PT_LOAD) {
            printf("Flags: %c%c%c 0x%016lx-0x%016lx (0x%06lx filesz, 0x%06lx memsz)\n",
                   (phdr->p_flags & PF_R) ? 'R' : ' ',
                   (phdr->p_flags & PF_W) ? 'W' : ' ',
                   (phdr->p_flags & PF_X) ? 'X' : ' ',
                   phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz,
                   phdr->p_filesz, phdr->p_memsz);
        } else if (phdr->p_type == PT_INTERP) {
            printf("%s\n", (char *)(binary->file_data + phdr->p_offset));
        } else {
            printf("\n");
        }
    }
    printf("\n");

    printf("=== Sections ===\n");
    for (uint32_t i = 0; i < binary->num_sections; i++) {
        rosetta_elf_section_t *sect = &binary->sections[i];
        printf("  %-20s 0x%016lx-%016lx (0x%06lx) %c%c%c\n",
               sect->name,
               sect->guest_addr, sect->guest_addr + sect->size,
               sect->size,
               (sect->flags & SHF_ALLOC)    ? 'A' : ' ',
               (sect->flags & SHF_WRITE)    ? 'W' : ' ',
               (sect->flags & SHF_EXECINSTR) ? 'X' : ' ');
    }
    printf("\n");

    if (binary->dynsym_count > 0) {
        printf("=== Dynamic Symbols ===\n");
        uint32_t printed = 0;
        for (uint32_t i = 0; i < binary->dynsym_count && printed < 20; i++) {
            elf64_sym_t *sym = &binary->dynsym[i];
            if (sym->st_name > 0 && sym->st_name < binary->dynstr_size) {
                const char *sym_name = binary->dynstr + sym->st_name;
                if (sym_name[0] != '\0' && sym->st_value != 0) {
                    printf("  0x%016lx: %s\n", sym->st_value, sym_name);
                    printed++;
                }
            }
        }
        if (binary->dynsym_count > printed) {
            printf("  ... and %u more symbols\n", binary->dynsym_count - printed);
        }
    }
}

uint64_t rosetta_elf_get_base_address(rosetta_elf_binary_t *binary)
{
    if (!binary || !binary->is_loaded) {
        return 0;
    }
    return binary->base_address;
}

void *rosetta_elf_guest_to_host(rosetta_elf_binary_t *binary,
                                uint64_t guest_addr)
{
    if (!binary) {
        return NULL;
    }

    /* Search through segments */
    for (uint32_t i = 0; i < binary->num_segments; i++) {
        rosetta_elf_segment_t *seg = &binary->segments[i];
        if (seg->host_addr != 0 &&
            guest_addr >= seg->guest_vaddr &&
            guest_addr < seg->guest_vaddr + seg->mem_size) {
            return (void *)(seg->host_addr + (guest_addr - seg->guest_vaddr));
        }
    }

    return NULL;
}

int rosetta_elf_is_static(rosetta_elf_binary_t *binary)
{
    if (!binary) {
        return 0;
    }
    return binary->is_static;
}

/* End of rosetta_elf_loader.c */
