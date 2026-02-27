/* ============================================================================
 * Rosetta Refactored - Debug Support Implementation
 * ============================================================================
 *
 * This module implements debug support functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_debug.h"
#include "rosetta_refactored_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

/* ============================================================================
 * Debug State
 * ============================================================================ */

static int g_debug_level = ROS_DEBUG_ERROR;
static int g_debug_output = ROS_DEBUG_OUT_STDERR;
static FILE *g_debug_file = NULL;
static bool g_debug_initialized = false;

/* ============================================================================
 * Trace Buffer
 * ============================================================================ */

static rosetta_trace_entry_t g_trace_buffer[ROS_TRACE_BUFFER_SIZE];
static uint32_t g_trace_head = 0;
static uint32_t g_trace_count = 0;
static bool g_trace_initialized = false;

/* ============================================================================
 * Debug Subsystem Functions
 * ============================================================================ */

/**
 * rosetta_debug_init - Initialize debug subsystem
 * @level: Debug level
 * @output: Output destination
 * @logfile: Path to log file
 * Returns: 0 on success, -1 on error
 */
int rosetta_debug_init(int level, int output, const char *logfile)
{
    if (g_debug_initialized) {
        rosetta_debug_cleanup();
    }

    g_debug_level = level;
    g_debug_output = output;

    if ((output & ROS_DEBUG_OUT_FILE) && logfile) {
        g_debug_file = fopen(logfile, "w");
        if (!g_debug_file) {
            return -1;
        }
    }

    g_debug_initialized = true;
    return 0;
}

/**
 * rosetta_debug_cleanup - Cleanup debug subsystem
 */
void rosetta_debug_cleanup(void)
{
    if (g_debug_file) {
        fclose(g_debug_file);
        g_debug_file = NULL;
    }
    g_debug_initialized = false;
}

/**
 * rosetta_debug_set_level - Set debug level
 * @level: New debug level
 */
void rosetta_debug_set_level(int level)
{
    g_debug_level = level;
}

/**
 * rosetta_debug_get_level - Get current debug level
 * Returns: Current debug level
 */
int rosetta_debug_get_level(void)
{
    return g_debug_level;
}

/**
 * rosetta_debug_print - Print formatted debug message
 * @level: Message level
 * @format: Format string
 * @...: Arguments
 */
void rosetta_debug_print(int level, const char *format, ...)
{
    va_list args;
    FILE *out = NULL;
    const char *level_str;

    if (!g_debug_initialized || !(level & g_debug_level)) {
        return;
    }

    /* Determine output destination */
    if (g_debug_output & ROS_DEBUG_OUT_STDERR) {
        out = stderr;
    } else if (g_debug_output & ROS_DEBUG_OUT_STDOUT) {
        out = stdout;
    } else if (g_debug_output & ROS_DEBUG_OUT_FILE) {
        out = g_debug_file;
    }

    if (!out) {
        return;
    }

    /* Get level string */
    switch (level) {
        case ROS_DEBUG_ERROR: level_str = "ERROR"; break;
        case ROS_DEBUG_WARN:  level_str = "WARN "; break;
        case ROS_DEBUG_INFO:  level_str = "INFO "; break;
        case ROS_DEBUG_TRACE: level_str = "TRACE"; break;
        case ROS_DEBUG_DUMP:  level_str = "DUMP "; break;
        default:              level_str = "?????"; break;
    }

    /* Print timestamp */
    fprintf(out, "[%s] ", level_str);

    /* Print message */
    va_start(args, format);
    vfprintf(out, format, args);
    va_end(args);

    fprintf(out, "\n");
    fflush(out);
}

/**
 * rosetta_debug_error - Print error message
 * @format: Format string
 * @...: Arguments
 */
