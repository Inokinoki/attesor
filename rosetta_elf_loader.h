/* ============================================================================
 * Rosetta ELF Binary Loader
 * ============================================================================
 *
 * This module handles loading and parsing of x86_64 ELF binaries
 * for translation by the Rosetta binary translator.
 *
 * This is the Linux counterpart to rosetta_macho_loader.h (macOS)
 * ============================================================================ */

#ifndef ROSETTA_ELF_LOADER_H
#define ROSETTA_ELF_LOADER_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* ============================================================================
 * ELF Constants
 * ============================================================================ */

/* ELF Magic Number */
#define EI_NIDENT 16
#define EI_MAG0         0       /* File identification byte 0 index */
#define EI_MAG1         1       /* File identification byte 1 index */
#define EI_MAG2         2       /* File identification byte 2 index */
#define EI_MAG3         3       /* File identification byte 3 index */
#define EI_CLASS        4       /* File class byte index */
#define EI_DATA         5       /* Data encoding byte index */
#define EI_VERSION      6       /* File version byte index */
#define EI_OSABI        7       /* OS/ABI identification byte index */
#define EI_ABIVERSION   8       /* ABI version byte index */

#define ELF_MAGIC "\x7F" "ELF"

/* ELF Class (32-bit vs 64-bit) */
#define ELFCLASSNONE    0       /* Invalid class */
#define ELFCLASS32      1       /* 32-bit objects */
#define ELFCLASS64      2       /* 64-bit objects */

/* ELF Data Encoding (little endian vs big endian) */
#define ELFDATANONE     0       /* Invalid data encoding */
#define ELFDATA2LSB     1       /* Little-endian */
#define ELFDATA2MSB     2       /* Big-endian */

/* ELF Version */
#define EV_CURRENT      1

/* ELF Machine Architecture */
#define EM_X86_64       62      /* AMD x86-64 architecture */

/* ELF File Types */
#define ET_NONE         0       /* No file type */
#define ET_REL          1       /* Relocatable file */
#define ET_EXEC         2       /* Executable file */
#define ET_DYN          3       /* Shared object file */
#define ET_CORE         4       /* Core file */

/* ELF Program Header Types */
#define PT_NULL         0       /* Unused entry */
#define PT_LOAD         1       /* Loadable segment */
#define PT_DYNAMIC      2       /* Dynamic linking information */
#define PT_INTERP       3       /* Interpreter path */
#define PT_NOTE         4       /* Auxiliary information */
#define PT_SHLIB        5       /* Reserved */
#define PT_PHDR         6       /* Program header table */
#define PT_GNU_EH_FRAME 0x6474e550    /* GCC .eh_frame_hdr segment */
#define PT_GNU_STACK    0x6474e551    /* Indicates stack executability */
#define PT_GNU_RELRO    0x6474e552    /* Read-only after relocations */

/* ELF Segment Flags */
#define PF_X            0x1     /* Execute */
#define PF_W            0x2     /* Write */
#define PF_R            0x4     /* Read */

/* ELF Section Header Types */
#define SHT_NULL        0       /* Section header table entry unused */
#define SHT_PROGBITS    1       /* Program-defined contents */
#define SHT_SYMTAB      2       /* Symbol table */
#define SHT_STRTAB      3       /* String table */
#define SHT_RELA        4       /* Relocation entries with explicit addends */
#define SHT_HASH        5       /* Symbol hash table */
#define SHT_DYNAMIC     6       /* Dynamic linking information */
#define SHT_NOTE        7       /* Note section */
#define SHT_NOBITS      8       /* No space (bss) */
#define SHT_REL         9       /* Relocation entries without explicit addends */
#define SHT_DYNSYM      11      /* Dynamic linker symbol table */

/* ELF Section Header Flags */
#define SHF_WRITE       0x1     /* Writable */
#define SHF_ALLOC       0x2     /* Occupies memory during execution */
#define SHF_EXECINSTR   0x4     /* Executable */
#define SHF_MERGE       0x10    /* Might be merged */
#define SHF_STRINGS     0x20    /* Contains null-terminated strings */
#define SHF_INFO_LINK   0x40    /* sh_info holds section header table index */
#define SHF_LINK_ORDER  0x80    /* Preserve order after combining */
#define SHF_OS_NONCONFORMING 0x100    /* Target-specific semantics */
#define SHF_GROUP       0x200   /* Member of section group */
#define SHF_TLS         0x400   /* Thread-local storage */
#define SHF_MASKOS      0x0ff00000  /* OS-specific */
#define SHF_MASKPROC    0xf0000000  /* Processor-specific */
#define SHF_ORDERED     0x40000000  /* Special ordering requirement */
#define SHF_EXCLUDE     0x80000000  /* Exclude from linking */

