/* ============================================================================
 * Rosetta Execution Engine
 * ============================================================================
 *
 * Main execution engine for running x86_64 binaries on ARM64
 * ============================================================================
 */

#ifndef ROSETTA_EXECUTE_H
#define ROSETTA_EXECUTE_H

#include "rosetta_types.h"
#include "rosetta_memmgr.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Instruction Fetching
 * ============================================================================ */

/**
 * Fetch a single byte from guest memory
 * @param memmgr Memory manager
 * @param guest_addr Guest virtual address
 * @return Byte value, or -1 on error
 */
int8_t rosetta_fetch_byte(rosetta_memmgr_t *memmgr, uint64_t guest_addr);

/**
 * Fetch x86_64 instruction from guest memory
 * @param memmgr Memory manager
 * @param guest_addr Guest virtual address (instruction pointer)
 * @param insn_buf Buffer to store instruction bytes
 * @param max_len Maximum bytes to fetch (15 = max x86_64 insn length)
 * @return Number of bytes fetched, or -1 on error
 */
ssize_t rosetta_fetch_insn(rosetta_memmgr_t *memmgr,
                           uint64_t guest_addr,
                           uint8_t *insn_buf,
                           size_t max_len);

/**
 * Get x86_64 instruction length
 * @param insn_buf Instruction bytes
 * @param buf_len Number of bytes available
 * @return Instruction length (1-15), or -1 on error
 */
ssize_t rosetta_insn_len(const uint8_t *insn_buf, size_t buf_len);

/* ============================================================================
 * Execution Context
 * ============================================================================ */

/**
 * Execution context for running x86_64 code
 */
typedef struct {
    RosettaThreadState *state;      /* Thread state */
    rosetta_memmgr_t *memmgr;         /* Memory manager */
    uint64_t guest_pc;                /* Guest program counter */
    uint64_t blocks_executed;         /* Number of blocks executed */
    uint64_t instructions_executed;   /* Number of instructions executed */
    int is_running;                   /* Execution flag */
    int exit_code;                    /* Program exit code */
} rosetta_exec_ctx_t;

/**
 * Create execution context
 * @param state Thread state
 * @param memmgr Memory manager
 * @return Execution context, or NULL on error
 */
rosetta_exec_ctx_t *rosetta_exec_create(RosettaThreadState *state,
                                         rosetta_memmgr_t *memmgr);

/**
 * Destroy execution context
 * @param ctx Execution context
 */
void rosetta_exec_destroy(rosetta_exec_ctx_t *ctx);

/* ============================================================================
 * Main Execution Loop
 * ============================================================================ */

/**
 * Execute x86_64 code until termination
 * @param ctx Execution context
 * @param entry_point Guest entry point address
 * @return Exit code
 */
int rosetta_execute(rosetta_exec_ctx_t *ctx, uint64_t entry_point);

/**
 * Execute single instruction
 * @param ctx Execution context
 * @return 0 on success, -1 on error, 1 on exit
 */
int rosetta_execute_single(rosetta_exec_ctx_t *ctx);

/**
 * Stop execution
 * @param ctx Execution context
 * @return 0 on success
 */
int rosetta_execute_stop(rosetta_exec_ctx_t *ctx);

/* ============================================================================
 * Block Translation and Execution
 * ============================================================================ */

/**
 * Translate and execute a basic block
 * @param ctx Execution context
 * @param block_start Start address of block
 * @return Number of instructions executed, or -1 on error
 */
int rosetta_execute_block(rosetta_exec_ctx_t *ctx, uint64_t block_start);

#endif /* ROSETTA_EXECUTE_H */
