/*
 * Rosetta Translator - Refactored Header
 * Apple's ARM64-to-x86_64 translation layer for Linux
 *
 * This header defines the core structures and types used by Rosetta.
 */

#ifndef ROSETTA_REFACTORED_H
#define ROSETTA_REFACTORED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>

/* Noreturn attribute */
#define noreturn _Noreturn

/* ============================================================================
 * Basic Type Definitions
 * ============================================================================ */

typedef uint8_t   byte;
typedef uint16_t  word;
typedef uint32_t  dword;
typedef uint64_t  qword;
typedef int64_t   longlong;
typedef uint64_t  ulonglong;

/* ============================================================================
 * ARM64 Register Context Structures
 * ============================================================================ */

/* Vector register (128-bit NEON/FP register) */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} Vector128;

/* ARM64 Floating Point Context */
typedef struct {
    uint32_t fpcr;  /* Floating Point Control Register */
    uint32_t fpsr;  /* Floating Point Status Register */
} FPContext;

/* ARM64 General Purpose Registers */
typedef struct {
    uint64_t x[30];  /* X0-X29 */
    uint64_t lr;     /* X30 - Link Register */
    uint64_t sp;     /* Stack Pointer */
    uint64_t pc;     /* Program Counter */
    uint64_t nzcv;   /* Condition flags (N, Z, C, V) */
} GPRContext;

/* ARM64 Vector Registers (V0-V31) */
typedef struct {
    Vector128 v[32];
} VectorContext;

/* Full ARM64 CPU Context */
typedef struct {
    GPRContext    gpr;
    VectorContext vec;
    FPContext     fp;
} CPUContext;

/* ============================================================================
 * Rosetta State Structures
 * ============================================================================ */

/* Translation Block Header */
typedef struct {
    uint64_t guest_pc;      /* Guest (ARM64) PC */
    uint64_t host_pc;       /* Host (x86_64) PC */
    uint32_t size;          /* Block size */
    uint32_t flags;         /* Block flags */
} TranslationBlock;

/* Translation Cache Entry */
typedef struct {
    uint64_t guest_addr;
    uint64_t host_addr;
    uint32_t hash;
    uint32_t refcount;
} TranslationCacheEntry;

/* Guest-Host Mapping */
typedef struct {
    uint64_t guest_base;
    uint64_t host_base;
    uint64_t size;
    uint32_t prot;
    uint32_t flags;
} MemoryMapping;

/* Thread Local State */
typedef struct {
    CPUContext    cpu;
    void         *tls_base;
    uint32_t      thread_id;
    uint32_t      flags;
    uint64_t      syscall_nr;
    int64_t       syscall_result;
} ThreadState;

/* ============================================================================
 * Function Pointer Types
 * ============================================================================ */

typedef void (*translate_func_t)(ThreadState *);
typedef int (*syscall_handler_t)(ThreadState *, int nr);
typedef void *(*memory_map_func_t)(uint64_t guest, uint64_t size);
typedef int (*memory_unmap_func_t)(uint64_t guest, uint64_t size);

/* ============================================================================
 * NEON/SIMD Helper Macros
 * ============================================================================ */

/* Extract byte from vector */
#define VGET_BYTE(v, i) (((uint8_t*)(v))[(i)])

/* Set byte in vector */
#define VSET_BYTE(v, i, b) (((uint8_t*)(v))[(i)] = (b))

/* NEON UMINV - Unsigned Minimum Across Vector */
static inline uint8_t neon_uminv(const uint8_t vec[16]) {
    uint8_t min = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] < min) min = vec[i];
    }
    return min;
}

/* NEON UMAXV - Unsigned Maximum Across Vector */
static inline uint8_t neon_umaxv(const uint8_t vec[16]) {
    uint8_t max = vec[0];
    for (int i = 1; i < 16; i++) {
        if (vec[i] > max) max = vec[i];
    }
    return max;
}

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/* Hash a 64-bit address */
uint32_t hash_address(uint64_t addr);

/* Hash a null-terminated string */
uint32_t hash_string(const char *s);

/* Compute hash over arbitrary data */
uint32_t hash_compute(const void *data, size_t len);

/* ============================================================================
 * Translation Cache
 * ============================================================================ */

/* Look up a translation by guest PC */
void *translation_lookup(uint64_t guest_pc);

/* Insert a translation into the cache */
int translation_insert(uint64_t guest, uint64_t host, size_t sz);

/* ============================================================================
 * Vector Operations - Arithmetic
 * ============================================================================ */

Vector128 v128_add(Vector128 a, Vector128 b);
Vector128 v128_sub(Vector128 a, Vector128 b);
Vector128 v128_mul(Vector128 a, Vector128 b);
Vector128 v128_and(Vector128 a, Vector128 b);
Vector128 v128_orr(Vector128 a, Vector128 b);
Vector128 v128_xor(Vector128 a, Vector128 b);
Vector128 v128_not(Vector128 a);
Vector128 v128_neg(Vector128 a);
Vector128 v128_shl(Vector128 a, int shift);
Vector128 v128_shr(Vector128 a, int shift);
Vector128 v128_sar(Vector128 a, int shift);

/* ============================================================================
 * Vector Operations - Compare
 * ============================================================================ */

Vector128 v128_eq(Vector128 a, Vector128 b);
Vector128 v128_neq(Vector128 a, Vector128 b);
Vector128 v128_lt(Vector128 a, Vector128 b);
Vector128 v128_gt(Vector128 a, Vector128 b);
Vector128 v128_lte(Vector128 a, Vector128 b);
Vector128 v128_gte(Vector128 a, Vector128 b);

/* ============================================================================
 * Vector Operations - Reduce
 * ============================================================================ */

Vector128 v128_umin(Vector128 a, Vector128 b);
Vector128 v128_umax(Vector128 a, Vector128 b);
Vector128 v128_smin(Vector128 a, Vector128 b);
Vector128 v128_smax(Vector128 a, Vector128 b);

/* Vector reduce across elements */
uint8_t v128_uminv(Vector128 a);
uint8_t v128_umaxv(Vector128 a);
int8_t v128_sminv(Vector128 a);
int8_t v128_smaxv(Vector128 a);

/* ============================================================================
 * Vector Operations - Conversion Helpers
 * ============================================================================ */

/* Convert ulong to vector */
Vector128 v128_from_ulong(uint64_t val);

/* Convert vector to ulong */
uint64_t ulong_from_v128(Vector128 v);

/* Zero vector */
Vector128 v128_zero(void);

/* Load vector from memory */
Vector128 v128_load(const void *addr);

/* Store vector to memory */
void v128_store(Vector128 v, void *addr);

/* Vector add across elements */
uint64_t v128_addv(Vector128 a);

/* Additional vector operations - Session 12 */
Vector128 v128_padd(Vector128 a);
Vector128 v128_abs(Vector128 a);
Vector128 v128_sat_add(Vector128 a, Vector128 b);
Vector128 v128_sat_sub(Vector128 a, Vector128 b);
Vector128 v128_rev(Vector128 a);
Vector128 v128_cnt(Vector128 a);