/* Dynamic Array Tags */
#define DT_NULL         0       /* End of dynamic array */
#define DT_NEEDED       1       /* Name of needed library */
#define DT_PLTRELSZ     2       /* Size in bytes of PLT relocs */
#define DT_PLTGOT       3       /* Processor defined value */
#define DT_HASH         4       /* Address of symbol hash table */
#define DT_STRTAB       5       /* Address of string table */
#define DT_SYMTAB       6       /* Address of symbol table */
#define DT_RELA         7       /* Address of Rela relocs */
#define DT_RELASZ       8       /* Total size of Rela relocs */
#define DT_RELAENT      9       /* Size of one Rela entry */
#define DT_STRSZ        10      /* Size of string table */
#define DT_SYMENT       11      /* Size of one symbol table entry */
#define DT_INIT         12      /* Address of init function */
#define DT_FINI         13      /* Address of termination function */
#define DT_SONAME       14      /* Name of shared object */
#define DT_RPATH        15      /* Library search path (deprecated) */
#define DT_SYMBOLIC     16      /* Start symbol search here */
#define DT_REL          17      /* Address of Rel relocs */
#define DT_RELSZ        18      /* Total size of Rel relocs */
#define DT_RELENT       19      /* Size of one Rel entry */
#define DT_PLTREL       20      /* Type of reloc in PLT */
#define DT_DEBUG        21      /* For debugging; unspecified */
#define DT_TEXTREL      22      /* Reloc might modify .text */
#define DT_JMPREL       23      /* Address of PLT relocs */
#define DT_BIND_NOW     24      /* Process relocations now */
#define DT_INIT_ARRAY   25      /* Array of init function addresses */
#define DT_FINI_ARRAY   26      /* Array of termination function addresses */
#define DT_INIT_ARRAYSZ 27      /* Size of DT_INIT_ARRAY */
#define DT_FINI_ARRAYSZ 28      /* Size of DT_FINI_ARRAY */
#define DT_RUNPATH      29      /* Library search path */
#define DT_FLAGS        30      /* Flags for the object being loaded */
#define DT_ENCODING     32      /* Start of encoded range */
#define DT_PREINIT_ARRAY 32     /* Array of preinit function addresses */
#define DT_PREINIT_ARRAYSZ 33   /* Size of DT_PREINIT_ARRAY */
#define DT_GNU_HASH     0x6ffffef5     /* GNU-style hash table */
#define DT_VERSYM       0x6ffffff0      /* Version symbol table */
#define DT_VERNEED      0x6ffffffe      /* Needed versions */
#define DT_VERNEEDNUM   0x6fffffff      /* Number of needed versions */

/* Relocation Types for x86_64 */
#define R_X86_64_NONE   0       /* No reloc */
#define R_X86_64_64     1       /* Direct 64 bit  */
#define R_X86_64_PC32   2       /* PC relative 32 bit signed */
#define R_X86_64_GOT32  3       /* 32 bit GOT entry */
#define R_X86_64_PLT32  4       /* 32 bit PLT address */
#define R_X86_64_COPY   5       /* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT 6     /* Create GOT entry */
#define R_X86_64_JUMP_SLOT 7    /* Create PLT entry */
#define R_X86_64_RELATIVE 8     /* Adjust by program base */
#define R_X86_64_GOTPCREL 9     /* 32 bit signed PC relative offset to GOT */
#define R_X86_64_32     10      /* Direct 32 bit zero extended */
#define R_X86_64_32S    11      /* Direct 32 bit sign extended */
#define R_X86_64_16     12      /* Direct 16 bit zero extended */
#define R_X86_64_PC16   13      /* 16 bit sign extended pc relative */
#define R_X86_64_8      14      /* Direct 8 bit sign extended  */
#define R_X86_64_PC8    15      /* 8 bit sign extended pc relative */
#define R_X86_64_IRELATIVE 37   /* Adjust indirectly by program base */

