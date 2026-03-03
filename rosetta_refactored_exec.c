/* ============================================================================
 * Rosetta Refactored - Execution Implementation
 * ============================================================================
 *
 * This module implements execution functions for running translated code
 * in the Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_refactored_exec.h"
#include "rosetta_refactored.h"
#include "rosetta_refactored_init.h"
#include "rosetta_x86_decode.h"
#include "rosetta_translate_dispatch.h"
#include "rosetta_arm64_emit.h"
#include "rosetta_trans_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External reference to global thread state */
extern ThreadState *rosetta_get_state(void);

/* External reference to translation cache functions */
extern void *refactored_translation_cache_lookup(uint64_t guest_pc);
extern void refactored_translation_cache_insert(uint64_t guest_pc, void *code, uint32_t size);
extern void *refactored_code_cache_alloc(size_t size);

/* Forward declarations for ARM64 emit functions */
extern void emit_nop(code_buffer_t *buf);
extern void emit_ret(code_buffer_t *buf);
extern void emit_b(code_buffer_t *buf, int32_t imm26);

/* ============================================================================
 * Block Chaining Helper
 * ============================================================================ */

/**
 * try_chain_block - Attempt to chain to a known block
 * @code_buf: Code buffer
 * @target_pc: Target guest PC
 * Returns: 1 if chained, 0 if not
 */
__attribute__((unused))
static int try_chain_block(code_buffer_t *code_buf, uint64_t target_pc)
{
    void *cached = refactored_translation_cache_lookup(target_pc);
    if (cached) {
        /* Chain directly to the cached block with ARM64 B instruction */
        int32_t offset = (int32_t)((uintptr_t)cached - (uintptr_t)code_buf->buffer);
        emit_b(code_buf, offset);
        return 1;
    }
    return 0;
}

/* ============================================================================
 * Main Translation Entry Point
 * ============================================================================ */

/**
 * translate_block - Translate a basic block of x86_64 instructions
 * @guest_pc: Starting program counter (x86_64 guest address)
 *
 * Translates x86_64 instructions in the block to ARM64.
 * Uses x86 decoder and dispatch to appropriate translators, emits ARM64 code.
 * Returns: Pointer to translated ARM64 code, or NULL on error
 */
void *translate_block(uint64_t guest_pc)
{
    const uint8_t *insn_ptr = (const uint8_t *)(uintptr_t)guest_pc;
    code_buffer_t code_buf;
    static uint8_t code_cache[65536];  /* 64KB code cache per block */
    int terminated = 0;
    int insn_count = 0;

    fprintf(stderr, "[translate_block] guest_pc=0x%lx\n", guest_pc);
    fflush(stderr);

    /* Check translation cache first */
    void *cached = refactored_translation_cache_lookup(guest_pc);
    fprintf(stderr, "[translate_block] cache lookup returned %p\n", cached);
    fflush(stderr);
    if (cached) {
        return cached;
    }

    /* Initialize code buffer for ARM64 emission */
    code_buffer_init_arm64(&code_buf, code_cache, sizeof(code_cache));

    /* Get thread state */
    ThreadState *state = rosetta_get_state();
    (void)state;  /* State management for future use */

    /* Translate up to 64 instructions or until branch */
    while (insn_count < 64 && !terminated) {
        /* Decode x86_64 instruction at current PC */
        x86_insn_t insn;
        int insn_len = decode_x86_insn(insn_ptr, &insn);

        fprintf(stderr, "[translate_block] insn_count=%d insn_len=%d opcode=0x%02x\n", insn_count, insn_len, insn.opcode);
        fflush(stderr);

        if (insn_len == 0) {
            /* Invalid instruction - end block */
            fprintf(stderr, "[translate_block] invalid instruction, breaking\n");
            fflush(stderr);
            break;
        }

        /* Map x86_64 registers to ARM64 */
        uint8_t arm_rd = map_x86_to_arm(insn.reg);
        uint8_t arm_rm = map_x86_to_arm(insn.rm);

        /* Translate using dispatcher */
        TranslateResult result = dispatch_translate_insn(
            &code_buf, &insn, arm_rd, arm_rm, guest_pc);

        fprintf(stderr, "[translate_block] translation success=%d is_block_end=%d\n", result.success, result.is_block_end);
        fflush(stderr);

        if (!result.success) {
            /* Translation failed - emit NOP and continue */
            emit_nop(&code_buf);
        }

        terminated = result.is_block_end;

        /* Advance to next x86_64 instruction */
        insn_ptr += insn.length;
        insn_count++;
    }

    fprintf(stderr, "[translate_block] loop ended, terminated=%d insn_count=%d\n", terminated, insn_count);
    fflush(stderr);

    /* Ensure block ends with RET if not already */
    if (!terminated) {
        fprintf(stderr, "[translate_block] emitting RET\n");
        fflush(stderr);
        emit_ret(&code_buf);
    }

    /* Allocate permanent storage for translated code */
    size_t code_size = code_buffer_get_size_arm64(&code_buf);
    fprintf(stderr, "[translate_block] code_size=%zu\n", code_size);
    fflush(stderr);
    void *perm_code = refactored_code_cache_alloc(code_size);
    fprintf(stderr, "[translate_block] perm_code=%p\n", perm_code);
    fflush(stderr);
    if (!perm_code) {
        perm_code = malloc(code_size);
    }
    if (perm_code) {
        memcpy(perm_code, code_cache, code_size);
        /* Insert into translation cache */
        refactored_translation_cache_insert(guest_pc, perm_code, code_size);
    }

    return perm_code ? perm_code : code_cache;
}

