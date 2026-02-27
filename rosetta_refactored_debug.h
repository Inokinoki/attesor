/* ============================================================================
 * Rosetta Refactored - Debug Support Header
 * ============================================================================
 *
 * This module provides debug support functions for the Rosetta
 * translation layer, including instruction dumping, block analysis,
 * and trace functionality.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_DEBUG_H
#define ROSETTA_REFACTORED_DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include "rosetta_types.h"

/* ============================================================================
 * Debug Configuration
 * ============================================================================ */

/* Debug level flags */
#define ROS_DEBUG_NONE      0x00
#define ROS_DEBUG_ERROR     0x01
#define ROS_DEBUG_WARN      0x02
#define ROS_DEBUG_INFO      0x04
#define ROS_DEBUG_TRACE     0x08
#define ROS_DEBUG_DUMP      0x10
#define ROS_DEBUG_VERBOSE   0x20

/* Debug output flags */
#define ROS_DEBUG_OUT_STDOUT    0x01
#define ROS_DEBUG_OUT_STDERR    0x02
#define ROS_DEBUG_OUT_FILE      0x04
#define ROS_DEBUG_OUT_SYSLOG    0x08

/* ============================================================================
 * Debug Output Functions
 * ============================================================================ */

/**
 * Initialize debug subsystem
 * @level: Debug level (combination of ROS_DEBUG_* flags)
 * @output: Output destination (combination of ROS_DEBUG_OUT_* flags)
 * @logfile: Path to log file (if ROS_DEBUG_OUT_FILE is set)
 * Returns: 0 on success, -1 on error
 */
int rosetta_debug_init(int level, int output, const char *logfile);

/**
 * Cleanup debug subsystem
 */
void rosetta_debug_cleanup(void);

/**
 * Set debug level
 * @level: New debug level
 */
void rosetta_debug_set_level(int level);

/**
 * Get current debug level
 * Returns: Current debug level flags
 */
int rosetta_debug_get_level(void);

/**
 * Print formatted debug message
 * @level: Message level
 * @format: Printf-style format string
 * @...: Format arguments
 */
void rosetta_debug_print(int level, const char *format, ...);

/**
 * Print error message
 * @format: Printf-style format string
 * @...: Format arguments
 */
void rosetta_debug_error(const char *format, ...);

/**
 * Print warning message
 * @format: Printf-style format string
 * @...: Format arguments
 */
void rosetta_debug_warn(const char *format, ...);

/**
 * Print info message
 * @format: Printf-style format string
 * @...: Format arguments
 */
void rosetta_debug_info(const char *format, ...);

/**
 * Print trace message
 * @format: Printf-style format string
 * @...: Format arguments
 */
void rosetta_debug_trace(const char *format, ...);

/* ============================================================================
 * Instruction Dumping
 * ============================================================================ */

/**
 * Dump ARM64 instruction encoding
 * @encoding: ARM64 instruction word
 * @pc: Program counter address
 */
void rosetta_debug_dump_arm64_insn(uint32_t encoding, uint64_t pc);

/**
 * Dump x86_64 instruction bytes
 * @code: Pointer to x86_64 code
 * @size: Number of bytes to dump
 * @pc: Program counter address
 */
void rosetta_debug_dump_x86_insn(const void *code, size_t size, uint64_t pc);

/**
 * Dump instruction translation mapping
 * @arm64_encoding: ARM64 instruction encoding
 * @x86_code: Translated x86_64 code
 * @x86_size: Size of translated code
 * @pc: Guest PC
 */
void rosetta_debug_dump_insn_translation(uint32_t arm64_encoding,
                                          const void *x86_code,
                                          size_t x86_size,
                                          uint64_t pc);

/* ============================================================================
 * Block Dumping
 * ============================================================================ */

/**
 * Dump translated block information
 * @guest_pc: Guest PC of block
 * @host_code: Host code pointer
 * @size: Block size in bytes
 * @insn_count: Number of instructions in block
 */
void rosetta_debug_dump_block(uint64_t guest_pc, void *host_code,
                               size_t size, int insn_count);

/**
 * Dump block header
 * @guest_pc: Guest PC
 * @size: Estimated block size
 */
void rosetta_debug_dump_block_header(uint64_t guest_pc, size_t size);

/**
 * Dump block footer with statistics
 * @actual_size: Actual block size
 * @insn_count: Number of instructions
 * @cycles: Estimated cycles (if available)
 */
void rosetta_debug_dump_block_footer(size_t actual_size, int insn_count, int cycles);

/* ============================================================================
 * Register and State Dumping
 * ============================================================================ */

