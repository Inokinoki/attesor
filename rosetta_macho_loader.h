/* ============================================================================
 * Rosetta Mach-O Binary Loader
 * ============================================================================
 *
 * This module handles loading and parsing of x86_64 Mach-O binaries
 * for translation by the Rosetta binary translator.
 * ============================================================================ */

#ifndef ROSETTA_MACHO_LOADER_H
#define ROSETTA_MACHO_LOADER_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Mach-O Constants
 * ============================================================================ */

#define MACHO_MAGIC_64      0xFEEDFACF  /* 64-bit Mach-O magic */
#define MACHO_CPUTYPE_X86   0x00000007  /* x86 CPU type */
#define MACHO_CPUTYPE_X86_64 0x01000007 /* x86_64 CPU type */
#define MACHO_CPUSUBTYPE_INTEL  0x00000003
#define MACHO_CPUSUBTYPE_INTEL_HASM 0x00000008

/* Load command types */
#define LC_SEGMENT_64       0x19
#define LC_SYMTAB           0x02
#define LC_LOAD_DYLINKER    0x22
#define LC_MAIN             0x28
#define LC_LOAD_DYLIB       0x19
#define LC_DYLD_INFO        0x22
#define LC_DYLD_INFO_ONLY   0x80000022
#define LC_UUID             0x1B
#define LC_VERSION_MIN_MACOSX 0x24

/* Segment flags */
#define SG_HIGHVM           0x00000001

/* Section types */
#define SECTION_TYPE_MASK   0x000000FF
#define S_REGULAR           0x00
#define S_ZEROFILL          0x01
#define S_CSTRING_LITERALS  0x02
#define S_4BYTE_LITERALS    0x03
#define S_8BYTE_LITERALS    0x04
#define S_LITERAL_POINTERS  0x05
#define S_NON_LAZY_SYMBOL_POINTERS 0x06
#define S_LAZY_SYMBOL_POINTERS     0x07
#define S_SYMBOL_STUBS      0x08
#define S_MOD_INIT_FUNC_POINTERS   0x09
#define S_THREAD_LOCAL_REGULAR     0x11
#define S_THREAD_LOCAL_ZEROFILL    0x12

/* ============================================================================
 * Mach-O Data Structures
 * ============================================================================ */

/**
 * Mach-O 64-bit Header
 */
typedef struct {
    uint32_t magic;         /* Mach-O magic number */
    uint32_t cputype;       /* CPU type */
    uint32_t cpusubtype;    /* CPU subtype */
    uint32_t filetype;      /* File type */
    uint32_t ncmds;         /* Number of load commands */
    uint32_t sizeofcmds;    /* Size of load commands */
    uint32_t flags;         /* Header flags */
    uint32_t reserved;      /* Reserved (for alignment) */
} macho_header_64_t;

/**
 * Mach-O 64-bit Segment Load Command
 */
typedef struct {
    uint32_t cmd;           /* Load command type */
    uint32_t cmdsize;       /* Size of load command */
    char     segname[16];   /* Segment name */
    uint64_t vmaddr;        /* Virtual memory address */
    uint64_t vmsize;        /* Virtual memory size */
    uint64_t fileoff;       /* File offset */
    uint64_t filesize;      /* File size */
    uint32_t maxprot;       /* Maximum protection */
    uint32_t initprot;      /* Initial protection */
    uint32_t nsects;        /* Number of sections */
    uint32_t flags;         /* Segment flags */
} macho_segment_64_t;

/**
 * Mach-O 64-bit Section
 */
typedef struct {
    char     sectname[16];  /* Section name */
    char     segname[16];   /* Parent segment name */
    uint64_t addr;          /* Section address */
    uint64_t size;          /* Section size */
    uint32_t offset;        /* File offset */
    uint32_t align;         /* Section alignment */
    uint32_t reloff;        /* Relocation offset */
    uint32_t nreloc;        /* Number of relocations */
    uint32_t flags;         /* Section flags */
    uint32_t reserved1;     /* Reserved */
    uint32_t reserved2;     /* Reserved */
    uint32_t reserved3;     /* Reserved */
} macho_section_64_t;

/**
 * Mach-O Main Load Command (entry point)
 */
typedef struct {
    uint32_t cmd;           /* LC_MAIN */
    uint32_t cmdsize;       /* Size of load command */
    uint64_t entryoff;      /* Entry point offset from start of __TEXT */
    uint64_t stacksize;     /* Initial stack size */
} macho_main_t;

