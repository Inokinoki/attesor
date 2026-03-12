/* ============================================================================
 * Rosetta Memory Access Helpers for Translated Code
 * ============================================================================
 *
 * These functions are called by translated ARM64 code to access guest memory.
 * They translate guest addresses to host addresses and perform the access.
 * ============================================================================ */

#include "rosetta_exec_helpers.h"
#include "rosetta_exec_context.h"
#include <stdint.h>
#include <stdio.h>

/* ============================================================================
 * Memory Access Functions
 * ============================================================================ */

/**
 * Read 64-bit value from guest memory
 * @param ctx Execution context (passed in X18)
 * @param guest_addr Guest virtual address
 * @return Value read from guest memory
 */
uint64_t rosetta_mem_read64(rosetta_exec_context_t *ctx, uint64_t guest_addr)
{
    fprintf(stderr, "[MEM_READ] read64: guest_addr=0x%lx host_base=%p\n",
            guest_addr, ctx->guest_mem_base);

    if (guest_addr + 7 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_READ] ERROR: Address out of bounds\n");
        return 0;
    }

    uint64_t *host_addr = (uint64_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    uint64_t value = *host_addr;

    fprintf(stderr, "[MEM_READ] read64: *%p = 0x%lx\n", host_addr, value);
    return value;
}

/**
 * Write 64-bit value to guest memory
 * @param ctx Execution context (passed in X18)
 * @param guest_addr Guest virtual address
 * @param value Value to write
 */
void rosetta_mem_write64(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint64_t value)
{
    fprintf(stderr, "[MEM_WRITE] write64: guest_addr=0x%lx value=0x%lx\n",
            guest_addr, value);

    if (guest_addr + 7 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_WRITE] ERROR: Address out of bounds\n");
        return;
    }

    uint64_t *host_addr = (uint64_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    *host_addr = value;

    fprintf(stderr, "[MEM_WRITE] write64: *%p = 0x%lx\n", host_addr, value);
}

/**
 * Read 32-bit value from guest memory
 */
uint32_t rosetta_mem_read32(rosetta_exec_context_t *ctx, uint64_t guest_addr)
{
    fprintf(stderr, "[MEM_READ] read32: guest_addr=0x%lx\n", guest_addr);

    if (guest_addr + 3 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_READ] ERROR: Address out of bounds\n");
        return 0;
    }

    uint32_t *host_addr = (uint32_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    return *host_addr;
}

/**
 * Write 32-bit value to guest memory
 */
void rosetta_mem_write32(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint32_t value)
{
    fprintf(stderr, "[MEM_WRITE] write32: guest_addr=0x%lx value=0x%x\n",
            guest_addr, value);

    if (guest_addr + 3 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_WRITE] ERROR: Address out of bounds\n");
        return;
    }

    uint32_t *host_addr = (uint32_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    *host_addr = value;
}

/**
 * Read 16-bit value from guest memory
 */
uint16_t rosetta_mem_read16(rosetta_exec_context_t *ctx, uint64_t guest_addr)
{
    fprintf(stderr, "[MEM_READ] read16: guest_addr=0x%lx\n", guest_addr);

    if (guest_addr + 1 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_READ] ERROR: Address out of bounds\n");
        return 0;
    }

    uint16_t *host_addr = (uint16_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    return *host_addr;
}

/**
 * Write 16-bit value to guest memory
 */
void rosetta_mem_write16(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint16_t value)
{
    fprintf(stderr, "[MEM_WRITE] write16: guest_addr=0x%lx value=0x%x\n",
            guest_addr, value);

    if (guest_addr + 1 >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_WRITE] ERROR: Address out of bounds\n");
        return;
    }

    uint16_t *host_addr = (uint16_t *)((uint8_t *)ctx->guest_mem_base + guest_addr);
    *host_addr = value;
}

/**
 * Read 8-bit value from guest memory
 */
uint8_t rosetta_mem_read8(rosetta_exec_context_t *ctx, uint64_t guest_addr)
{
    fprintf(stderr, "[MEM_READ] read8: guest_addr=0x%lx\n", guest_addr);

    if (guest_addr >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_READ] ERROR: Address out of bounds\n");
        return 0;
    }

    uint8_t *host_addr = (uint8_t *)ctx->guest_mem_base + guest_addr;
    return *host_addr;
}

/**
 * Write 8-bit value to guest memory
 */
void rosetta_mem_write8(rosetta_exec_context_t *ctx, uint64_t guest_addr, uint8_t value)
{
    fprintf(stderr, "[MEM_WRITE] write8: guest_addr=0x%lx value=0x%x\n",
            guest_addr, value);

    if (guest_addr >= ctx->guest_mem_size) {
        fprintf(stderr, "[MEM_WRITE] ERROR: Address out of bounds\n");
        return;
    }

    uint8_t *host_addr = (uint8_t *)ctx->guest_mem_base + guest_addr;
    *host_addr = value;
}
