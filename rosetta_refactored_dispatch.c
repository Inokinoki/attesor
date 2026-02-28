/* ============================================================================
 * Rosetta Refactored - Instruction Dispatch Implementation
 * ============================================================================
 *
 * This module implements instruction dispatch functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_dispatch.h"
#include "rosetta_refactored_stats.h"
#include "rosetta_translate_alu_main.h"
#include "rosetta_translate_mem_main.h"
#include "rosetta_translate_branch_main.h"
#include "rosetta_translate_mov.h"
#include "rosetta_translate_compare.h"
#include "rosetta_translate_system.h"
#include "rosetta_translate_conditional.h"
#include "rosetta_translate_bitfield.h"
#include "rosetta_emit_x86.h"

/* Include new functional modules (NEON, FP, Atomic) */
#include "rosetta_refactored_neon.h"
#include "rosetta_refactored_float.h"
#include "rosetta_refactored_atomic.h"

/* ============================================================================
 * Instruction Dispatch
 * ============================================================================ */

/**
 * rosetta_decode_and_dispatch - Decode ARM64 instruction and dispatch to handler
 */
int rosetta_decode_and_dispatch(uint32_t encoding, code_buf_t *code_buf,
                                 ThreadState *state, uint64_t pc, int *terminated)
{
    /* Try each translation module in order of frequency */

    /* 1. ALU instructions (most common) */
    if (translate_alu_dispatch(encoding, code_buf, state->guest.x, &state->guest.pstate) == 0) {
        rosetta_stats_record_alu("unknown");
        return 0;
    }

    /* 2. Compare instructions */
    if (translate_compare_dispatch(encoding, code_buf, state->guest.x,
                                   &state->guest.pstate) == 0) {
        rosetta_stats_record_alu("compare");
        return 0;
    }

    /* 3. MOV instructions */
    if (translate_mov_dispatch(encoding, code_buf, state->guest.x) == 0) {
        rosetta_stats_record_alu("mov");
        return 0;
    }

    /* 4. Conditional instructions (CSEL, CSET, etc.) */
    if (translate_cond_dispatch(encoding, code_buf, state->guest.x,
                                (uint32_t *)&state->guest.pstate) == 0) {
        rosetta_stats_record_alu("conditional");
        return 0;
    }

    /* 4b. Bitfield instructions (BFI, UBFX, SBFX, etc.) */
    if (translate_bitfield_dispatch(encoding, code_buf, state->guest.x) == 0) {
        rosetta_stats_record_alu("bitfield");
        return 0;
    }

    /* 5. Memory instructions */
    if (translate_mem_dispatch(encoding, code_buf, state->guest.x) == 0) {
        rosetta_stats_record_mem("unknown");
        return 0;
    }

    /* 6. Branch instructions */
    if (translate_branch_dispatch(encoding, code_buf, state->guest.x,
                                  pc, terminated) == 0) {
        rosetta_stats_record_branch("unknown");
        return 0;
    }

    /* 7. System instructions */
    if (translate_system_dispatch(encoding, code_buf, state->guest.x) == 0) {
        rosetta_stats_record_insn("SYSTEM");
        *terminated = 1;
        return 0;
    }

    /* 8. NEON/SIMD instructions */
    if (translate_neon_dispatch(encoding, code_buf, (Vector128 *)state->guest.v,
                                state->guest.x) == 0) {
        rosetta_stats_record_insn("NEON");
        return 0;
    }

    /* 9. Floating-Point instructions */
    if (translate_fp_dispatch(encoding, code_buf, (Vector128 *)state->guest.v,
                              state->guest.x, (uint32_t *)&state->guest.pstate) == 0) {
        rosetta_stats_record_insn("FP");
        return 0;
    }

    /* 10. Atomic/Barrier instructions
     * Note: Atomic operations are handled by translate_system_dispatch() above.
     * The refactored atomic module uses a different signature and is not used here.
     */
    /* translate_atomic_dispatch is not used - atomics handled by system dispatch */

    /* Unknown/unimplemented instruction */
    return -1;
}

/**
 * rosetta_translate_instruction - Translate single instruction
 */
int rosetta_translate_instruction(uint32_t encoding, code_buf_t *code_buf,
                                   ThreadState *state, uint64_t pc)
{
    int terminated = 0;
    return rosetta_decode_and_dispatch(encoding, code_buf, state, pc, &terminated);
}

/**
 * rosetta_dispatch_alu - Dispatch ALU instruction
 */
int rosetta_dispatch_alu(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs, uint64_t *pstate)
{
    return translate_alu_dispatch(encoding, code_buf, regs, pstate);
}

/**
 * rosetta_dispatch_memory - Dispatch memory instruction
 */
int rosetta_dispatch_memory(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs)
{
    return translate_mem_dispatch(encoding, code_buf, regs);
}

/**
 * rosetta_dispatch_branch - Dispatch branch instruction
 */
int rosetta_dispatch_branch(uint32_t encoding, code_buf_t *code_buf,
                             uint64_t *regs, uint64_t pc, int *terminated)
{
    return translate_branch_dispatch(encoding, code_buf, regs, pc, terminated);
}

/**
 * rosetta_dispatch_system - Dispatch system instruction
 */
int rosetta_dispatch_system(uint32_t encoding, code_buf_t *code_buf, uint64_t *regs)
{
    return translate_system_dispatch(encoding, code_buf, regs);
}

/* ============================================================================
 * Module Registration
 * ============================================================================ */

/* Registered modules */
static rosetta_module_t g_modules[] = {
    {
        .name = "ALU",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_alu_dispatch
    },
    {
        .name = "Compare",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_compare_dispatch
    },
    {
        .name = "MOV",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_mov_dispatch
    },
    {
        .name = "Conditional",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_cond_dispatch
    },
    {
        .name = "Bitfield",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_bitfield_dispatch
    },
    {
        .name = "Memory",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_mem_dispatch
    },
    {
        .name = "Branch",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_branch_dispatch
    },
    {
        .name = "System",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_system_dispatch
    },
    {
        .name = "NEON",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_neon_dispatch
    },
    {
        .name = "FP",
        .init = NULL,
        .cleanup = NULL,
        .dispatch = (void *)translate_fp_dispatch
    },
    /* Atomic module removed - atomics handled by translate_system_dispatch() */
    { NULL, NULL, NULL, NULL }  /* Sentinel */
};

/**
 * rosetta_register_module - Register a translation module
 */
int rosetta_register_module(rosetta_module_t *module)
{
    (void)module;  /* Placeholder for future implementation */
    return 0;
}

/**
 * rosetta_init_all_modules - Initialize all registered modules
 */
int rosetta_init_all_modules(void)
{
    rosetta_module_t *mod = g_modules;

    while (mod->name != NULL) {
        if (mod->init != NULL) {
            if (mod->init() != 0) {
                return -1;
            }
        }
        mod++;
    }

    return 0;
}

/**
 * rosetta_cleanup_all_modules - Cleanup all registered modules
 */
void rosetta_cleanup_all_modules(void)
{
    rosetta_module_t *mod = g_modules;

    while (mod->name != NULL) {
        if (mod->cleanup != NULL) {
            mod->cleanup();
        }
        mod++;
    }
}