void rosetta_debug_error(const char *format, ...)
{
    va_list args;

    if (!g_debug_initialized || !(g_debug_level & ROS_DEBUG_ERROR)) {
        return;
    }

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/**
 * rosetta_debug_warn - Print warning message
 * @format: Format string
 * @...: Arguments
 */
void rosetta_debug_warn(const char *format, ...)
{
    va_list args;

    if (!g_debug_initialized || !(g_debug_level & ROS_DEBUG_WARN)) {
        return;
    }

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

/**
 * rosetta_debug_info - Print info message
 * @format: Format string
 * @...: Arguments
 */
void rosetta_debug_info(const char *format, ...)
{
    va_list args;

    if (!g_debug_initialized || !(g_debug_level & ROS_DEBUG_INFO)) {
        return;
    }

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

/**
 * rosetta_debug_trace - Print trace message
 * @format: Format string
 * @...: Arguments
 */
void rosetta_debug_trace(const char *format, ...)
{
    va_list args;

    if (!g_debug_initialized || !(g_debug_level & ROS_DEBUG_TRACE)) {
        return;
    }

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

/* ============================================================================
 * Instruction Dumping Functions
 * ============================================================================ */

/**
 * rosetta_debug_dump_arm64_insn - Dump ARM64 instruction encoding
 * @encoding: ARM64 instruction word
 * @pc: Program counter
 */
void rosetta_debug_dump_arm64_insn(uint32_t encoding, uint64_t pc)
{
    printf("ARM64 [0x%016llx]: 0x%08x\n", (unsigned long long)pc, encoding);
}

/**
 * rosetta_debug_dump_x86_insn - Dump x86_64 instruction bytes
 * @code: x86_64 code pointer
 * @size: Number of bytes
 * @pc: Program counter
 */
void rosetta_debug_dump_x86_insn(const void *code, size_t size, uint64_t pc)
{
    const uint8_t *bytes = (const uint8_t *)code;
    size_t i;

    printf("x86_64 [0x%016llx]: ", (unsigned long long)pc);
    for (i = 0; i < size && i < 16; i++) {
        printf("%02x ", bytes[i]);
    }
    if (size > 16) {
        printf("... (%zu bytes)", size);
    }
    printf("\n");
}

/**
 * rosetta_debug_dump_insn_translation - Dump instruction translation
 * @arm64_encoding: ARM64 instruction
 * @x86_code: x86_64 code
 * @x86_size: x86_64 code size
 * @pc: Guest PC
 */
void rosetta_debug_dump_insn_translation(uint32_t arm64_encoding,
                                          const void *x86_code,
                                          size_t x86_size,
                                          uint64_t pc)
{
    printf("TRANSLATION [0x%016llx]: ARM64 0x%08x -> x86 ",
           (unsigned long long)pc, arm64_encoding);

    const uint8_t *bytes = (const uint8_t *)x86_code;
    for (size_t i = 0; i < x86_size && i < 8; i++) {
        printf("%02x ", bytes[i]);
    }
    if (x86_size > 8) {
        printf("... ");
    }
    printf("(%zu bytes)\n", x86_size);
}

/* ============================================================================
 * Block Dumping Functions
 * ============================================================================ */

/**
 * rosetta_debug_dump_block - Dump translated block
 * @guest_pc: Guest PC
 * @host_code: Host code
 * @size: Block size
 * @insn_count: Instruction count
 */
void rosetta_debug_dump_block(uint64_t guest_pc, void *host_code,
                               size_t size, int insn_count)
{
    printf("=== BLOCK TRANSLATION ===\n");
    printf("Guest PC:   0x%016llx\n", (unsigned long long)guest_pc);
    printf("Host PC:    0x%016llx\n", (unsigned long long)(uintptr_t)host_code);
    printf("Block size: %zu bytes\n", size);
    printf("Instructions: %d\n", insn_count);
    printf("Expansion:  %.2f bytes/insn\n",
           insn_count > 0 ? (double)size / insn_count : 0.0);
    printf("========================\n");
}

/**
 * rosetta_debug_dump_block_header - Dump block header
 * @guest_pc: Guest PC
 * @size: Estimated size
 */
void rosetta_debug_dump_block_header(uint64_t guest_pc, size_t size)
{
    printf("\n--- BLOCK BEGIN [0x%016llx] (est. %zu bytes) ---\n",
           (unsigned long long)guest_pc, size);
}

/**
 * rosetta_debug_dump_block_footer - Dump block footer
 * @actual_size: Actual size
 * @insn_count: Instruction count
 * @cycles: Estimated cycles
 */
void rosetta_debug_dump_block_footer(size_t actual_size, int insn_count, int cycles)
{
    printf("--- BLOCK END (%zu bytes, %d insns",
           actual_size, insn_count);
    if (cycles >= 0) {
        printf(", ~%d cycles", cycles);
    }
    printf(") ---\n");
}

/* ============================================================================
 * Register and State Dumping Functions
 * ============================================================================ */

/**
 * rosetta_debug_dump_guest_state - Dump guest state
 * @state: Thread state
 * @mask: Register mask
 */
void rosetta_debug_dump_guest_state(ThreadState *state, uint64_t mask)
{
    int i;

    printf("=== GUEST STATE ===\n");
    printf("PC: 0x%016llx\n", (unsigned long long)state->guest.pc);
    printf("SP: 0x%016llx\n", (unsigned long long)state->guest.sp);
    printf("\n");

    for (i = 0; i < 32; i++) {
        if (mask == 0 || (mask & (1ULL << i))) {
            printf("X%-2d: 0x%016llx%s", i,
                   (unsigned long long)state->guest.x[i],
                   (i % 4 == 3) ? "\n" : "  ");
        }
    }

    printf("\nFlags: 0x%08llx (N=%d Z=%d C=%d V=%d)\n",
           (unsigned long long)state->guest.pstate,
           (int)((state->guest.pstate >> 31) & 1),
           (int)((state->guest.pstate >> 30) & 1),
           (int)((state->guest.pstate >> 29) & 1),
           (int)((state->guest.pstate >> 28) & 1));
}

/**
 * rosetta_debug_dump_reg - Dump specific register
 * @name: Register name
 * @reg: Register number
 * @value: Register value
 */
void rosetta_debug_dump_reg(const char *name, uint8_t reg, uint64_t value)
{
    printf("%s (X%d): 0x%016llx\n", name, reg, (unsigned long long)value);
}

/**
 * rosetta_debug_dump_flags - Dump NZCV flags
 * @pstate: PSTATE value
 */
void rosetta_debug_dump_flags(uint64_t pstate)
{
    printf("PSTATE: 0x%08llx\n", (unsigned long long)pstate);
    printf("  N (Negative):   %d\n", (int)((pstate >> 31) & 1));
    printf("  Z (Zero):       %d\n", (int)((pstate >> 30) & 1));
    printf("  C (Carry):      %d\n", (int)((pstate >> 29) & 1));
    printf("  V (Overflow):   %d\n", (int)((pstate >> 28) & 1));
}

/**
 * rosetta_debug_dump_memory - Dump memory region
 * @addr: Start address
 * @size: Size in bytes
 * @width: Element width
 */
void rosetta_debug_dump_memory(uint64_t addr, size_t size, int width)
{
    uint8_t *mem = (uint8_t *)(uintptr_t)addr;
    size_t i, j;

    printf("Memory dump at 0x%016llx (%zu bytes, width %d):\n",
           (unsigned long long)addr, size, width);

    for (i = 0; i < size; i += 16) {
        printf("0x%016llx: ", (unsigned long long)(addr + i));

        /* Hex dump */
        for (j = 0; j < 16 && (i + j) < size; j++) {
            printf("%02x ", mem[i + j]);
        }

        /* ASCII dump */
        printf(" |");
        for (j = 0; j < 16 && (i + j) < size; j++) {
            char c = mem[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}

/* ============================================================================
 * Hex Dump Utilities
 * ============================================================================ */

/**
 * rosetta_debug_hexdump - Hex dump with ASCII
 * @data: Data to dump
 * @size: Size of data
 * @base: Base address
 */
void rosetta_debug_hexdump(const void *data, size_t size, uint64_t base)
{
    const uint8_t *bytes = (const uint8_t *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("0x%016llx: ", (unsigned long long)(base + i));

        /* Hex portion */
        for (j = 0; j < 16 && (i + j) < size; j++) {
            if (j == 8) {
                printf(" ");
            }
            printf("%02x ", bytes[i + j]);
        }

        /* Padding for short lines */
        for (j = (i + 16 < size) ? 16 : (size - i); j < 16; j++) {
            printf("   ");
            if (j == 8) {
                printf(" ");
            }
        }

        /* ASCII portion */
        printf(" |");
        for (j = 0; j < 16 && (i + j) < size; j++) {
            char c = bytes[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");
    }
}

/**
 * rosetta_debug_hexdump_line - Hex dump single line
 * @data: Data pointer
 * @offset: Offset within block
 */
void rosetta_debug_hexdump_line(const uint8_t *data, size_t offset)
{
    size_t i;

    printf("0x%04zx: ", offset);

    for (i = 0; i < 16; i++) {
        printf("%02x ", data[offset + i]);
    }

    printf("|");
    for (i = 0; i < 16; i++) {
        char c = data[offset + i];
        printf("%c", (c >= 32 && c < 127) ? c : '.');
    }
    printf("|\n");
}

/* ============================================================================
 * Trace Buffer Functions
 * ============================================================================ */

/**
 * rosetta_trace_init - Initialize trace buffer
 * Returns: 0 on success, -1 on error
 */
int rosetta_trace_init(void)
{
    if (g_trace_initialized) {
        rosetta_trace_cleanup();
    }

    memset(g_trace_buffer, 0, sizeof(g_trace_buffer));
    g_trace_head = 0;
    g_trace_count = 0;
    g_trace_initialized = true;

    return 0;
}

/**
 * rosetta_trace_cleanup - Cleanup trace buffer
 */
void rosetta_trace_cleanup(void)
{
    g_trace_initialized = false;
}

/**
 * rosetta_trace_record - Record trace event
 * @type: Event type
 * @guest_pc: Guest PC
 * @host_pc: Host PC
 * @data: Additional data
 */
void rosetta_trace_record(uint8_t type, uint64_t guest_pc,
                          uint64_t host_pc, uint64_t data)
{
    rosetta_trace_entry_t *entry;

    if (!g_trace_initialized) {
        return;
    }

    entry = &g_trace_buffer[g_trace_head];

    /* Get timestamp */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    entry->timestamp = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

    entry->guest_pc = guest_pc;
    entry->host_pc = host_pc;
    entry->arm64_insn = (uint32_t)data;
    entry->event_type = type;
    entry->flags = 0;

    g_trace_head = (g_trace_head + 1) % ROS_TRACE_BUFFER_SIZE;
    if (g_trace_count < ROS_TRACE_BUFFER_SIZE) {
        g_trace_count++;
    }
}

/**
 * rosetta_trace_flush - Flush trace buffer to output
 */
void rosetta_trace_flush(void)
{
    /* Trace is kept in memory; flush would write to file */
    /* Implementation depends on output configuration */
}

/**
 * rosetta_trace_dump - Dump trace buffer contents
 */
void rosetta_trace_dump(void)
{
    uint32_t i, count;
    uint32_t start;

    if (!g_trace_initialized || g_trace_count == 0) {
        printf("Trace buffer is empty\n");
        return;
    }

    printf("=== TRACE BUFFER (%u entries) ===\n", g_trace_count);

    /* Determine start position */
    if (g_trace_count < ROS_TRACE_BUFFER_SIZE) {
        start = 0;
        count = g_trace_count;
    } else {
        start = g_trace_head;
        count = ROS_TRACE_BUFFER_SIZE;
    }

    for (i = 0; i < count; i++) {
        uint32_t idx = (start + i) % ROS_TRACE_BUFFER_SIZE;
        rosetta_trace_entry_t *entry = &g_trace_buffer[idx];

        const char *type_str;
        switch (entry->event_type) {
            case ROS_TRACE_BLOCK_ENTRY: type_str = "BLOCK+"; break;
            case ROS_TRACE_BLOCK_EXIT:  type_str = "BLOCK-"; break;
            case ROS_TRACE_INSN:        type_str = "INSN  "; break;
            case ROS_TRACE_SYSCALL:     type_str = "SYSCALL"; break;
            case ROS_TRACE_EXCEPTION:   type_str = "EXCEPT "; break;
            case ROS_TRACE_INTERRUPT:   type_str = "INT    "; break;
            default:                    type_str = "UNKNOWN"; break;
        }

        printf("[%010llu] %s guest=0x%016llx host=0x%016llx insn=0x%08x\n",
               (unsigned long long)entry->timestamp,
               type_str,
               (unsigned long long)entry->guest_pc,
               (unsigned long long)entry->host_pc,
               entry->arm64_insn);
    }

    printf("================================\n");
}

/* ============================================================================
 * Disassembly Support (Placeholder)
 * ============================================================================ */

/**
 * rosetta_debug_disasm_arm64 - Disassemble ARM64 instruction
 * @encoding: ARM64 instruction
 * @pc: Program counter
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Bytes written
 */
int rosetta_debug_disasm_arm64(uint32_t encoding, uint64_t pc,
                                char *buf, size_t buf_size)
{
    (void)pc;  /* Reserved for future use */

    /* Simple placeholder - just show encoding */
    return snprintf(buf, buf_size, ".word 0x%08x", encoding);
}

/**
 * rosetta_debug_disasm_x86 - Disassemble x86_64 instruction
 * @code: x86_64 code
 * @pc: Program counter
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Bytes consumed
 */
int rosetta_debug_disasm_x86(const void *code, uint64_t pc,
                              char *buf, size_t buf_size)
{
    const uint8_t *bytes = (const uint8_t *)code;
    (void)pc;  /* Reserved for future use */

    /* Simple placeholder - show bytes */
    return snprintf(buf, buf_size, ".byte 0x%02x, 0x%02x, 0x%02x, 0x%02x",
                    bytes[0], bytes[1], bytes[2], bytes[3]);
}
