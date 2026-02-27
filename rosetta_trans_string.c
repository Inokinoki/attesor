/* ============================================================================
 * Rosetta Translator - String Operations Implementation
 * ============================================================================ */

#include "rosetta_trans_string.h"

static void *memory_translate_addr(uint64_t guest_addr);

static void *memory_translate_addr(uint64_t guest_addr)
{
    return (void *)(uintptr_t)guest_addr;
}

int translate_movs(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx)
{
    uint64_t rsi = state->guest.x[6];  /* RSI */
    uint64_t rdi = state->guest.x[7];  /* RDI */
    uint32_t count = rep ? ecx : 1;
    int df = (state->guest.pstate >> 27) & 1;  /* Direction flag (stub) */
    int step = df ? -size : size;

    void *src_addr = memory_translate_addr(rsi);
    void *dst_addr = memory_translate_addr(rdi);

    if (src_addr == NULL || dst_addr == NULL) {
        return -1;
    }

    for (uint32_t i = 0; i < count; i++) {
        uint64_t val = 0;
        switch (size) {
            case 1: val = *(uint8_t *)src_addr; break;
            case 2: val = *(uint16_t *)src_addr; break;
            case 4: val = *(uint32_t *)src_addr; break;
            case 8: val = *(uint64_t *)src_addr; break;
        }
        switch (size) {
            case 1: *(uint8_t *)dst_addr = (uint8_t)val; break;
            case 2: *(uint16_t *)dst_addr = (uint16_t)val; break;
            case 4: *(uint32_t *)dst_addr = (uint32_t)val; break;
            case 8: *(uint64_t *)dst_addr = val; break;
        }
        src_addr = (uint8_t *)src_addr + step;
        dst_addr = (uint8_t *)dst_addr + step;
    }

    state->guest.x[6] = (uint64_t)src_addr;
    state->guest.x[7] = (uint64_t)dst_addr;

    if (rep) {
        state->guest.x[1] = 0;  /* RCX = 0 */
    }

    return 0;
}

int translate_stos(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx)
{
    uint64_t rdi = state->guest.x[7];  /* RDI */
    uint64_t rax = state->guest.x[0];  /* RAX */
    uint32_t count = rep ? ecx : 1;
    int df = (state->guest.pstate >> 27) & 1;
    int step = df ? -size : size;

    void *dst_addr = memory_translate_addr(rdi);
    if (dst_addr == NULL) {
        return -1;
    }

    for (uint32_t i = 0; i < count; i++) {
        switch (size) {
            case 1: *(uint8_t *)dst_addr = (uint8_t)rax; break;
            case 2: *(uint16_t *)dst_addr = (uint16_t)rax; break;
            case 4: *(uint32_t *)dst_addr = (uint32_t)rax; break;
            case 8: *(uint64_t *)dst_addr = rax; break;
        }
        dst_addr = (uint8_t *)dst_addr + step;
    }

    state->guest.x[7] = (uint64_t)dst_addr;

    if (rep) {
        state->guest.x[1] = 0;
    }

    return 0;
}

int translate_lods(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx)
{
    uint64_t rsi = state->guest.x[6];  /* RSI */
    uint32_t count = rep ? ecx : 1;
    int df = (state->guest.pstate >> 27) & 1;
    int step = df ? -size : size;

    void *src_addr = memory_translate_addr(rsi);
    if (src_addr == NULL) {
        return -1;
    }

    uint64_t val = 0;
    for (uint32_t i = 0; i < count; i++) {
        switch (size) {
            case 1: val = *(uint8_t *)src_addr; break;
            case 2: val = *(uint16_t *)src_addr; break;
            case 4: val = *(uint32_t *)src_addr; break;
            case 8: val = *(uint64_t *)src_addr; break;
        }
        src_addr = (uint8_t *)src_addr + step;
    }

    state->guest.x[0] = val;  /* RAX */
    state->guest.x[6] = (uint64_t)src_addr;

    if (rep) {
        state->guest.x[1] = 0;
    }

    return 0;
}

int translate_cmps(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx)
{
    uint64_t rsi = state->guest.x[6];
    uint64_t rdi = state->guest.x[7];
    uint32_t count = rep ? ecx : 1;
    int df = (state->guest.pstate >> 27) & 1;
    int step = df ? -size : size;

    void *src_addr = memory_translate_addr(rsi);
    void *dst_addr = memory_translate_addr(rdi);

    if (src_addr == NULL || dst_addr == NULL) {
        return -1;
    }

    uint64_t result = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint64_t a = 0, b = 0;
        switch (size) {
            case 1: a = *(uint8_t *)src_addr; b = *(uint8_t *)dst_addr; break;
            case 2: a = *(uint16_t *)src_addr; b = *(uint16_t *)dst_addr; break;
            case 4: a = *(uint32_t *)src_addr; b = *(uint32_t *)dst_addr; break;
            case 8: a = *(uint64_t *)src_addr; b = *(uint64_t *)dst_addr; break;
        }
        result = a - b;
        if (rep && result != 0) break;
        src_addr = (uint8_t *)src_addr + step;
        dst_addr = (uint8_t *)dst_addr + step;
    }

    /* Update flags */
    uint64_t nzcv = 0;
    if (result & (1ULL << 63)) nzcv |= (1ULL << 31);
    if (result == 0) nzcv |= (1ULL << 30);

    state->guest.pstate = nzcv;
    state->guest.x[6] = (uint64_t)src_addr;
    state->guest.x[7] = (uint64_t)dst_addr;

    return 0;
}

int translate_scas(ThreadState *state, const uint8_t *insn, int size, bool rep, uint32_t ecx)
{
    uint64_t rdi = state->guest.x[7];
    uint64_t rax = state->guest.x[0];
    uint32_t count = rep ? ecx : 1;
    int df = (state->guest.pstate >> 27) & 1;
    int step = df ? -size : size;

    void *dst_addr = memory_translate_addr(rdi);
    if (dst_addr == NULL) {
        return -1;
    }

    uint64_t result = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint64_t val = 0;
        switch (size) {
            case 1: val = *(uint8_t *)dst_addr; break;
            case 2: val = *(uint16_t *)dst_addr; break;
            case 4: val = *(uint32_t *)dst_addr; break;
            case 8: val = *(uint64_t *)dst_addr; break;
        }
        result = rax - val;
        if (rep && result == 0) break;
        dst_addr = (uint8_t *)dst_addr + step;
    }

    uint64_t nzcv = 0;
    if (result & (1ULL << 63)) nzcv |= (1ULL << 31);
    if (result == 0) nzcv |= (1ULL << 30);

    state->guest.pstate = nzcv;
    state->guest.x[7] = (uint64_t)dst_addr;

    return 0;
}
