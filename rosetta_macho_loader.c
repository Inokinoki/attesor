/* ============================================================================
 * Rosetta Mach-O Binary Loader - Implementation
 * ============================================================================
 *
 * This module implements loading and parsing of x86_64 Mach-O binaries
 * for translation by the Rosetta binary translator.
 * ============================================================================ */

#include "rosetta_macho_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* ============================================================================
 * File Loading Helper
 * ============================================================================ */

/**
 * Map a file into memory
 */
static uint8_t *map_file(const char *filename, size_t *out_size)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return NULL;
    }

    uint8_t *data = (uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) {
        perror("mmap");
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
 * Binary Loading Implementation
 * ============================================================================ */

int rosetta_macho_load(const char *filename, rosetta_binary_t **out_binary)
{
    if (!filename || !out_binary) {
        return -1;
    }

    /* Allocate binary structure */
    rosetta_binary_t *binary = (rosetta_binary_t *)calloc(1, sizeof(rosetta_binary_t));
    if (!binary) {
        return -1;
    }

    /* Map file into memory */
    binary->file_data = map_file(filename, &binary->file_size);
    if (!binary->file_data) {
        free(binary);
        return -1;
    }

    binary->filename = strdup(filename);

    /* Parse Mach-O header */
    if (binary->file_size < sizeof(macho_header_64_t)) {
        fprintf(stderr, "File too small for Mach-O header\n");
        rosetta_macho_unload(binary);
        return -1;
    }

    macho_header_64_t *header = (macho_header_64_t *)binary->file_data;
    binary->header = *header;

    /* Validate magic number */
    if (header->magic != MACHO_MAGIC_64) {
        fprintf(stderr, "Invalid Mach-O magic: 0x%08X (expected 0x%08X)\n",
                header->magic, MACHO_MAGIC_64);
        rosetta_macho_unload(binary);
        return -1;
    }

    /* Validate CPU type */
    if (header->cputype != MACHO_CPUTYPE_X86_64) {
        fprintf(stderr, "Not an x86_64 binary: cputype=0x%08X\n", header->cputype);
        rosetta_macho_unload(binary);
        return -1;
    }

    /* Check for PIE */
    binary->is_pie = (header->flags & 0x20) != 0; /* MH_PIE */

    /* Parse load commands */
    uint8_t *lc_ptr = binary->file_data + sizeof(macho_header_64_t);
    uint32_t lc_count = 0;

    /* First pass: count segments and sections */
    uint32_t num_segments = 0;
    uint32_t num_sections = 0;

    for (uint32_t i = 0; i < header->ncmds; i++) {
        uint32_t cmd = *(uint32_t *)lc_ptr;
        uint32_t cmdsize = *(uint32_t *)(lc_ptr + 4);

        if (cmd == LC_SEGMENT_64) {
            macho_segment_64_t *seg = (macho_segment_64_t *)lc_ptr;
            num_segments++;
            num_sections += seg->nsects;
        }

        lc_ptr += cmdsize;
        lc_count++;
    }

    /* Allocate arrays */
    binary->segments = (rosetta_segment_t *)calloc(num_segments, sizeof(rosetta_segment_t));
    binary->sections = (rosetta_section_t *)calloc(num_sections, sizeof(rosetta_section_t));
    binary->num_segments = num_segments;
    binary->num_sections = num_sections;

    if (!binary->segments || !binary->sections) {
        fprintf(stderr, "Failed to allocate segment/section arrays\n");
        rosetta_macho_unload(binary);
        return -1;
    }

    /* Second pass: fill in segments and sections */
    lc_ptr = binary->file_data + sizeof(macho_header_64_t);
    uint32_t seg_idx = 0;
    uint32_t sect_idx = 0;

    for (uint32_t i = 0; i < header->ncmds; i++) {
        uint32_t cmd = *(uint32_t *)lc_ptr;
        uint32_t cmdsize = *(uint32_t *)(lc_ptr + 4);

        if (cmd == LC_SEGMENT_64) {
            macho_segment_64_t *seg = (macho_segment_64_t *)lc_ptr;
            rosetta_segment_t *rseg = &binary->segments[seg_idx++];

            strncpy(rseg->name, seg->segname, 16);
            rseg->vmaddr = seg->vmaddr;
            rseg->vmsize = seg->vmsize;
            rseg->fileoff = seg->fileoff;
            rseg->filesize = seg->filesize;
            rseg->maxprot = seg->maxprot;
            rseg->initprot = seg->initprot;
            rseg->nsects = seg->nsects;

            /* Process sections */
            macho_section_64_t *sects = (macho_section_64_t *)(lc_ptr + sizeof(macho_segment_64_t));
            for (uint32_t j = 0; j < seg->nsects; j++) {
                rosetta_section_t *rsect = &binary->sections[sect_idx++];

                strncpy(rsect->name, sects[j].sectname, 16);
                strncpy(rsect->segment, sects[j].segname, 16);
                rsect->addr = sects[j].addr;
                rsect->size = sects[j].size;
                rsect->file_offset = sects[j].offset;
                rsect->flags = sects[j].flags;
                rsect->data = binary->file_data + sects[j].offset;
            }
        } else if (cmd == LC_MAIN) {
            macho_main_t *main_cmd = (macho_main_t *)lc_ptr;
            binary->entry_offset = main_cmd->entryoff;
            /* Entry point will be calculated after base address is determined */
        } else if (cmd == LC_SYMTAB) {
            macho_symtab_t *symtab = (macho_symtab_t *)lc_ptr;
            binary->symbols = (macho_nlist_64_t *)(binary->file_data + symtab->symoff);
            binary->num_symbols = symtab->nsyms;
            binary->string_table = (char *)(binary->file_data + symtab->stroff);
            binary->string_table_size = symtab->strsize;
        }

        lc_ptr += cmdsize;
    }

    /* Determine base address */
    uint64_t min_addr = UINT64_MAX;
    for (uint32_t i = 0; i < binary->num_segments; i++) {
        if (binary->segments[i].filesize > 0 &&
            binary->segments[i].vmaddr < min_addr) {
            min_addr = binary->segments[i].vmaddr;
        }
    }
    binary->base_address = binary->is_pie ? 0 : min_addr;

    /* Calculate entry point */
    if (binary->entry_offset > 0) {
        binary->entry_point = binary->base_address + binary->entry_offset;
    }

    /* Find __TEXT,__text section for entry point if LC_MAIN not present */
    if (binary->entry_point == 0) {
        void *text_section = rosetta_macho_get_section(binary, "__TEXT", "__text");
        if (text_section) {
            binary->entry_point = binary->base_address +
                                  ((uint8_t *)text_section - binary->file_data);
        }
    }

    binary->is_loaded = 1;
    *out_binary = binary;
    return 0;
}

void rosetta_macho_unload(rosetta_binary_t *binary)
{
    if (!binary) {
        return;
    }

    if (binary->filename) {
        free(binary->filename);
    }

    if (binary->file_data) {
        unmap_file(binary->file_data, binary->file_size);
    }

    if (binary->segments) {
        free(binary->segments);
    }

    if (binary->sections) {
        free(binary->sections);
    }

    free(binary);
}

int rosetta_macho_validate_x86_64(rosetta_binary_t *binary)
{
    if (!binary) {
        return 0;
    }

    if (binary->header.magic != MACHO_MAGIC_64) {
        return 0;
    }

    if (binary->header.cputype != MACHO_CPUTYPE_X86_64) {
        return 0;
    }

    return 1;
}

uint64_t rosetta_macho_get_entry_point(rosetta_binary_t *binary)
{
    if (!binary || !binary->is_loaded) {
        return 0;
    }
    return binary->entry_point;
}

void *rosetta_macho_get_section(rosetta_binary_t *binary,
                                const char *segname,
                                const char *sectname)
{
    if (!binary || !segname || !sectname) {
        return NULL;
    }

    for (uint32_t i = 0; i < binary->num_sections; i++) {
        rosetta_section_t *sect = &binary->sections[i];
        if (strcmp(sect->segment, segname) == 0 &&
            strcmp(sect->name, sectname) == 0) {
            return sect->data;
        }
    }

    return NULL;
}

uint64_t rosetta_macho_lookup_symbol(rosetta_binary_t *binary, const char *name)
{
    if (!binary || !name || !binary->symbols || !binary->string_table) {
        return 0;
    }

    for (uint32_t i = 0; i < binary->num_symbols; i++) {
        macho_nlist_64_t *sym = &binary->symbols[i];
        if (sym->n_un_strx < binary->string_table_size) {
            const char *sym_name = binary->string_table + sym->n_un_strx;
            if (strcmp(sym_name, name) == 0) {
                return sym->n_value;
            }
            /* Also check for underscore-prefixed version */
            if (name[0] != '_' && sym_name[0] == '_' &&
                strcmp(sym_name + 1, name) == 0) {
                return sym->n_value;
            }
        }
    }

    return 0;
}

void rosetta_macho_print_info(rosetta_binary_t *binary)
{
    if (!binary) {
        return;
    }

    printf("=== Mach-O Binary Information ===\n");
    printf("File: %s\n", binary->filename);
    printf("Size: %zu bytes\n", binary->file_size);
    printf("Magic: 0x%08X\n", binary->header.magic);
    printf("CPU Type: 0x%08X (x86_64)\n", binary->header.cputype);
    printf("CPU Subtype: 0x%08X\n", binary->header.cpusubtype);
    printf("File Type: 0x%08X\n", binary->header.filetype);
    printf("Load Commands: %u\n", binary->header.ncmds);
    printf("PIE: %s\n", binary->is_pie ? "Yes" : "No");
    printf("Base Address: 0x%016lX\n", binary->base_address);
    printf("Entry Point: 0x%016lX\n", binary->entry_point);
    printf("\n");

    printf("=== Segments ===\n");
    for (uint32_t i = 0; i < binary->num_segments; i++) {
        rosetta_segment_t *seg = &binary->segments[i];
        printf("  %s:\n", seg->name);
        printf("    VM Address: 0x%016lX - 0x%016lX\n",
               seg->vmaddr, seg->vmaddr + seg->vmsize);
        printf("    File Offset: 0x%08lX (%lu bytes)\n",
               seg->fileoff, seg->filesize);
        printf("    Protection: max=0x%X init=0x%X\n",
               seg->maxprot, seg->initprot);
        printf("    Sections: %u\n", seg->nsects);
    }
    printf("\n");

    printf("=== Sections ===\n");
    for (uint32_t i = 0; i < binary->num_sections; i++) {
        rosetta_section_t *sect = &binary->sections[i];
        printf("  %s.%s:\n", sect->segment, sect->name);
        printf("    Address: 0x%016lX (%lu bytes)\n", sect->addr, sect->size);
        printf("    File Offset: 0x%08X\n", sect->file_offset);
        printf("    Flags: 0x%08X\n", sect->flags);
    }
    printf("\n");

    if (binary->num_symbols > 0) {
        printf("=== Symbols (%u total) ===\n", binary->num_symbols);
        int printed = 0;
        for (uint32_t i = 0; i < binary->num_symbols && printed < 20; i++) {
            macho_nlist_64_t *sym = &binary->symbols[i];
            if (sym->n_un_strx < binary->string_table_size) {
                const char *sym_name = binary->string_table + sym->n_un_strx;
                if (sym_name[0] != '\0' && sym->n_value != 0) {
                    printf("  0x%016lX: %s\n", sym->n_value, sym_name);
                    printed++;
                }
            }
        }
        if (binary->num_symbols > 20) {
            printf("  ... and %u more symbols\n", binary->num_symbols - 20);
        }
    }
}

/* End of rosetta_macho_loader.c */