/**
 * Mach-O Symbol Table Load Command
 */
typedef struct {
    uint32_t cmd;           /* LC_SYMTAB */
    uint32_t cmdsize;       /* Size of load command */
    uint32_t symoff;        /* Symbol table offset */
    uint32_t nsyms;         /* Number of symbols */
    uint32_t stroff;        /* String table offset */
    uint32_t strsize;       /* String table size */
} macho_symtab_t;

/**
 * Mach-O Symbol (64-bit)
 */
typedef struct {
    uint32_t n_un_strx;     /* Symbol table index */
    uint8_t  n_type;        /* Symbol type */
    uint8_t  n_sect;        /* Section number */
    uint16_t n_desc;        /* Symbol descriptor */
    uint64_t n_value;       /* Symbol value */
} macho_nlist_64_t;

/* ============================================================================
 * Rosetta Binary Image Structure
 * ============================================================================ */

/**
 * Loaded section information
 */
typedef struct {
    char     name[32];          /* Section name */
    char     segment[16];       /* Segment name */
    uint64_t addr;              /* Load address */
    uint64_t size;              /* Section size */
    uint64_t file_offset;       /* Offset in file */
    uint32_t flags;             /* Section flags */
    void    *data;              /* Pointer to loaded data */
} rosetta_section_t;

/**
 * Loaded segment information
 */
typedef struct {
    char     name[16];          /* Segment name */
    uint64_t vmaddr;            /* Virtual address */
    uint64_t vmsize;            /* Virtual size */
    uint64_t fileoff;           /* File offset */
    uint64_t filesize;          /* File size */
    uint32_t maxprot;           /* Max protection */
    uint32_t initprot;          /* Init protection */
    uint32_t nsects;            /* Number of sections */
} rosetta_segment_t;

/**
 * Rosetta loaded binary image
 */
typedef struct {
    char          *filename;            /* Binary filename */
    uint8_t       *file_data;           /* Mapped file data */
    size_t         file_size;           /* File size */

    /* Header info */
    macho_header_64_t header;           /* Mach-O header */

    /* Entry point */
    uint64_t       entry_point;         /* Entry point address */
    uint64_t       entry_offset;        /* Entry point file offset */

    /* Segments and sections */
    rosetta_segment_t *segments;        /* Loaded segments */
    uint32_t         num_segments;      /* Number of segments */
    rosetta_section_t *sections;        /* Loaded sections */
    uint32_t         num_sections;      /* Number of sections */

    /* Symbol table */
    macho_nlist_64_t *symbols;          /* Symbol table */
    uint32_t         num_symbols;       /* Number of symbols */
    char            *string_table;      /* String table */
    uint32_t         string_table_size; /* String table size */

    /* Base address for execution */
    uint64_t       base_address;        /* Base load address */

    /* Status */
    int            is_loaded;           /* Load status */
    int            is_pie;              /* Position independent executable */
} rosetta_binary_t;

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * Load a Mach-O x86_64 binary from file
 * @param filename Path to the binary file
 * @param binary Output: pointer to loaded binary structure
 * @return 0 on success, -1 on error
 */
int rosetta_macho_load(const char *filename, rosetta_binary_t **binary);

/**
 * Unload a Mach-O binary
 * @param binary Binary to unload
 */
void rosetta_macho_unload(rosetta_binary_t *binary);

/**
 * Validate Mach-O binary is x86_64
 * @param binary Loaded binary
 * @return 1 if valid x86_64, 0 otherwise
 */
int rosetta_macho_validate_x86_64(rosetta_binary_t *binary);

/**
 * Get entry point address
 * @param binary Loaded binary
 * @return Entry point virtual address
 */
uint64_t rosetta_macho_get_entry_point(rosetta_binary_t *binary);

/**
 * Get section data by name
 * @param binary Loaded binary
 * @param segname Segment name
 * @param sectname Section name
 * @return Pointer to section data, or NULL if not found
 */
void *rosetta_macho_get_section(rosetta_binary_t *binary,
                                const char *segname,
                                const char *sectname);

/**
 * Lookup symbol by name
 * @param binary Loaded binary
 * @param name Symbol name
 * @return Symbol address, or 0 if not found
 */
uint64_t rosetta_macho_lookup_symbol(rosetta_binary_t *binary, const char *name);

/**
 * Print binary information (for debugging)
 * @param binary Loaded binary
 */
void rosetta_macho_print_info(rosetta_binary_t *binary);

#endif /* ROSETTA_MACHO_LOADER_H */
