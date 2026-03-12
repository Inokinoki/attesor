/* ============================================================================
 * Rosetta Execution Context for Translated Code
 * ============================================================================
 *
 * This defines the context structure passed to translated ARM64 code
 * via register X18 (platform-specific register per ARM64 ABI)
 * ============================================================================ */

#ifndef ROSETTA_EXEC_CONTEXT_H
#define ROSETTA_EXEC_CONTEXT_H

#include "rosetta_types.h"
#include "rosetta_memmgr.h"
#include <stdint.h>

/* ============================================================================
 * Execution Context Structure
 * ============================================================================ */

/**
 * Context passed to translated ARM64 code via X18 register
 * This structure provides all the information needed for translated code
 * to access guest memory and registers
 */
typedef struct {
    /* Memory management */
    void *guest_mem_base;           /* Base of guest memory in host address space */
    uint64_t guest_mem_size;        /* Size of guest memory */

    /* Guest register state */
    RosettaThreadState *state;      /* Full thread state (includes guest registers) */

    /* Reserved for future use */
    uint64_t reserved[8];

} rosetta_exec_context_t;

/* ============================================================================
 * Memory Access Helper Functions
 * ============================================================================ */

/**
 * Read from guest memory
 * These functions are called by translated code to access guest memory
 * @param ctx Execution context (passed in X18)
 * @param guest_addr Guest virtual address
 * @return Value read from guest memory
 */

/* Function declarations (implementations in rosetta_exec_helpers.c) */
uint8_t rosetta_mem_read8(rosetta_exec_context_t *ctx, uint64_t guest_addr);
uint16_t rosetta_mem_read16(rosetta_exec_context_t *ctx, uint64_t guest_addr);
uint32_t rosetta_mem_read32(rosetta_exec_context_t *ctx, uint64_t guest_addr);
uint64_t rosetta_mem_read64(rosetta_exec_context_t *ctx, uint64_t guest_addr);

/**
 * Write to guest memory
 * @param ctx Execution context (passed in X18)
 * @param guest_addr Guest virtual address
 * @param value Value to write
 */

void rosetta_mem_write8(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint8_t value);
void rosetta_mem_write16(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint16_t value);
void rosetta_mem_write32(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint32_t value);
void rosetta_mem_write64(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint64_t value);

/* ============================================================================
 * Guest Register Access
 * ============================================================================ */

/**
 * Get guest register value
 * @param ctx Execution context (passed in X18)
 * @param reg_index x86_64 register index (0=RAX, 1=RCX, etc.)
 * @return Register value
 */
static inline uint64_t rosetta_get_greg(rosetta_exec_context_t *ctx, int reg_index)
{
    if (reg_index < 0 || reg_index >= 16) {
        return 0;
    }
    return ctx->state->guest.r[reg_index];
}

/**
 * Set guest register value
 * @param ctx Execution context (passed in X18)
 * @param reg_index x86_64 register index (0=RAX, 1=RCX, etc.)
 * @param value Value to set
 */
static inline void rosetta_set_greg(rosetta_exec_context_t *ctx, int reg_index, uint64_t value)
{
    if (reg_index >= 0 && reg_index < 16) {
        ctx->state->guest.r[reg_index] = value;
    }
}

#endif /* ROSETTA_EXEC_CONTEXT_H */