/* ============================================================================
 * CRC32 Checksum Functions
 * ============================================================================ */

/* CRC32 checksum of a byte */
uint32_t crc32_byte(uint32_t crc, uint8_t byte);

/* CRC32 checksum of a word */
uint32_t crc32_word(uint32_t crc, uint32_t word);

/* ============================================================================
 * FP Translation - Additional Functions
 * ============================================================================ */

/* FP conditional select */
int translate_fcsel(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Syscall Handlers
 * ============================================================================ */

/* Basic I/O */
int syscall_read(ThreadState *state);
int syscall_write(ThreadState *state);
int syscall_open(ThreadState *state);
int syscall_close(ThreadState *state);
int syscall_lseek(ThreadState *state);
int syscall_access(ThreadState *state);
int syscall_pipe(ThreadState *state);

/* Memory */
int syscall_mmap(ThreadState *state);
int syscall_munmap(ThreadState *state);
int syscall_mprotect(ThreadState *state);
int syscall_brk(ThreadState *state);

/* File status */
int syscall_stat(ThreadState *state);
int syscall_fstat(ThreadState *state);
int syscall_lstat(ThreadState *state);

/* Process */
int syscall_getpid(ThreadState *state);
int syscall_gettid(ThreadState *state);
int syscall_uname(ThreadState *state);
int syscall_fcntl(ThreadState *state);
int syscall_set_tid_address(ThreadState *state);
noreturn void syscall_exit(ThreadState *state);
noreturn void syscall_exit_group(ThreadState *state);

/* Time */
int syscall_gettimeofday(ThreadState *state);
int syscall_clock_gettime(ThreadState *state);
int syscall_nanosleep(ThreadState *state);

/* Signal */
int syscall_rt_sigaction(ThreadState *state);
int syscall_rt_sigprocmask(ThreadState *state);
int syscall_sched_yield(ThreadState *state);

/* IPC/Sync */
int syscall_futex(ThreadState *state);
int syscall_arch_prctl(ThreadState *state);

/* ============================================================================
 * Memory Management
 * ============================================================================ */

void *memory_map_guest(uint64_t guest, uint64_t size);
int memory_unmap_guest(uint64_t guest, uint64_t size);
int memory_protect_guest(uint64_t guest, uint64_t size, int32_t prot);
void *memory_translate_addr(uint64_t guest);
int memory_init(void);
void memory_cleanup(void);
void *memory_map_guest_with_prot(uint64_t guest, uint64_t size, int32_t prot);

/* ============================================================================
 * Context Save/Restore
 * ============================================================================ */

void context_noop_1(void);
void context_noop_2(void);

/* ============================================================================
 * Helper Utilities
 * ============================================================================ */

void *helper_block_translate(uint64_t guest_pc);
int helper_block_insert(uint64_t guest, uint64_t host, size_t size);
void *helper_block_lookup(uint64_t guest_pc);
int helper_block_remove(uint64_t guest_pc);
void helper_block_invalidate(void);
void helper_context_switch(CPUContext *old_ctx, CPUContext *new_ctx);
void helper_syscall_enter(ThreadState *state, int nr);
void helper_syscall_exit(ThreadState *state, int64_t result);
void helper_interrupt(ThreadState *state, int vector);
uint64_t switch_case_handler_13(uint64_t value);
uint64_t switch_case_handler_2e(uint64_t value);

/* ============================================================================
 * FP Estimates
 * ============================================================================ */

float fp_recip_estimate(float value);
float fp_rsqrt_estimate(float value);

/* ============================================================================
 * Additional Syscall Handlers
 * ============================================================================ */

int syscall_ioctl(ThreadState *state);
int syscall_dup2(ThreadState *state);
int syscall_dup3(ThreadState *state);
int syscall_poll(ThreadState *state);
int syscall_select(ThreadState *state);
int syscall_readv(ThreadState *state);
int syscall_writev(ThreadState *state);
int syscall_getcwd(ThreadState *state);
int syscall_chdir(ThreadState *state);
int syscall_rename(ThreadState *state);
int syscall_mkdir(ThreadState *state);
int syscall_rmdir(ThreadState *state);
int syscall_unlink(ThreadState *state);
int syscall_symlink(ThreadState *state);
int syscall_readlink(ThreadState *state);
int syscall_chmod(ThreadState *state);
int syscall_lchown(ThreadState *state);
int syscall_getdents(ThreadState *state);

/* ============================================================================
 * Binary Translation Core
 * ============================================================================ */

void *translate_block(uint64_t guest_pc);
void *translate_block_fast(uint64_t guest_pc);

/* ============================================================================
 * ALU Translation Functions
 * ============================================================================ */

int translate_add(ThreadState *state, const uint8_t *insn);
int translate_sub(ThreadState *state, const uint8_t *insn);
int translate_and(ThreadState *state, const uint8_t *insn);
int translate_orr(ThreadState *state, const uint8_t *insn);
int translate_eor(ThreadState *state, const uint8_t *insn);
int translate_mul(ThreadState *state, const uint8_t *insn);
int translate_div(ThreadState *state, const uint8_t *insn);
int translate_mvn(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Branch Translation Functions
 * ============================================================================ */

int translate_b(ThreadState *state, const uint8_t *insn);
int translate_bl(ThreadState *state, const uint8_t *insn);
int translate_br(ThreadState *state, const uint8_t *insn);
int translate_bcond(ThreadState *state, const uint8_t *insn);
int translate_cbz(ThreadState *state, const uint8_t *insn);
int translate_cbnz(ThreadState *state, const uint8_t *insn);
int translate_tbz(ThreadState *state, const uint8_t *insn);
int translate_tbnz(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Compare Translation Functions
 * ============================================================================ */

int translate_cmp(ThreadState *state, const uint8_t *insn);
int translate_cmn(ThreadState *state, const uint8_t *insn);
int translate_tst(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Load/Store Translation Functions
 * ============================================================================ */

int translate_ldr(ThreadState *state, const uint8_t *insn);
int translate_str(ThreadState *state, const uint8_t *insn);
int translate_ldp(ThreadState *state, const uint8_t *insn);
int translate_stp(ThreadState *state, const uint8_t *insn);
int translate_ldrb(ThreadState *state, const uint8_t *insn);
int translate_strb(ThreadState *state, const uint8_t *insn);
int translate_ldrh(ThreadState *state, const uint8_t *insn);
int translate_strh(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * System Instruction Translation Functions
 * ============================================================================ */

int translate_mrs(ThreadState *state, const uint8_t *insn);
int translate_msr(ThreadState *state, const uint8_t *insn);
int translate_svc(ThreadState *state, const uint8_t *insn);
int translate_hlt(ThreadState *state, const uint8_t *insn);
int translate_brk(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * FP Translation Functions
 * ============================================================================ */

int translate_fmov(ThreadState *state, const uint8_t *insn);
int translate_fadd(ThreadState *state, const uint8_t *insn);
int translate_fsub(ThreadState *state, const uint8_t *insn);
int translate_fmul(ThreadState *state, const uint8_t *insn);
int translate_fdiv(ThreadState *state, const uint8_t *insn);
int translate_fsqrt(ThreadState *state, const uint8_t *insn);
int translate_fcmp(ThreadState *state, const uint8_t *insn);
int translate_fcvt(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * NEON Translation Functions
 * ============================================================================ */

int translate_ld1(ThreadState *state, const uint8_t *insn);
int translate_st1(ThreadState *state, const uint8_t *insn);
int translate_ld2(ThreadState *state, const uint8_t *insn);
int translate_st2(ThreadState *state, const uint8_t *insn);
int translate_ld3(ThreadState *state, const uint8_t *insn);
int translate_st3(ThreadState *state, const uint8_t *insn);
int translate_ld4(ThreadState *state, const uint8_t *insn);
int translate_st4(ThreadState *state, const uint8_t *insn);
int translate_dup(ThreadState *state, const uint8_t *insn);
int translate_ext(ThreadState *state, const uint8_t *insn);
int translate_tbl(ThreadState *state, const uint8_t *insn);
int translate_tbx(ThreadState *state, const uint8_t *insn);
int translate_ushr(ThreadState *state, const uint8_t *insn);
int translate_sshr(ThreadState *state, const uint8_t *insn);
int translate_shl(ThreadState *state, const uint8_t *insn);
int translate_scf(ThreadState *state, const uint8_t *insn);

/* ============================================================================
 * Additional Syscall Handlers - Process/Signal
 * ============================================================================ */

int syscall_kill(ThreadState *state);
int syscall_wait4(ThreadState *state);
int syscall_capget(ThreadState *state);
int syscall_capset(ThreadState *state);
int syscall_mincore(ThreadState *state);
int syscall_settimeofday(ThreadState *state);
int syscall_getcpu(ThreadState *state);
int syscall_prlimit(ThreadState *state);
int syscall_clone(ThreadState *state);
int syscall_execve(ThreadState *state);
int syscall_set_robust_list(ThreadState *state);
int syscall_get_robust_list(ThreadState *state);
int syscall_clock_getres(ThreadState *state);

/* ============================================================================
 * Network Syscall Handlers
 * ============================================================================ */

int syscall_socket(ThreadState *state);
int syscall_connect(ThreadState *state);
int syscall_sendto(ThreadState *state);
int syscall_recvfrom(ThreadState *state);
int syscall_epoll_create(ThreadState *state);
int syscall_epoll_ctl(ThreadState *state);
int syscall_epoll_wait(ThreadState *state);

/* ============================================================================
 * Helper Utilities
 * ============================================================================ */

void helper_debug_trace(const char *msg, uint64_t value);
uint64_t helper_perf_counter(void);

/* ============================================================================
 * Vector Conversion Helpers
 * ============================================================================ */

Vector128 v128_dup(uint8_t val);
uint8_t v128_extract_byte(Vector128 v, int index);
Vector128 v128_insert_byte(Vector128 v, int index, uint8_t val);
Vector128 v128_zip_lo(Vector128 a, Vector128 b);
Vector128 v128_zip_hi(Vector128 a, Vector128 b);

/* ============================================================================
 * Context Save/Restore
 * ============================================================================ */

void restore_cpu_context_full(CPUContext *ctx, const void *save_area);
void save_fp_context(uint64_t *save);
void restore_fp_context(uint64_t *save);

/* ============================================================================
 * Initialization Helpers
 * ============================================================================ */

void setup_signal_tramp(void);
void init_translation_cache(void);
void init_syscall_table(void);

/* ============================================================================
 * Signal Handling
 * ============================================================================ */

void signal_handler_fault(int sig, siginfo_t *info, void *context);
void init_signal_handlers(void);

/* ============================================================================
 * Common Rosetta Functions - Forward Declarations
 * ============================================================================ */

/* Entry point - initializes translation environment */
void rosetta_entry(void);

/* FP register management */
void set_fp_registers(uint64_t fpcr_value, uint64_t fpsr_value);
void clear_fp_registers(void);

/* Translation environment initialization */
void init_translation_env(void **entry_callback);

/* CPU context save/restore */
void save_cpu_context_full(CPUContext *ctx, void *save_area, long flags);

/* Translation functions */
void *translate_block(uint64_t guest_pc);
void execute_translated(ThreadState *state, void *block);

/* Memory operations (SIMD-optimized) */
void *rosetta_memchr_simd(const void *s, long n);
void *rosetta_memchr_simd_unaligned(const void *s, long n);
int   rosetta_strcmp_simd(const char *s1, const char *s2);
int   rosetta_strncmp_simd(const char *s1, const char *s2, size_t n);
int   rosetta_memcmp_simd(const void *s1, const void *s2, size_t n);
void *rosetta_memcpy_aligned(void *dest, const void *src, size_t n);
void *rosetta_memset_simd(void *s, int c, size_t n);

/* String and Memory utilities (Session 14) */
size_t rosetta_strlen(const char *s);
char  *rosetta_strcpy(char *dest, const char *src);
char  *rosetta_strcat(char *dest, const char *src);
int    rosetta_strcmp(const char *s1, const char *s2);
int    rosetta_strncmp(const char *s1, const char *s2, size_t n);
void  *rosetta_memcpy(void *dest, const void *src, size_t n);
void  *rosetta_memmove(void *dest, const void *src, size_t n);
int    rosetta_memcmp(const void *s1, const void *s2, size_t n);
void  *rosetta_memset(void *s, int c, size_t n);
void  *rosetta_memchr(const void *s, int c, size_t n);

/* FP register access */
uint32_t read_fpcr(void);
void write_fpcr(uint32_t val);
uint32_t read_fpsr(void);
void write_fpsr(uint32_t val);

/* Syscall handling */
int64_t handle_syscall(ThreadState *state, int nr);

/* Daemon functions */
void rosettad_entry(void);
void init_daemon_state(void);

/* ============================================================================
 * Session 15: Switch Case Handlers and ELF Parsing
 * ============================================================================ */

/* Switch case handlers */
uint64_t switch_case_handler_1a(uint64_t value);
uint64_t switch_case_handler_2b(uint64_t value);

/* ELF parsing helpers */
int elf_parse_header(const void *data, void *header);
int elf_lookup_section(const void *data, uint32_t section_idx, void *result);

/* Bitmask helper for ARM64 immediates */
int is_bitmask_immediate(uint64_t bitmask, int *rot, int *imm);

/* Translation infrastructure helpers */
int translation_cache_init(void);
void translation_cache_flush(void);
void *translation_cache_lookup(uint64_t guest_pc);

/* Memory management helpers - Session 17 */
void *rosetta_malloc(size_t size);
void rosetta_free(void *ptr);
void *rosetta_calloc(size_t nmemb, size_t size);

/* Runtime support - Session 17 */
void rosetta_abort(void);
int rosetta_atexit(void (*func)(void));

/* Code cache helpers - Session 18 */
void *code_cache_alloc(size_t size);
void code_cache_free(void *ptr);
int code_cache_protect(void *addr, size_t size, int prot);

/* Debug helpers - Session 18 */
void debug_trace(const char *msg, uint64_t value);
void debug_dump_regs(void);

/* Translation helpers - Session 19 */
void translate_movz(ThreadState *state, const uint8_t *insn);
void translate_movk(ThreadState *state, const uint8_t *insn);
void translate_movn(ThreadState *state, const uint8_t *insn);

/* Vector helpers - Session 19 */
Vector128 v128_dupw(uint32_t val);
Vector128 v128_dupd(double val);

/* ============================================================================
 * Session 20: Additional Vector and Translation Helpers
 * ============================================================================ */

/* Vector extraction/insertion operations */
uint32_t v128_extract_word(Vector128 v, int index);
Vector128 v128_insert_word(Vector128 v, int index, uint32_t val);
uint64_t v128_extract_dword(Vector128 v, int index);
Vector128 v128_insert_dword(Vector128 v, int index, uint64_t val);
Vector128 v128_mov(Vector128 v);
Vector128 v128_tbl(Vector128 t1, Vector128 t2, Vector128 i);

/* Translation block optimization helpers */
int translate_block_optimize(void *block, size_t size);
int translate_block_link(void *from_block, void *to_block, size_t size);
int translate_block_unlink(void *block);

/* ELF parsing helpers */
int elf_find_symbol(const void *elf_base, const char *name, void *sym_out);
int elf_get_section_by_name(const void *elf_base, const char *name, void *shdr_out);
int elf_relocate(const void *elf_base, uint64_t load_offset);

/* ============================================================================
 * Session 21: vDSO and ELF Parsing Functions
 * ============================================================================ */

/* SIMD helper functions */
uint64_t has_zero_byte(uint64_t x);

/* ELF64 parsing helpers */
typedef struct {
    uint32_t e_type;
    uint32_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint32_t e_ehsize;
    uint32_t e_phentsize;
    uint32_t e_phnum;
    uint32_t e_shentsize;
    uint32_t e_shnum;
    uint32_t e_shstrndx;
} elf64_header_info_t;

int elf64_parse_header(const void *base, void *info);
const void *elf64_find_section_by_type(const void *base, const void *info, uint32_t type);
const void *elf64_find_section_by_name(const void *base, const void *info, const char *name);
int elf64_lookup_symbol(const void *base, const void *symtab, const void *strtab,
                        const char *sym_name, void **sym_value);
uint64_t elf64_compute_load_offset(const void *base, const void *info);

/* VDSO helpers */
void *vdso_lookup_symbol(const void *vdso_base, const void *info, const char *sym_name);
int vdso_init(const void *vdso_base, void **clock_getres_out,
              void **gettimeofday_out, void **clock_gettime_out);

/* ============================================================================
 * Session 22: Advanced NEON and Vector Operations
 * ============================================================================ */

/* Vector permutation and reduction operations */
Vector128 v128_padd(Vector128 a);
Vector128 v128_abs(Vector128 a);
Vector128 v128_sat_add(Vector128 a, Vector128 b);
Vector128 v128_sat_sub(Vector128 a, Vector128 b);
Vector128 v128_rev(Vector128 a);
Vector128 v128_cnt(Vector128 a);
uint64_t v128_addv(Vector128 a);

/* Session 22: Advanced NEON operations */
Vector128 v128_cls(Vector128 a);
Vector128 v128_clz(Vector128 a);
Vector128 v128_ctz(Vector128 a);
Vector128 v128_fcpy(Vector128 a);
Vector128 v128_dup_lane(Vector128 a, int lane);
Vector128 v128_ext(Vector128 a, Vector128 b, int imm4);

/* ============================================================================
 * Session 23: ELF Dynamic Linker Support
 * ============================================================================ */

/* ELF dynamic tag types */
typedef struct {
    int64_t d_tag;      /* Dynamic entry type */
    union {
        uint64_t d_val; /* Integer value */
        uint64_t d_ptr; /* Pointer value */
    } d_un;
} Elf64_Dyn;

/* ELF relocation entry types */
typedef struct {
    uint64_t r_offset;  /* Address */
    uint64_t r_info;    /* Relocation type and symbol index */
    int64_t r_addend;   /* Addend for relocation calculation */
} Elf64_Rela;

/* Dynamic section parsing */
int elf_parse_dynamic_section(const void *base, void *dyn_info);
int elf_find_dynamic_entry(const void *dyn, int64_t tag, uint64_t *value);

/* Relocation processing */
int elf_process_relocations(const void *elf_base, uint64_t load_offset);
int elf_apply_relocation(const void *elf_base, const Elf64_Rela *rela, uint64_t load_offset);
int elf_process_dynamic_relocs(const void *elf_base, uint64_t load_offset);
int elf_process_plt_relocs(const void *elf_base, uint64_t load_offset);

/* Symbol resolution with hash tables */
uint32_t elf_hash_symbol(const char *name);
uint32_t elf_gnu_hash_symbol(const char *name);
int elf_hash_lookup(const void *symtab, const void *strtab,
                    const void *hashtab, const char *name, void **sym_value);

/* GOT/PLT setup */
int elf_setup_got(const void *elf_base, uint64_t load_offset);
int elf_setup_plt(const void *elf_base, uint64_t load_offset);
int elf_resolve_plt_entry(const void *elf_base, uint64_t plt_index);
int elf_init_plt_got(const void *elf_base, uint64_t load_offset);

/* Version and symbol binding */
int elf_get_symbol_binding(const void *sym, int *binding);
int elf_get_symbol_type(const void *sym, int *type);
int elf_check_symbol_version(const void *base, const char *name);

/* ============================================================================
 * Session 23: Translation Infrastructure Extensions
 * ============================================================================ */

/* Translation block structure */
typedef struct {
    uint64_t guest_pc;      /* Guest PC */
    uint64_t host_pc;       /* Host PC (translated code) */
    uint32_t size;          /* Size of translated code */
    uint32_t flags;         /* Translation flags */
    uint32_t hash;          /* Hash of guest PC */
    uint32_t refcount;      /* Reference count */
    uint64_t chain[2];      /* Chained successor blocks */
} TranslatedBlock;

/* Translation cache operations */
void *translation_alloc_block(uint64_t guest_pc, size_t code_size);
int translation_free_block(void *block);
void translation_invalidate_block(uint64_t guest_pc);
void translation_flush_cache(void *addr, size_t size);

/* Block chaining for optimization */
int translation_chain_blocks(void *from_block, void *to_block, int index);
int translation_unchain_blocks(void *block);
void *translation_get_chained_block(void *block, int index);

/* Code cache operations */
int code_cache_init(size_t size);
void code_cache_cleanup(void);
void *code_cache_alloc_aligned(size_t size, size_t alignment);
int code_cache_mark_executable(void *addr, size_t size);

/* ============================================================================
 * Session 24: Additional Vector and Memory Operations
 * ============================================================================ */

/* Additional NEON vector operations */
Vector128 v128_bic(Vector128 a, Vector128 b);          /* Bitwise bit clear */
Vector128 v128_orn(Vector128 a, Vector128 b);          /* Bitwise OR NOT */
Vector128 v128_eor_not(Vector128 a, Vector128 b);      /* EOR with NOT of second */
Vector128 v128_rbit(Vector128 a);                       /* Reverse bit order */
Vector128 v128_shl_narrow(Vector128 a, int shift);     /* Narrowing shift left */
Vector128 v128_sshr_narrow(Vector128 a, int shift);    /* Narrowing signed shift right */
Vector128 v128_ushr_narrow(Vector128 a, int shift);    /* Narrowing unsigned shift right */

/* Vector narrowing operations */
Vector128 v128_narrow_s16_u8(Vector128 a);             /* Signed 16-bit to unsigned 8-bit */
Vector128 v128_narrow_s16_s8(Vector128 a);             /* Signed 16-bit to signed 8-bit */
Vector128 v128_narrow_u16_u8(Vector128 a);             /* Unsigned 16-bit to unsigned 8-bit */
Vector128 v128_narrow_s32_s16(Vector128 a);            /* Signed 32-bit to signed 16-bit */
Vector128 v128_narrow_u32_u16(Vector128 a);            /* Unsigned 32-bit to unsigned 16-bit */

/* Vector widening operations */
Vector128 v128_widen_u8_u16_lo(Vector128 a);           /* Unsigned widen low half */
Vector128 v128_widen_u8_u16_hi(Vector128 a);           /* Unsigned widen high half */
Vector128 v128_widen_s8_s16_lo(Vector128 a);           /* Signed widen low half */
Vector128 v128_widen_s8_s16_hi(Vector128 a);           /* Signed widen high half */

/* Floating-point vector operations */
Vector128 v128_fadd(Vector128 a, Vector128 b);         /* FP vector add */
Vector128 v128_fsub(Vector128 a, Vector128 b);         /* FP vector subtract */
Vector128 v128_fmul(Vector128 a, Vector128 b);         /* FP vector multiply */
Vector128 v128_fdiv(Vector128 a, Vector128 b);         /* FP vector divide */
Vector128 v128_fsqrt(Vector128 a);                     /* FP vector square root */
Vector128 v128_frecpe(Vector128 a);                    /* FP reciprocal estimate */
Vector128 v128_frsqrte(Vector128 a);                   /* FP reciprocal sqrt estimate */
Vector128 v128_fcmp_eq(Vector128 a, Vector128 b);      /* FP compare equal */
Vector128 v128_fcmp_lt(Vector128 a, Vector128 b);      /* FP compare less than */

/* Memory management utilities */
void *rosetta_mmap_anonymous(size_t size, int prot);
int rosetta_munmap_region(void *addr, size_t size);
int rosetta_mprotect_region(void *addr, size_t size, int prot);
void *rosetta_memalign(size_t alignment, size_t size);
void rosetta_memzero(void *s, size_t n);

/* String utilities extended */
char *rosetta_strncpy(char *dest, const char *src, size_t n);
char *rosetta_strncat(char *dest, const char *src, size_t n);
int rosetta_strcasecmp(const char *s1, const char *s2);
char *rosetta_strchr(const char *s, int c);
char *rosetta_strrchr(const char *s, int c);
size_t rosetta_strspn(const char *s, const char *accept);
size_t rosetta_strcspn(const char *s, const char *reject);

/* Memory utilities extended */
void *rosetta_memccpy(void *dest, const void *src, int c, size_t n);
void *rosetta_memchr_inv(const void *s, int c, size_t n);
int rosetta_memcmp_consttime(const void *s1, const void *s2, size_t n);

/* Bit manipulation utilities */
uint32_t bitreverse32(uint32_t x);
uint64_t bitreverse64(uint64_t x);
uint32_t count_leading_zeros32(uint32_t x);
uint64_t count_leading_zeros64(uint64_t x);
uint32_t count_set_bits32(uint32_t x);
uint64_t count_set_bits64(uint64_t x);

/* Translation infrastructure extended */
int translation_block_is_valid(uint64_t guest_pc);
void translation_block_set_flag(uint64_t guest_pc, uint32_t flag);
void translation_block_clear_flag(uint64_t guest_pc, uint32_t flag);

/* Signal handling extended */
int setup_signal_trampoline(void *handler);
void cleanup_signal_handlers(void);
int signal_block(int sig);
int signal_unblock(int sig);

/* ============================================================================
 * Session 25: Additional Utility Functions
 * ============================================================================ */

/* Additional vector utility functions */
Vector128 v128_shuffle(Vector128 v, Vector128 indices);    /* Vector shuffle */
Vector128 v128_interleave_lo(Vector128 a, Vector128 b);    /* Interleave low elements */
Vector128 v128_interleave_hi(Vector128 a, Vector128 b);    /* Interleave high elements */
uint32_t v128_test_zero(Vector128 v);                       /* Test if any element is zero */
uint32_t v128_test_sign(Vector128 v);                       /* Test sign of elements */

/* Additional translation helpers */
int translate_ldrb_imm(ThreadState *state, const uint8_t *insn);  /* Load register byte immediate */
int translate_strb_imm(ThreadState *state, const uint8_t *insn);  /* Store register byte immediate */
int translate_ldrh_imm(ThreadState *state, const uint8_t *insn);  /* Load register halfword immediate */
int translate_strh_imm(ThreadState *state, const uint8_t *insn);  /* Store register halfword immediate */

/* ELF helper functions */
int elf_validate_magic(const void *base);                   /* Validate ELF magic number */
uint64_t elf_get_entry_point(const void *base);             /* Get ELF entry point */

/* Runtime utility functions */
int rosetta_getpagesize(void);                              /* Get system page size */
uint64_t rosetta_get_tick_count(void);                      /* Get system tick count */

/* ============================================================================
 * Session 26: Additional Translation and Vector Functions
 * ============================================================================ */

/* Additional vector utility functions */
Vector128 v128_fcmp_gt(Vector128 a, Vector128 b);          /* FP compare greater than */
Vector128 v128_fcmp_le(Vector128 a, Vector128 b);          /* FP compare less or equal */
Vector128 v128_fcmp_ge(Vector128 a, Vector128 b);          /* FP compare greater or equal */
Vector128 v128_frint(Vector128 a);                          /* FP round to integer */
Vector128 v128_fabs(Vector128 a);                           /* FP absolute value */
Vector128 v128_fneg(Vector128 a);                           /* FP negate */

/* Additional translation helpers - register offset */
int translate_ldr_reg(ThreadState *state, const uint8_t *insn);  /* Load register with register offset */
int translate_str_reg(ThreadState *state, const uint8_t *insn);  /* Store register with register offset */
int translate_ldrb_reg(ThreadState *state, const uint8_t *insn); /* Load register byte with register offset */
int translate_strb_reg(ThreadState *state, const uint8_t *insn); /* Store register byte with register offset */

/* Additional translation helpers - pre/post indexed */
int translate_ldr_pre(ThreadState *state, const uint8_t *insn);  /* Load register pre-indexed */
int translate_ldr_post(ThreadState *state, const uint8_t *insn); /* Load register post-indexed */
int translate_str_pre(ThreadState *state, const uint8_t *insn);  /* Store register pre-indexed */
int translate_str_post(ThreadState *state, const uint8_t *insn); /* Store register post-indexed */

/* ============================================================================
 * Session 27: Advanced NEON Translation and Vector Operations
 * ============================================================================ */

/* Additional vector operations - FP rounding variants */
Vector128 v128_frintn(Vector128 a);                             /* FP round to nearest, ties to even */
Vector128 v128_frintz(Vector128 a);                             /* FP round toward zero */
Vector128 v128_frintp(Vector128 a);                             /* FP round toward +infinity */
Vector128 v128_frintm(Vector128 a);                             /* FP round toward -infinity */
Vector128 v128_fcvtns(Vector128 a);                             /* FP convert to signed integer */
Vector128 v128_fcvtnu(Vector128 a);                             /* FP convert to unsigned integer */

/* Translation helpers - signed variants */
int translate_ldrsb_imm(ThreadState *state, const uint8_t *insn);   /* Load register signed byte imm */
int translate_ldrsh_imm(ThreadState *state, const uint8_t *insn);   /* Load register signed halfword imm */
int translate_ldrsw_imm(ThreadState *state, const uint8_t *insn);   /* Load register signed word imm */

/* Translation helpers - register offset signed variants */
int translate_ldrsb_reg(ThreadState *state, const uint8_t *insn);   /* Load register signed byte reg */
int translate_ldrsh_reg(ThreadState *state, const uint8_t *insn);   /* Load register signed halfword reg */
int translate_ldrsw_reg(ThreadState *state, const uint8_t *insn);   /* Load register signed word reg */

/* Translation helpers - load/store pair immediate */
int translate_ldp_imm(ThreadState *state, const uint8_t *insn);     /* Load pair immediate */
int translate_stp_imm(ThreadState *state, const uint8_t *insn);     /* Store pair immediate */

/* Additional ALU translation variants */
int translate_add_imm(ThreadState *state, const uint8_t *insn);     /* Add immediate */
int translate_sub_imm(ThreadState *state, const uint8_t *insn);     /* Subtract immediate */
int translate_and_imm(ThreadState *state, const uint8_t *insn);     /* AND immediate */
int translate_orr_imm(ThreadState *state, const uint8_t *insn);     /* ORR immediate */

/* ============================================================================
 * Session 28: Advanced SIMD Saturation and Narrowing Operations
 * ============================================================================ */

/* Saturating arithmetic (8-bit, 16-bit, 32-bit, 64-bit) */
Vector128 v128_sqadd(Vector128 a, Vector128 b);                /* Signed saturating add (8-bit) */
Vector128 v128_uqadd(Vector128 a, Vector128 b);                /* Unsigned saturating add (8-bit) */
Vector128 v128_sqsub(Vector128 a, Vector128 b);                /* Signed saturating subtract (8-bit) */
Vector128 v128_uqsub(Vector128 a, Vector128 b);                /* Unsigned saturating subtract (8-bit) */
Vector128 v128_sqadd_16(Vector128 a, Vector128 b);             /* Signed saturating add (16-bit) */
Vector128 v128_uqadd_16(Vector128 a, Vector128 b);             /* Unsigned saturating add (16-bit) */
Vector128 v128_sqsub_16(Vector128 a, Vector128 b);             /* Signed saturating subtract (16-bit) */
Vector128 v128_uqsub_16(Vector128 a, Vector128 b);             /* Unsigned saturating subtract (16-bit) */
Vector128 v128_sqadd_32(Vector128 a, Vector128 b);             /* Signed saturating add (32-bit) */
Vector128 v128_uqadd_32(Vector128 a, Vector128 b);             /* Unsigned saturating add (32-bit) */
Vector128 v128_sqsub_32(Vector128 a, Vector128 b);             /* Signed saturating subtract (32-bit) */
Vector128 v128_uqsub_32(Vector128 a, Vector128 b);             /* Unsigned saturating subtract (32-bit) */
Vector128 v128_sqadd_64(Vector128 a, Vector128 b);             /* Signed saturating add (64-bit) */
Vector128 v128_uqadd_64(Vector128 a, Vector128 b);             /* Unsigned saturating add (64-bit) */
Vector128 v128_sqsub_64(Vector128 a, Vector128 b);             /* Signed saturating subtract (64-bit) */
Vector128 v128_uqsub_64(Vector128 a, Vector128 b);             /* Unsigned saturating subtract (64-bit) */

/* Saturating doubling multiply high */
Vector128 v128_sqdmulh(Vector128 a, Vector128 b);              /* Signed saturating doubling multiply high (16-bit) */
Vector128 v128_sqrdmulh(Vector128 a, Vector128 b);             /* Signed saturating rounding doubling multiply high (16-bit) */
Vector128 v128_sqdmulh_32(Vector128 a, Vector128 b);           /* Signed saturating doubling multiply high (32-bit) */
Vector128 v128_sqrdmulh_32(Vector128 a, Vector128 b);          /* Signed saturating rounding doubling multiply high (32-bit) */

/* ============================================================================
 * Session 29: NEON Narrowing and Widening Operations (Additional)
 * ============================================================================ */

/* Narrowing operations (64-bit to 32-bit, saturating) */
Vector128 v128_narrow_s64_s32(Vector128 a);                    /* Narrow signed 64-bit to signed 32-bit */
Vector128 v128_narrow_u64_u32(Vector128 a);                    /* Narrow unsigned 64-bit to unsigned 32-bit */

/* Widening operations (16-bit to 32-bit, sign/zero extend) */
Vector128 v128_widen_s16_s32_lo(Vector128 a);                  /* Signed widen low half 16-bit to 32-bit */
Vector128 v128_widen_s16_s32_hi(Vector128 a);                  /* Signed widen high half 16-bit to 32-bit */
Vector128 v128_widen_u16_u32_lo(Vector128 a);                  /* Unsigned widen low half 16-bit to 32-bit */
Vector128 v128_widen_u16_u32_hi(Vector128 a);                  /* Unsigned widen high half 16-bit to 32-bit */

/* Pairwise add operations (two-argument version) */
Vector128 v128_padd2(Vector128 a, Vector128 b);                /* Pairwise add (32-bit elements, two vectors) */
Vector128 v128_padd_16(Vector128 a, Vector128 b);              /* Pairwise add (16-bit elements) */
Vector128 v128_padd_8(Vector128 a, Vector128 b);               /* Pairwise add (8-bit elements) */

/* Pairwise add and accumulate long operations */
Vector128 v128_padal_s8(Vector128 a, Vector128 b);             /* Signed pairwise add long accumulate (8-bit) */
Vector128 v128_padal_u8(Vector128 a, Vector128 b);             /* Unsigned pairwise add long accumulate (8-bit) */
Vector128 v128_padal_s16(Vector128 a, Vector128 b);            /* Signed pairwise add long accumulate (16-bit) */
Vector128 v128_padal_u16(Vector128 a, Vector128 b);            /* Unsigned pairwise add long accumulate (16-bit) */
Vector128 v128_padal_s32(Vector128 a, Vector128 b);            /* Signed pairwise add long accumulate (32-bit) */
Vector128 v128_padal_u32(Vector128 a, Vector128 b);            /* Unsigned pairwise add long accumulate (32-bit) */

/* ============================================================================
 * Session 30: Narrowing with Shift and Saturating Convert Operations
 * ============================================================================ */

/* Saturating narrowing shift right (SQSHRN/UQSHRN) - with shift parameter */
Vector128 v128_sqshrn_s16_u8(Vector128 a, int shift);          /* Signed saturating narrow 16->8 with shift */
Vector128 v128_sqshrn_s32_s16(Vector128 a, int shift);         /* Signed saturating narrow 32->16 with shift */
Vector128 v128_sqshrn_s64_s32(Vector128 a, int shift);         /* Signed saturating narrow 64->32 with shift */
Vector128 v128_uqshrn_u16_u8(Vector128 a, int shift);          /* Unsigned saturating narrow 16->8 with shift */
Vector128 v128_uqshrn_u32_u16(Vector128 a, int shift);         /* Unsigned saturating narrow 32->16 with shift */
Vector128 v128_uqshrn_u64_u32(Vector128 a, int shift);         /* Unsigned saturating narrow 64->32 with shift */

/* Saturating rounding narrowing shift right (SQRSHRN/UQRSHRN) - with shift and rounding */
Vector128 v128_sqrshrn_s16_u8(Vector128 a, int shift);         /* Signed saturating rounding narrow 16->8 */
Vector128 v128_sqrshrn_s32_s16(Vector128 a, int shift);        /* Signed saturating rounding narrow 32->16 */
Vector128 v128_sqrshrn_s64_s32(Vector128 a, int shift);        /* Signed saturating rounding narrow 64->32 */
Vector128 v128_uqrshrn_u16_u8(Vector128 a, int shift);         /* Unsigned saturating rounding narrow 16->8 */
Vector128 v128_uqrshrn_u32_u16(Vector128 a, int shift);        /* Unsigned saturating rounding narrow 32->16 */
Vector128 v128_uqrshrn_u64_u32(Vector128 a, int shift);        /* Unsigned saturating rounding narrow 64->32 */

/* Saturating extract narrow (SQXTN/UQXTN) - no shift, just saturation */
Vector128 v128_sqxtn_s16_s32(Vector128 a);                     /* Signed saturating narrow 32->16 */
Vector128 v128_sqxtn_s8_s16(Vector128 a);                      /* Signed saturating narrow 16->8 */
Vector128 v128_sqxtn_s32_s64(Vector128 a);                     /* Signed saturating narrow 64->32 */
Vector128 v128_uqxtn_u16_u32(Vector128 a);                     /* Unsigned saturating narrow 32->16 */
Vector128 v128_uqxtn_u8_u16(Vector128 a);                      /* Unsigned saturating narrow 16->8 */
Vector128 v128_uqxtn_u32_u64(Vector128 a);                     /* Unsigned saturating narrow 64->32 */

/* Saturating extract unsigned narrow (SQXTUN) - signed to unsigned conversion */
Vector128 v128_sqxtun_s16_u32(Vector128 a);                    /* Signed to unsigned narrow 32->16 */
Vector128 v128_sqxtun_s8_u16(Vector128 a);                     /* Signed to unsigned narrow 16->8 */
Vector128 v128_sqxtun_s32_u64(Vector128 a);                    /* Signed to unsigned narrow 64->32 */

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Vector by Scalar
 * ============================================================================ */

/* Vector by scalar multiplication */
Vector128 v128_mul_scalar_u16(Vector128 a, uint16_t scalar);   /* Unsigned 16-bit mul by scalar */
Vector128 v128_mul_scalar_u32(Vector128 a, uint32_t scalar);   /* Unsigned 32-bit mul by scalar */
Vector128 v128_mul_scalar_s16(Vector128 a, int16_t scalar);    /* Signed 16-bit mul by scalar */
Vector128 v128_mul_scalar_s32(Vector128 a, int32_t scalar);    /* Signed 32-bit mul by scalar */

/* Vector by scalar multiply-accumulate (MLA) */
Vector128 v128_mla_scalar_u16(Vector128 a, Vector128 b, uint16_t scalar);  /* Unsigned 16-bit MLA */
Vector128 v128_mla_scalar_u32(Vector128 a, Vector128 b, uint32_t scalar);  /* Unsigned 32-bit MLA */
Vector128 v128_mla_scalar_s16(Vector128 a, Vector128 b, int16_t scalar);   /* Signed 16-bit MLA */
Vector128 v128_mla_scalar_s32(Vector128 a, Vector128 b, int32_t scalar);   /* Signed 32-bit MLA */

/* Vector by scalar multiply-subtract (MLS) */
Vector128 v128_mls_scalar_u16(Vector128 a, Vector128 b, uint16_t scalar);  /* Unsigned 16-bit MLS */
Vector128 v128_mls_scalar_u32(Vector128 a, Vector128 b, uint32_t scalar);  /* Unsigned 32-bit MLS */
Vector128 v128_mls_scalar_s16(Vector128 a, Vector128 b, int16_t scalar);   /* Signed 16-bit MLS */
Vector128 v128_mls_scalar_s32(Vector128 a, Vector128 b, int32_t scalar);   /* Signed 32-bit MLS */

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Extended Table Lookup
 * ============================================================================ */

/* Multi-register table lookup (TBL) */
Vector128 v128_tbl_2regs(Vector128 t1, Vector128 t2, Vector128 i);   /* TBL with 2 tables */
Vector128 v128_tbl_3regs(Vector128 t1, Vector128 t2, Vector128 t3, Vector128 i);  /* TBL with 3 tables */
Vector128 v128_tbl_4regs(Vector128 t1, Vector128 t2, Vector128 t3, Vector128 t4, Vector128 i);  /* TBL with 4 tables */

/* Multi-register table lookup extension (TBX) */
Vector128 v128_tbx_2regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 i);   /* TBX with 2 tables */
Vector128 v128_tbx_3regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 t3, Vector128 i);  /* TBX with 3 tables */
Vector128 v128_tbx_4regs(Vector128 dest, Vector128 t1, Vector128 t2, Vector128 t3, Vector128 t4, Vector128 i);  /* TBX with 4 tables */

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Complex Number
 * ============================================================================ */

/* Floating-point complex multiply-accumulate (FCMLA) */
Vector128 v128_fcmla(Vector128 a, Vector128 b, Vector128 c, int rot);  /* Complex MLA with rotation */

/* Floating-point complex add (FCADD) */
Vector128 v128_fcadd(Vector128 a, Vector128 b, int rot);  /* Complex add with rotation */

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Floating-point Narrowing
 * ============================================================================ */

/* Floating-point convert to lower precision (FCVTN) */
Vector128 v128_fcvtn_f32_f64(Vector128 a);  /* Convert double to float */
Vector128 v128_fcvtn_f16_f32(Vector128 a);  /* Convert float to half */

/* Floating-point convert to higher precision (FCVTN2) */
Vector128 v128_fcvtn2_f32_f64(Vector128 a);  /* Convert double to float (high) */

/* Floating-point multiply long (FCMULL) */
Vector128 v128_fcmull_f32_f16(Vector128 a, Vector128 b);  /* FP multiply long half->float */

/* ============================================================================
 * Session 31: Advanced SIMD Operations - Polynomial
 * ============================================================================ */

/* Polynomial multiply long (PMULL) */
Vector128 v128_pmull(Vector128 a, Vector128 b);   /* Polynomial multiply 8-bit to 16-bit */
Vector128 v128_pmull2(Vector128 a, Vector128 b);  /* Polynomial multiply high 8-bit to 16-bit */

/* Polynomial multiply (PMUL) */
Vector128 v128_pmul(Vector128 a, Vector128 b);    /* Polynomial multiply 8-bit */

/* ============================================================================
 * Session 32: Cryptographic Extensions
 * ============================================================================ */

/* AES Cryptographic Extensions */
Vector128 v128_aese(Vector128 state, Vector128 round_key);  /* AES round encryption */
Vector128 v128_aesd(Vector128 state, Vector128 round_key);  /* AES round decryption */
Vector128 v128_aesmc(Vector128 col);                        /* AES mix columns */
Vector128 v128_aesimc(Vector128 col);                       /* AES inverse mix columns */

/* SHA Cryptographic Extensions */
Vector128 v128_sha1c(Vector128 hash, Vector128 data, Vector128 wk);  /* SHA1 hash update (C) */
Vector128 v128_sha1p(Vector128 hash, Vector128 data, Vector128 wk);  /* SHA1 hash update (P) */
Vector128 v128_sha1m(Vector128 hash, Vector128 data, Vector128 wk);  /* SHA1 hash update (M) */
Vector128 v128_sha1su0(Vector128 data, Vector128 wk);               /* SHA1 schedule update 0 */
Vector128 v128_sha1su1(Vector128 data, Vector128 wk);               /* SHA1 schedule update 1 */
Vector128 v128_sha256h(Vector128 hash, Vector128 data, Vector128 wk); /* SHA256 hash update (H) */
Vector128 v128_sha256h2(Vector128 hash, Vector128 data, Vector128 wk);/* SHA256 hash update (H2) */
Vector128 v128_sha256su0(Vector128 data, Vector128 wk);              /* SHA256 schedule update 0 */
Vector128 v128_sha256su1(Vector128 data, Vector128 wk);              /* SHA256 schedule update 1 */

/* CRC32 Extensions (word-sized variants) */
uint32_t crc32b(uint32_t crc, uint8_t byte);      /* CRC32 byte */
uint32_t crc32h(uint32_t crc, uint16_t halfword); /* CRC32 halfword */
uint32_t crc32w(uint32_t crc, uint32_t word);     /* CRC32 word */
uint64_t crc32x(uint64_t crc, uint64_t dword);    /* CRC32 doubleword */

/* ============================================================================
 * Session 32: Random Number Generation
 * ============================================================================ */

/* Random Number Generator */
uint64_t rndr(void);    /* Random number */
uint64_t rndrrs(void);  /* Reseeded random number */

/* ============================================================================
 * Session 32: Additional SIMD Utility Functions
 * ============================================================================ */

/* Vector bit manipulation */
Vector128 v128_bswap(Vector128 a);           /* Byte swap (reverse byte order) */
Vector128 v128_bitsel(Vector128 a, Vector128 b, Vector128 c);  /* Bit select (cond ? a : b) */

/* Vector narrowing with rounding */
Vector128 v128_rshrn(Vector128 a, int shift);   /* Rounded shift right narrow */
Vector128 v128_srshrn(Vector128 a, int shift);  /* Signed rounded shift right narrow */
Vector128 v128_urshrn(Vector128 a, int shift);  /* Unsigned rounded shift right narrow */

/* Vector convert to floating-point */
Vector128 v128_scvtf(Vector128 a, int fracbits);  /* Signed convert to float */
Vector128 v128_ucvtf(Vector128 a, int fracbits);  /* Unsigned convert to float */

/* Vector convert from floating-point */
Vector128 v128_fcvts(Vector128 a, int fracbits);  /* Float convert to signed */
Vector128 v128_fcvtu(Vector128 a, int fracbits);  /* Float convert to unsigned */

/* ============================================================================
 * Session 34: Additional Utility Functions
 * ============================================================================ */

/* Additional String Utilities */
char *rosetta_strdup(const char *s);                    /* Duplicate a string */
char *rosetta_strstr(const char *haystack, const char *needle);  /* Find substring */
char *rosetta_strpbrk(const char *s, const char *charset);       /* Find first match from set */
char *rosetta_strtok(char *str, const char *delim);             /* Tokenize string */
void *rosetta_memmem(const void *haystack, size_t haystack_len,
                     const void *needle, size_t needle_len);     /* Find memory region */
void *rosetta_memrchr(const void *s, int c, size_t n);          /* Find last occurrence */

/* Additional Integer/Bit Utilities */
uint32_t count_trailing_zeros32(uint32_t x);            /* Count trailing zeros (32-bit) */
uint64_t count_trailing_zeros64(uint64_t x);            /* Count trailing zeros (64-bit) */
bool is_power_of_2(uint64_t x);                          /* Check if power of 2 */
uint64_t round_up_to_pow2(uint64_t x);                   /* Round up to power of 2 */

/* Translation Infrastructure Utilities */
size_t translation_cache_get_size(void);                 /* Get cache size */
bool translation_cache_is_full(void);                    /* Check if cache full */
size_t code_cache_get_free_space(void);                  /* Get free code cache space */
void code_cache_reset(void);                             /* Reset code cache */

/* Additional ELF Utilities */
uint64_t elf_get_section_offset(const void *base, uint32_t section_index);  /* Get section offset */
uint64_t elf_get_section_size(const void *base, uint32_t section_index);    /* Get section size */
bool elf_is_valid_class64(const void *base);             /* Check if 64-bit ELF */
bool elf_is_valid_machine_aarch64(const void *base);     /* Check if AArch64 ELF */

/* Additional Memory Utilities */
void *rosetta_memchr_eq(const void *s, const uint8_t mask[16], size_t n);  /* Find byte in mask */
void *rosetta_memcpy_nonoverlapping(void *dest, const void *src, size_t n); /* Non-overlapping copy */
void *rosetta_memmove_safe(void *dest, const void *src, size_t n);         /* Safe memory move */
void rosetta_memswap(void *a, void *b, size_t n);        /* Swap memory regions */
void *rosetta_memfill_word(void *dest, uint64_t word, size_t n);  /* Fill with word pattern */

/* Additional String Utilities - Length Limited */
size_t rosetta_strnlen(const char *s, size_t maxlen);    /* String length with limit */
size_t rosetta_strlcpy(char *dest, const char *src, size_t destsize);  /* Safe string copy */
size_t rosetta_strlcat(char *dest, const char *src, size_t destsize);  /* Safe string concat */

#endif /* ROSETTA_REFACTORED_H */