/**
 * translate_block_fast - Fast path translation (lookup only)
 * @guest_pc: Guest x86_64 program counter
 *
 * Returns: Pointer to cached translation or NULL if not found
 */
void *translate_block_fast(uint64_t guest_pc)
{
    return refactored_translation_cache_lookup(guest_pc);
}

/**
 * rosetta_translate - Main translation entry point
 * @guest_pc: Guest x86_64 program counter
 *
 * Translates the block at guest_pc and returns host code pointer.
 * Returns: Pointer to translated ARM64 code
 */
void *rosetta_translate(uint64_t guest_pc)
{
    return translate_block(guest_pc);
}

/* ============================================================================
 * Execution Entry Points
 * ============================================================================ */

/**
 * rosetta_execute - Execute translated code
 * @host_code: Pointer to translated ARM64 code
 *
 * Executes the translated code and updates guest state.
 */
void rosetta_execute(void *host_code)
{
    if (!host_code) {
        return;
    }

    /* Execute the translated code */
    void (*func)(void) = (void (*)(void))host_code;
    func();
}

/**
 * rosetta_run - Run x86_64 code at given address via translation
 * @guest_pc: Guest x86_64 program counter
 *
 * Translates x86_64 code to ARM64 and executes.
 */
void rosetta_run(uint64_t guest_pc)
{
    void *host_code = rosetta_translate(guest_pc);
    rosetta_execute(host_code);
}

/**
 * execute_translated - Execute translated code with state
 * @state: Thread state
 * @block: Pointer to translated block
 */
void execute_translated(ThreadState *state, void *block)
{
    (void)state;  /* State is updated during translation */
    rosetta_execute(block);
}

/* ============================================================================
 * Interpreter Mode (for testing and fallback)
 * ============================================================================ */

/**
 * rosetta_interpret - Interpret a single ARM64 instruction
 * @insn: ARM64 instruction encoding
 * @state: Thread state to update
 * @pc: Current program counter
 * Returns: Number of bytes consumed (always 4 for ARM64)
 */