/**
 * Dump guest register state
 * @state: Thread state to dump
 * @mask: Register mask (0 = all, bits = specific registers)
 */
void rosetta_debug_dump_guest_state(ThreadState *state, uint64_t mask);

/**
 * Dump specific ARM64 register
 * @reg: Register number (0-30)
 * @value: Register value
 */
void rosetta_debug_dump_reg(const char *name, uint8_t reg, uint64_t value);

/**
 * Dump NZCV flags
 * @pstate: PSTATE value
 */
void rosetta_debug_dump_flags(uint64_t pstate);

/**
 * Dump memory region
 * @addr: Start address
 * @size: Number of bytes to dump
 * @width: Element width (1, 2, 4, or 8 bytes)
 */
void rosetta_debug_dump_memory(uint64_t addr, size_t size, int width);

/* ============================================================================
 * Hex Dump Utilities
 * ============================================================================ */

/**
 * Hex dump with ASCII
 * @data: Data to dump
 * @size: Size of data
 * @base: Base address for display
 */
void rosetta_debug_hexdump(const void *data, size_t size, uint64_t base);

/**
 * Hex dump single line (16 bytes)
 * @data: Data to dump
 * @offset: Offset within block
 */
void rosetta_debug_hexdump_line(const uint8_t *data, size_t offset);

/* ============================================================================
 * Trace Buffer
 * ============================================================================ */

/* Trace buffer entry */
typedef struct {
    uint64_t timestamp;
    uint64_t guest_pc;
    uint64_t host_pc;
    uint32_t arm64_insn;
    uint8_t event_type;
    uint8_t flags;
} rosetta_trace_entry_t;

/* Trace event types */
#define ROS_TRACE_BLOCK_ENTRY   0x01
#define ROS_TRACE_BLOCK_EXIT    0x02
#define ROS_TRACE_INSN          0x03
#define ROS_TRACE_SYSCALL       0x04
#define ROS_TRACE_EXCEPTION     0x05
#define ROS_TRACE_INTERRUPT     0x06

/* Trace buffer configuration */
#define ROS_TRACE_BUFFER_SIZE   4096

/**
 * Initialize trace buffer
 * Returns: 0 on success, -1 on error
 */
int rosetta_trace_init(void);

/**
 * Cleanup trace buffer
 */
void rosetta_trace_cleanup(void);

/**
 * Record trace event
 * @type: Event type
 * @guest_pc: Guest PC
 * @host_pc: Host PC (if applicable)
 * @data: Additional data
 */
void rosetta_trace_record(uint8_t type, uint64_t guest_pc,
                          uint64_t host_pc, uint64_t data);

/**
 * Flush trace buffer to output
 */
void rosetta_trace_flush(void);

/**
 * Dump trace buffer contents
 */
void rosetta_trace_dump(void);

/* ============================================================================
 * Disassembly Support
 * ============================================================================ */

/**
 * Disassemble ARM64 instruction
 * @encoding: ARM64 instruction encoding
 * @pc: Program counter
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Number of bytes written
 */
int rosetta_debug_disasm_arm64(uint32_t encoding, uint64_t pc,
                                char *buf, size_t buf_size);

/**
 * Disassemble x86_64 instruction
 * @code: Pointer to x86_64 code
 * @pc: Program counter
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Number of bytes consumed
 */
int rosetta_debug_disasm_x86(const void *code, uint64_t pc,
                              char *buf, size_t buf_size);

/* ============================================================================
 * Convenience Macros
 * ============================================================================ */

#ifdef DEBUG

#define ROS_DBG(fmt, ...) \
    rosetta_debug_print(ROS_DEBUG_INFO, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ROS_DBG_TRACE(fmt, ...) \
    rosetta_debug_print(ROS_DEBUG_TRACE, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ROS_DBG_DUMP(fmt, ...) \
    rosetta_debug_print(ROS_DEBUG_DUMP, fmt, ##__VA_ARGS__)

#define ROS_DBG_ERR(fmt, ...) \
    rosetta_debug_error("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ROS_DBG_WARN(fmt, ...) \
    rosetta_debug_warn("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#else /* !DEBUG */

#define ROS_DBG(fmt, ...) ((void)0)
#define ROS_DBG_TRACE(fmt, ...) ((void)0)
#define ROS_DBG_DUMP(fmt, ...) ((void)0)
#define ROS_DBG_ERR(fmt, ...) ((void)0)
#define ROS_DBG_WARN(fmt, ...) ((void)0)

#endif /* DEBUG */

#endif /* ROSETTA_REFACTORED_DEBUG_H */