/* Symbol Binding */
#define STB_LOCAL       0       /* Local symbol */
#define STB_GLOBAL      1       /* Global symbol */
#define STB_WEAK        2       /* Weak symbol */

/* Symbol Types */
#define STT_NOTYPE      0       /* Unspecified type */
#define STT_OBJECT      1       /* Data object */
#define STT_FUNC        2       /* Function */
#define STT_SECTION     3       /* Section */
#define STT_FILE        4       /* Source file */
#define STT_COMMON      5       /* Common data object */
#define STT_TLS         6       /* Thread-local data object */

/* Special Section Indices */
#define SHN_UNDEF       0       /* Undefined section */
#define SHN_ABS         0xfff1  /* Absolute symbol */
#define SHN_COMMON      0xfff2  /* Common symbol */

/* ============================================================================
 * ELF Data Structures
 * ============================================================================ */

/**
 * ELF 64-bit Header
 */
typedef struct {
    uint8_t  e_ident[EI_NIDENT]; /* Magic number and other info */
    uint16_t e_type;             /* Object file type */
    uint16_t e_machine;          /* Architecture */
    uint32_t e_version;          /* Object file version */
    uint64_t e_entry;            /* Entry point virtual address */
    uint64_t e_phoff;            /* Program header table file offset */
    uint64_t e_shoff;            /* Section header table file offset */
    uint32_t e_flags;            /* Processor-specific flags */
    uint16_t e_ehsize;           /* ELF header size in bytes */
    uint16_t e_phentsize;        /* Program header table entry size */
    uint16_t e_phnum;            /* Program header table entry count */
    uint16_t e_shentsize;        /* Section header table entry size */
    uint16_t e_shnum;            /* Section header table entry count */
    uint16_t e_shstrndx;         /* Section header string table index */
} elf64_header_t;

/**
 * ELF 64-bit Program Header
 */
typedef struct {
    uint32_t p_type;             /* Segment type */
    uint32_t p_flags;            /* Segment flags */
    uint64_t p_offset;           /* Segment file offset */
    uint64_t p_vaddr;            /* Segment virtual address */
    uint64_t p_paddr;            /* Segment physical address */
    uint64_t p_filesz;           /* Segment size in file */
    uint64_t p_memsz;            /* Segment size in memory */
    uint64_t p_align;            /* Segment alignment */
} elf64_phdr_t;

/**
 * ELF 64-bit Section Header
 */
typedef struct {
    uint32_t sh_name;            /* Section name (string table index) */
    uint32_t sh_type;            /* Section type */
    uint64_t sh_flags;           /* Section flags */
    uint64_t sh_addr;            /* Section virtual address at execution */
    uint64_t sh_offset;          /* Section file offset */
    uint64_t sh_size;            /* Section size in bytes */
    uint32_t sh_link;            /* Link to another section */
    uint32_t sh_info;            /* Additional section information */
    uint64_t sh_addralign;       /* Section alignment */
    uint64_t sh_entsize;         /* Entry size if section holds table */
} elf64_shdr_t;

/**
 * ELF 64-bit Relocation Entry (with addend)
 */
typedef struct {
    uint64_t r_offset;           /* Address where to apply relocation */
    uint32_t r_info;             /* Relocation type and symbol index */
    int32_t  r_addend;           /* Addend */
} elf64_rela_t;

/**
 * ELF 64-bit Relocation Entry (without addend)
 */
typedef struct {
    uint64_t r_offset;           /* Address where to apply relocation */
    uint32_t r_info;             /* Relocation type and symbol index */
} elf64_rel_t;

/**
 * ELF 64-bit Symbol Table Entry
 */
typedef struct {
    uint32_t st_name;            /* Symbol name (string table index) */
    uint8_t  st_info;            /* Symbol type and binding */
    uint8_t  st_other;           /* Symbol visibility */
    uint16_t st_shndx;           /* Section index */
    uint64_t st_value;           /* Symbol value */
    uint64_t st_size;            /* Symbol size */
} elf64_sym_t;

/**
 * ELF 64-bit Dynamic Entry
 */
typedef struct {
    int64_t  d_tag;              /* Dynamic entry type */
    union {
        uint64_t d_val;          /* Integer value */
        uint64_t d_ptr;          /* Address value */
    } d_un;
} elf64_dyn_t;

/* ============================================================================
 * Rosetta ELF Binary Image Structure
 * ============================================================================ */

/**
 * Loaded ELF segment information
 */