uint64_t rosetta_interpret(uint32_t insn, ThreadState *state, uint64_t pc)
{
    uint8_t rd = insn & 0x1F;
    uint8_t rn = (insn >> 5) & 0x1F;
    uint8_t rm = (insn >> 16) & 0x1F;

    /* Decode and execute based on instruction class */
    if ((insn & 0x7F000000) == 0x0B000000) {  /* ADD */
        state->host.x[rd] = state->host.x[rn] + state->host.x[rm];
    } else if ((insn & 0x7F000000) == 0x4B000000) {  /* SUB */
        state->host.x[rd] = state->host.x[rn] - state->host.x[rm];
    } else if ((insn & 0x7F000000) == 0x0A000000) {  /* AND */
        state->host.x[rd] = state->host.x[rn] & state->host.x[rm];
    } else if ((insn & 0x7F000000) == 0x2A000000) {  /* ORR */
        state->host.x[rd] = state->host.x[rn] | state->host.x[rm];
    } else if ((insn & 0x7F000000) == 0x4A000000) {  /* EOR */
        state->host.x[rd] = state->host.x[rn] ^ state->host.x[rm];
    } else if ((insn & 0x7FE00000) == 0x2A200000) {  /* MVN */
        state->host.x[rd] = ~state->host.x[rm];
    } else if ((insn & 0x7FE0FC00) == 0x1B007C00) {  /* MUL */
        state->host.x[rd] = state->host.x[rn] * state->host.x[rm];
    } else if ((insn & 0x7F000000) == 0xEB000000) {  /* CMP */
        uint64_t op1 = state->host.x[rn];
        uint64_t op2 = state->host.x[rm];
        uint64_t result = op1 - op2;
        state->host.pstate = 0;
        if (result & (1ULL << 63)) state->host.pstate |= (1ULL << 31);  /* N */
        if (result == 0) state->host.pstate |= (1ULL << 30);  /* Z */
        if (op1 < op2) state->host.pstate |= (1ULL << 29);  /* C */
    } else if ((insn & 0xFF200000) == 0xEA000000) {  /* TST */
        uint64_t result = state->host.x[rn] & state->host.x[rm];
        state->host.pstate = 0;
        if (result & (1ULL << 63)) state->host.pstate |= (1ULL << 31);  /* N */
        if (result == 0) state->host.pstate |= (1ULL << 30);  /* Z */
    } else if ((insn & 0xFF800000) == 0xD2800000) {  /* MOVZ */
        uint16_t imm16 = (insn >> 5) & 0xFFFF;
        uint8_t shift = ((insn >> 21) & 0x03) * 16;
        state->host.x[rd] = (uint64_t)imm16 << shift;
    } else if ((insn & 0xFF800000) == 0xF2800000) {  /* MOVK */
        uint16_t imm16 = (insn >> 5) & 0xFFFF;
        uint8_t shift = ((insn >> 21) & 0x03) * 16;
        state->host.x[rd] = (state->host.x[rd] & ~(0xFFFFULL << shift)) |
                              ((uint64_t)imm16 << shift);
    } else if ((insn & 0xFF800000) == 0x12800000) {  /* MOVN */
        uint16_t imm16 = (insn >> 5) & 0xFFFF;
        uint8_t shift = ((insn >> 21) & 0x03) * 16;
        state->host.x[rd] = ~(uint64_t)imm16 << shift;
    } else if ((insn & 0xFC000000) == 0x14000000) {  /* B */
        int32_t imm26 = (int32_t)((insn & 0x03FFFFFF) << 6) >> 4;
        state->host.pc = pc + imm26;
        return 4;
    } else if ((insn & 0xFC000000) == 0x94000000) {  /* BL */
        int32_t imm26 = (int32_t)((insn & 0x03FFFFFF) << 6) >> 4;
        state->host.x[30] = pc + 4;
        state->host.pc = pc + imm26;
        return 4;
    } else if ((insn & 0xFFFFFBFF) == 0xD65F0000) {  /* RET */
        state->host.pc = state->host.x[30];
        return 4;
    } else if ((insn & 0xFFFFFC00) == 0xD61F0000) {  /* BR */
        state->host.pc = state->host.x[rn];
        return 4;
    } else if ((insn & 0xFFC00000) == 0xF9400000) {  /* LDR */
        uint8_t size = (insn >> 2) & 0x03;
        uint16_t imm12 = ((insn >> 10) & 0xFFF) << size;
        uint64_t addr = state->host.x[rn] + imm12;
        switch (size) {
            case 0: state->host.x[rd] = *(uint8_t *)(uintptr_t)addr; break;
            case 1: state->host.x[rd] = *(uint16_t *)(uintptr_t)addr; break;
            case 2: state->host.x[rd] = *(uint32_t *)(uintptr_t)addr; break;
            case 3: state->host.x[rd] = *(uint64_t *)(uintptr_t)addr; break;
        }
    } else if ((insn & 0xFF800000) == 0xF9000000) {  /* STR */
        uint8_t size = (insn >> 2) & 0x03;
        uint16_t imm12 = ((insn >> 10) & 0xFFF) << size;
        uint64_t addr = state->host.x[rn] + imm12;
        switch (size) {
            case 0: *(uint8_t *)(uintptr_t)addr = (uint8_t)state->host.x[rd]; break;
            case 1: *(uint16_t *)(uintptr_t)addr = (uint16_t)state->host.x[rd]; break;
            case 2: *(uint32_t *)(uintptr_t)addr = (uint32_t)state->host.x[rd]; break;
            case 3: *(uint64_t *)(uintptr_t)addr = state->host.x[rd]; break;
        }
    }

    state->host.pc = pc + 4;
    return 4;
}

/**
 * rosetta_run_interpreter - Run ARM64 code in interpreter mode
 * @guest_pc: Guest ARM64 program counter
 * @max_insns: Maximum number of instructions to execute
 *
 * Interprets ARM64 code without translation.
 * Useful for testing and debugging.
 */
void rosetta_run_interpreter(uint64_t guest_pc, int max_insns)
{
    ThreadState *state = rosetta_get_state();
    int count = 0;

    state->host.pc = guest_pc;

    while (count < max_insns) {
        uint32_t insn = *(uint32_t *)(uintptr_t)state->host.pc;
        rosetta_interpret(insn, state, state->host.pc);

        /* Check for exit conditions */
        if (insn == 0xD65F03C0) {  /* RET X30 */
            break;
        }
        count++;
    }
}

/* ============================================================================
 * Syscall Handling
 * ============================================================================ */

/**
 * get_syscall_handler - Get system call handler
 * @nr: System call number
 * Returns: Pointer to syscall handler function
 */
void *get_syscall_handler(int nr)
{
    (void)nr;  /* Placeholder for future implementation */
    return NULL;
}

/**
 * handle_syscall - Handle system call
 * @state: Thread state
 * @nr: System call number
 * Returns: System call result
 */
int64_t handle_syscall(ThreadState *state, int nr)
{
    (void)state;
    (void)nr;
    return -1;  /* Not implemented */
}

/**
 * helper_syscall_enter - Helper for syscall entry
 * @state: Thread state
 * @nr: System call number
 */
void helper_syscall_enter(ThreadState *state, int nr)
{
    (void)state;
    (void)nr;
}

/**
 * helper_syscall_exit - Helper for syscall exit
 * @state: Thread state
 * @result: System call result
 */
void helper_syscall_exit(ThreadState *state, int64_t result)
{
    (void)state;
    (void)result;
}

/**
 * helper_interrupt - Helper for interrupt handling
 * @state: Thread state
 * @vector: Interrupt vector
 */
void helper_interrupt(ThreadState *state, int vector)
{
    (void)state;
    (void)vector;
}
