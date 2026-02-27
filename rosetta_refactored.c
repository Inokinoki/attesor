/* ============================================================================
 * Rosetta Translator - Refactored Implementation (Modular Wrapper)
 * ============================================================================
 *
 * This module serves as the main wrapper that integrates all modular
 * translation components.
 *
 * Translation Flow:
 * 1. ARM64 instructions are decoded using arm64_decode helpers
 * 2. Instruction is dispatched to appropriate translator
 * 3. x86_64 machine code is emitted using codegen
 * 4. Translated code is cached for reuse
 * ============================================================================ */

#include "rosetta_refactored.h"

/* Include modular components */
#include "rosetta_refactored_init.h"
#include "rosetta_refactored_exec.h"
#include "rosetta_refactored_reg.h"
#include "rosetta_refactored_helpers.h"
#include "rosetta_refactored_debug.h"
#include "rosetta_refactored_stats.h"
#include "rosetta_refactored_block.h"
#include "rosetta_refactored_memory.h"
#include "rosetta_refactored_signal.h"
#include "rosetta_refactored_dispatch.h"
#include "rosetta_refactored_syscall.h"
#include "rosetta_refactored_exception.h"
#include "rosetta_trans_cache.h"
#include "rosetta_optimizer.h"
#include "rosetta_info.h"

/* Include translation modules for dispatch */
#include "rosetta_translate_alu_main.h"
#include "rosetta_translate_mem_main.h"
#include "rosetta_translate_branch_main.h"
#include "rosetta_translate_mov.h"
#include "rosetta_translate_compare.h"
#include "rosetta_translate_system.h"
#include "rosetta_emit_x86.h"
#include "rosetta_arm64_decode.h"

/* Include trans_* modules for additional instruction support */
#include "rosetta_trans_mov.h"
#include "rosetta_trans_compare.h"
#include "rosetta_trans_alu.h"
#include "rosetta_trans_mem.h"
#include "rosetta_trans_branch.h"
#include "rosetta_trans_bit.h"
#include "rosetta_trans_special.h"
#include "rosetta_trans_system.h"

/* Include new functional modules (NEON, FP, Atomic) */
#include "rosetta_refactored_neon.h"
#include "rosetta_refactored_float.h"
#include "rosetta_refactored_atomic.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================
 * Compatibility Layer - Backward compatible functions
 * ============================================================================ */

/* From rosetta_refactored_init.c */
extern int rosetta_init(void);
extern void rosetta_cleanup(void);
extern ThreadState *rosetta_get_state(void);
extern bool rosetta_is_initialized(void);

/* From rosetta_refactored_exec.c */
extern void *translate_block(uint64_t guest_pc);
extern void *translate_block_fast(uint64_t guest_pc);
extern void *rosetta_translate(uint64_t guest_pc);
extern void rosetta_execute(void *host_code);
extern void rosetta_run(uint64_t guest_pc);
extern uint64_t rosetta_interpret(uint32_t insn, ThreadState *state, uint64_t pc);
extern void rosetta_run_interpreter(uint64_t guest_pc, int max_insns);

/* From rosetta_refactored_reg.c */
extern uint64_t rosetta_get_reg(uint8_t reg);
extern void rosetta_set_reg(uint8_t reg, uint64_t value);
extern uint64_t rosetta_get_pc(void);
extern void rosetta_set_pc(uint64_t pc);
extern uint64_t rosetta_get_sp(void);
extern void rosetta_set_sp(uint64_t sp);
extern uint64_t rosetta_get_flags(void);
extern void rosetta_set_flags(uint64_t flags);
extern void rosetta_print_state(void);

/* ============================================================================
 * Re-export optimizer functions (backward compatibility)
 * ============================================================================ */

/* From rosetta_optimizer.c */
extern void *translate_block_inline(uint64_t guest_pc);
extern int translate_and_execute(uint64_t guest_pc);
extern void *translate_block_with_stats(uint64_t guest_pc, bool record_stats);
extern void execute_with_stats(void *host_code, int insn_count);

/* ============================================================================
 * Re-export statistics functions (backward compatibility)
 * ============================================================================ */

/**
 * rosetta_get_stats - Get translation statistics
 * @stats: Pointer to stats structure to fill
 */
void rosetta_get_stats(rosetta_stats_t *stats)
{
    if (stats) {
        rosetta_stats_get(stats);
    }
}

/**
 * rosetta_reset_stats - Reset translation statistics
 */
void rosetta_reset_stats(void)
{
    rosetta_stats_reset();
    rosetta_stats_reset_local();
}

/**
 * rosetta_print_stats - Print translation statistics
 */
void rosetta_print_stats(void)
{
    rosetta_stats_print(true);
}

/* ============================================================================
 * Instruction Dispatch (wrapper around rosetta_dispatch module)
 * ============================================================================ */

/**
 * decode_and_dispatch - Decode ARM64 instruction and dispatch to handler
 * @encoding: ARM64 instruction encoding
 * @code_buf: Code buffer for x86_64 emission
 * @state: Guest thread state
 * @pc: Current program counter
 * @terminated: Output flag for block termination
 * Returns: 0 on success, -1 on unknown instruction
 */
int decode_and_dispatch(uint32_t encoding, code_buf_t *code_buf,
                        ThreadState *state, uint64_t pc, int *terminated)
{
    return rosetta_decode_and_dispatch(encoding, code_buf, state, pc, terminated);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/* Note: rosetta_version(), rosetta_capabilities(), and
 * rosetta_get_supported_instructions() are now provided by
 * the rosetta_info module. They are automatically available
 * when linking with librosetta.a.
 */