typedef struct {
    uint64_t guest_vaddr;        /* Virtual address in guest memory */
    uint64_t host_addr;          /* Host memory address */
    uint64_t file_offset;        /* Offset in ELF file */
    uint64_t file_size;          /* Size in file */
    uint64_t mem_size;           /* Size in memory (may be larger for .bss) */
    uint32_t prot;               /* Protection flags (PF_R | PF_W | PF_X) */
    uint32_t flags;              /* Additional flags */
    uint8_t *data;               /* Pointer to loaded data */
} rosetta_elf_segment_t;

/**
 * Loaded ELF section information
 */
typedef struct {
    char     name[32];           /* Section name */
    char      *shstrtab;          /* String table for section names */
    uint64_t guest_addr;         /* Virtual address in guest memory */
    uint64_t host_addr;          /* Host memory address */
    uint64_t size;               /* Section size */
    uint32_t type;               /* Section type */
    uint32_t flags;              /* Section flags */
    uint32_t link;               /* Section link */
    uint32_t info;               /* Section info */
    uint64_t addralign;          /* Alignment */
    uint8_t *data;               /* Pointer to section data */
} rosetta_elf_section_t;

/**
 * Relocation information
 */
typedef struct {
    uint64_t offset;             /* Relocation offset */
    uint32_t type;               /* Relocation type */
    uint32_t sym_idx;            /* Symbol index */
    int64_t  addend;             /* Addend */
    int      applied;            /* Whether relocation has been applied */
} rosetta_rela_t;

/**
 * Rosetta loaded ELF binary image
 */
typedef struct {
    /* File information */
    char     *filename;          /* Binary filename */
    uint8_t  *file_data;         /* Mapped file data */
    size_t    file_size;         /* File size */

    /* ELF header */
    elf64_header_t header;       /* ELF header */

    /* Entry point */
    uint64_t entry_point;        /* Entry point virtual address */

    /* Program headers (segments) */
    elf64_phdr_t     *phdrs;     /* Program header array */
    uint32_t          phdr_count;/* Number of program headers */

    /* Section headers */
    elf64_shdr_t     *shdrs;     /* Section header array */
    uint32_t          shdr_count;/* Number of section headers */
    char             *shstrtab;  /* Section header string table */

    /* Loaded segments */
    rosetta_elf_segment_t *segments;  /* Loaded loadable segments */
    uint32_t             num_segments; /* Number of loaded segments */

    /* Loaded sections */
    rosetta_elf_section_t *sections;  /* Loaded sections */
    uint32_t             num_sections; /* Number of loaded sections */

    /* Symbol tables */
    elf64_sym_t      *dynsym;    /* Dynamic symbol table */
    uint32_t          dynsym_count; /* Number of dynamic symbols */
    char             *dynstr;    /* Dynamic string table */
    uint32_t          dynstr_size; /* Size of dynamic string table */

    elf64_sym_t      *symtab;    /* Symbol table */
    uint32_t          symtab_count; /* Number of symbols */
    char             *strtab;    /* String table */
    uint32_t          strtab_size; /* Size of string table */

    /* Relocations */
    rosetta_rela_t   *relas;     /* Relocation entries */
    uint32_t         rela_count; /* Number of relocation entries */

    /* Dynamic linking */
    elf64_dyn_t      *dynamic;   /* Dynamic section */
    uint32_t         dynamic_count; /* Number of dynamic entries */
    char            *interp;     /* Interpreter path (e.g., ld-linux.so) */

    /* Base address for execution */
    uint64_t         base_address; /* Base load address */

    /* Status */
    int              is_loaded;  /* Load status */
    int              is_static;  /* Static binary (no interpreter) */
    int              is_pie;     /* Position independent executable */

    /* Performance optimization: Symbol hash table */
    struct {
        uint32_t    *buckets;        /* Hash buckets */
        uint32_t    *chains;         /* Hash chains */
        uint32_t     nbuckets;       /* Number of buckets */
        uint32_t     nchains;        /* Number of chains */
        int          initialized;    /* Whether hash table is ready */
    } symbol_hash;

    /* Performance optimization: Symbol cache */
    struct {
        char       *name;            /* Cached symbol name */
        uint64_t    value;           /* Cached symbol value */
        int         valid;           /* Whether cache entry is valid */
    } symbol_cache[16];              /* 16-entry LRU cache */

} rosetta_elf_binary_t;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * Load an ELF x86_64 binary from file
 * @param filename Path to the binary file
 * @param binary Output: pointer to loaded binary structure
 * @return 0 on success, -1 on error
 */
int rosetta_elf_load(const char *filename, rosetta_elf_binary_t **binary);

/**
 * Unload an ELF binary
 * @param binary Binary to unload
 */
void rosetta_elf_unload(rosetta_elf_binary_t *binary);

/**
 * Validate ELF binary is x86_64
 * @param binary Loaded binary
 * @return 1 if valid x86_64, 0 otherwise
 */
int rosetta_elf_validate_x86_64(rosetta_elf_binary_t *binary);

/**
 * Get entry point address
 * @param binary Loaded binary
 * @return Entry point virtual address
 */
uint64_t rosetta_elf_get_entry_point(rosetta_elf_binary_t *binary);

/**
 * Get section data by name
 * @param binary Loaded binary
 * @param name Section name
 * @return Pointer to section information, or NULL if not found
 */
rosetta_elf_section_t *rosetta_elf_get_section(rosetta_elf_binary_t *binary,
                                               const char *name);

/**
 * Lookup symbol by name
 * @param binary Loaded binary
 * @param name Symbol name
 * @return Symbol value, or 0 if not found
 */
uint64_t rosetta_elf_lookup_symbol(rosetta_elf_binary_t *binary,
                                   const char *name);

/**
 * Map all loadable segments into memory
 * @param binary Loaded binary
 * @return 0 on success, -1 on error
 */
int rosetta_elf_map_segments(rosetta_elf_binary_t *binary);

/**
 * Apply all relocations
 * @param binary Loaded binary
 * @return 0 on success, -1 on error
 */
int rosetta_elf_relocate(rosetta_elf_binary_t *binary);

/**
 * Get interpreter path (dynamic linker)
 * @param binary Loaded binary
 * @return Interpreter path string, or NULL if static
 */
const char *rosetta_elf_get_interpreter(rosetta_elf_binary_t *binary);

/**
 * Print binary information (for debugging)
 * @param binary Loaded binary
 */
void rosetta_elf_print_info(rosetta_elf_binary_t *binary);

/**
 * Get base address for execution
 * @param binary Loaded binary
 * @return Base address
 */
uint64_t rosetta_elf_get_base_address(rosetta_elf_binary_t *binary);

/**
 * Translate guest virtual address to host address
 * @param binary Loaded binary
 * @param guest_addr Guest virtual address
 * @return Host address, or NULL if not mapped
 */
void *rosetta_elf_guest_to_host(rosetta_elf_binary_t *binary,
                                uint64_t guest_addr);

/**
 * Check if binary is static
 * @param binary Loaded binary
 * @return 1 if static, 0 if dynamic
 */
int rosetta_elf_is_static(rosetta_elf_binary_t *binary);

/* ============================================================================
 * ELF Binary Cache
 * ============================================================================ */

/**
 * ELF binary cache entry for performance optimization
 */
typedef struct {
    char *filename;             /* Binary filename */
    uint64_t file_size;         /* File size for validation */
    time_t mtime;               /* File modification time */
    rosetta_elf_binary_t *binary; /* Cached binary structure */
    int valid;                  /* Whether cache entry is valid */
} rosetta_elf_cache_entry_t;

/**
 * ELF binary cache structure
 */
typedef struct {
    rosetta_elf_cache_entry_t entries[8];  /* 8-entry cache */
    int count;                  /* Number of valid entries */
    int enabled;                /* Whether cache is enabled */
    uint64_t hits;              /* Cache hits */
    uint64_t misses;            /* Cache misses */
} rosetta_elf_cache_t;

/**
 * Initialize ELF binary cache
 */
void rosetta_elf_cache_init(void);

/**
 * Clean up ELF binary cache
 */
void rosetta_elf_cache_cleanup(void);

/**
 * Enable or disable ELF binary cache
 * @param enabled 1 to enable, 0 to disable
 */
void rosetta_elf_cache_set_enabled(int enabled);

/**
 * Get cache statistics
 * @param hits Output: cache hits
 * @param misses Output: cache misses
 */
void rosetta_elf_cache_stats(uint64_t *hits, uint64_t *misses);

/**
 * Clear ELF binary cache
 */
void rosetta_elf_cache_clear(void);

#endif /* ROSETTA_ELF_LOADER_H */
