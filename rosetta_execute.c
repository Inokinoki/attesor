/* ============================================================================
 * Rosetta Execution Engine - Implementation
 * ============================================================================
 *
 * Main execution engine for running x86_64 binaries on ARM64
 * ============================================================================
 */

#include "rosetta_execute.h"
#include "rosetta_refactored.h"
#include "rosetta_refactored_exec.h"
#include "rosetta_x86_insns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

/* Forward declarations for code buffer (to avoid header conflicts) */
typedef struct code_buffer code_buffer_t;

/* Forward declaration for TranslateResult */
typedef struct {
    bool success;           /* Translation succeeded */
    bool is_block_end;      /* Instruction ends the basic block */
    int insn_length;        /* Length of translated instruction */
} TranslateResult;

/* Forward declarations to avoid conflicts */
extern void *refactored_translation_cache_lookup(uint64_t guest_pc);
extern void refactored_translation_cache_insert(uint64_t guest_pc, void *code, uint32_t size);
extern void *refactored_code_cache_alloc(size_t size);

/* Register mapping: x86_64 -> ARM64 */
static inline uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

/* External translation functions (from existing codebase) */
extern void *translate_block(uint64_t guest_pc);
extern void rosetta_run(uint64_t guest_pc);

/* External dispatcher function */
extern TranslateResult dispatch_translate_insn(
    void *code_buf, const x86_insn_t *insn,
    uint8_t arm_rd, uint8_t arm_rm, uint64_t block_pc);

/* External ARM64 emit functions */
extern void emit_byte(void *buf, uint8_t byte);
extern void emit_word32(void *buf, uint32_t word);
extern void emit_word64(void *buf, uint64_t word);
extern void emit_nop(void *buf);
extern void emit_ret(void *buf);

/* External code buffer functions */
extern int code_buffer_init(code_buffer_t *buf, u8 *buffer, u32 size);
extern u32 code_buffer_get_size(code_buffer_t *buf);

/* ============================================================================
 * Instruction Fetching
 * ============================================================================ */

/**
 * Fetch a single byte from guest memory
 */
int8_t rosetta_fetch_byte(rosetta_memmgr_t *memmgr, uint64_t guest_addr)
{
    if (!memmgr) {
        return -1;
    }

    uint8_t value;
    ssize_t ret = rosetta_memmgr_read(memmgr, guest_addr, &value, 1);
    if (ret != 1) {
        return -1;
    }

    return (int8_t)value;
}

/**
 * Fetch x86_64 instruction from guest memory
 */
ssize_t rosetta_fetch_insn(rosetta_memmgr_t *memmgr,
                           uint64_t guest_addr,
                           uint8_t *insn_buf,
                           size_t max_len)
{
    if (!memmgr || !insn_buf || max_len == 0) {
        return -1;
    }

    /* x86_64 instructions are at most 15 bytes */
    if (max_len > 15) {
        max_len = 15;
    }

    /* Fetch instruction bytes from guest memory */
    ssize_t ret = rosetta_memmgr_read(memmgr, guest_addr, insn_buf, max_len);
    if (ret < 0) {
        return -1;
    }

    /* Check if we got enough bytes */
    if (ret == 0) {
        /* Tried to read past executable region */
        return -1;
    }

    return ret;
}

/**
 * Get x86_64 instruction length
 * This is a simplified implementation
 * Real x86_64 instruction length decoding is complex
 */
ssize_t rosetta_insn_len(const uint8_t *insn_buf, size_t buf_len)
{
    if (!insn_buf || buf_len == 0) {
        return -1;
    }

    /* Simplified x86_64 instruction length decoder */
    /* This is a basic implementation - a full one would use the x86 decoder */

    uint8_t prefix = 0;
    uint8_t rex = 0;
    uint8_t opcode = 0;
    size_t len = 0;

    /* Check for REX prefix (0x40-0x4F) */
    if (buf_len > 0 && insn_buf[0] >= 0x40 && insn_buf[0] <= 0x4F) {
        rex = insn_buf[0];
        len = 1;
    }

    /* Check for legacy prefixes (0x66, 0x67, etc.) */
    while (len < buf_len) {
        uint8_t b = insn_buf[len];
        if (b == 0x66 || b == 0x67 || b == 0xF0 || b == 0xF2 || b == 0xF3) {
            len++;
            continue;
        }
        break;
    }

    if (len >= buf_len) {
        return buf_len;  /* Use what we have */
    }

    /* Get opcode */
    opcode = insn_buf[len];
    len++;

    /* Simple length estimation based on opcode */
    /* This is NOT comprehensive - real x86 decoding is very complex */
    switch (opcode) {
        /* Simple instructions (1 byte) */
        case 0x90: /* NOP */
        case 0xC3: /* RET */
        case 0xC2: /* RET imm16 */
            if (opcode == 0xC2) len += 1;  /* RET has 2-byte immediate */
            return len;

        /* MOV r32, r/m32 (0x89 /r) */
        case 0x89:
            len += 2;  /* ModR/M + SIB + disp */
            return len;

        /* MOV r/m32, r32 (0x8B /r) */
        case 0x8B:
            len += 2;
            return len;

        /* MOV r32, imm32 (0xB8+r) */
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            len += 4;  /* 4-byte immediate */
            return len;

        /* CALL rel32 (0xE8) */
        case 0xE8:
            len += 4;  /* 4-byte relative offset */
            return len;

        /* JMP rel8 (0xEB) */
        case 0xEB:
            len += 1;  /* 1-byte relative offset */
            return len;

        /* JMP rel32 (0xE9) */
        case 0xE9:
            len += 4;  /* 4-byte relative offset */
            return len;

        /* Conditional jumps (0x70-0x7F) */
        default:
            if (opcode >= 0x70 && opcode <= 0x7F) {
                len += 1;  /* 1-byte relative offset */
                return len;
            }
            /* Unknown instruction - use minimum length */
            return len;
    }

    return len;
}

/* ============================================================================
 * Block Translation with Memory Manager
 * ============================================================================
 */

/**
 * Translate a basic block using guest memory manager
 * @param memmgr: Memory manager for fetching instructions
 * @param guest_pc: Guest program counter (block start address)
 * @param out_size: Output parameter for translated code size
 * @return Pointer to translated ARM64 code, or NULL on error
 */
static void *translate_block_with_memmgr(rosetta_memmgr_t *memmgr,
                                          uint64_t guest_pc,
                                          size_t *out_size)
{
    if (!memmgr || !out_size) {
        return NULL;
    }

    printf("[TRANS] ==================================================\n");
    printf("[TRANS] Translating block at 0x%lx\n", guest_pc);
    printf("[TRANS] ==================================================\n");

    /* Check translation cache first */
    void *cached = refactored_translation_cache_lookup(guest_pc);
    if (cached) {
        printf("[TRANS] ✅ Block found in cache: %p\n", cached);
        *out_size = 0;  /* Size unknown from cache */
        return cached;
    }

    printf("[TRANS] 🔍 Block not in cache, translating...\n");

    /* Initialize code buffer for ARM64 emission */
    static uint8_t code_cache[65536];  /* 64KB code cache per block */
    static uint8_t code_buf_struct[4096];  /* Space for code_buffer struct */
    void *code_buf = code_buf_struct;

    printf("[TRANS] 📝 Initializing code buffer: buffer=%p, size=%zu\n",
           code_cache, sizeof(code_cache));

    code_buffer_init_arm64(code_buf, code_cache, sizeof(code_cache));

    printf("[TRANS] ✅ Code buffer initialized\n");

    /* Translate up to 64 instructions or until branch */
    uint64_t current_pc = guest_pc;
    int insn_count = 0;
    int terminated = 0;
    const int max_insns = 64;

    printf("[TRANS] 🔄 Starting translation loop (max %d instructions)\n", max_insns);

    while (insn_count < max_insns && !terminated) {
        /* Fetch x86_64 instruction from guest memory */
        uint8_t insn_buf[15];
        ssize_t fetched = rosetta_fetch_insn(memmgr, current_pc, insn_buf, sizeof(insn_buf));

        if (fetched <= 0) {
            printf("[TRANS] ❌ Failed to fetch instruction at 0x%lx\n", current_pc);
            break;
        }

        printf("[TRANS] [%d] 📥 Fetched %zd bytes at 0x%lx: ", insn_count, fetched, current_pc);
        for (int i = 0; i < fetched && i < 8; i++) {
            printf("%02x ", insn_buf[i]);
        }
        printf("\n");

        /* Decode x86_64 instruction */
        x86_insn_t insn;
        int insn_len = decode_x86_insn(insn_buf, &insn);

        if (insn_len == 0) {
            printf("[TRANS] ❌ Invalid instruction at 0x%lx, ending block\n", current_pc);
            break;
        }

        printf("[TRANS] [%d] 🔎 Decoded: len=%d opcode=0x%02x reg=%d rm=%d\n",
               insn_count, insn_len, insn.opcode, insn.reg, insn.rm);

        /* Map x86_64 registers to ARM64 */
        uint8_t arm_rd = map_x86_to_arm(insn.reg);
        uint8_t arm_rm = map_x86_to_arm(insn.rm);

        printf("[TRANS] [%d] 🔄 Register mapping: x86 reg=%d → ARM r%d, x86 rm=%d → ARM r%d\n",
               insn_count, insn.reg, arm_rd, insn.rm, arm_rm);

        /* Translate using dispatcher */
        printf("[TRANS] [%d] ⚙️ Calling dispatcher...\n", insn_count);
        TranslateResult result = dispatch_translate_insn(
            code_buf, &insn, arm_rd, arm_rm, guest_pc);

        printf("[TRANS] [%d] 📊 Dispatcher result: success=%d is_block_end=%d\n",
               insn_count, result.success, result.is_block_end);

        if (!result.success) {
            /* Translation failed - emit NOP and continue */
            printf("[TRANS] [%d] ⚠️ Translation failed, emitting NOP\n", insn_count);
            emit_nop(code_buf);
        }

        terminated = result.is_block_end;

        /* Advance to next x86_64 instruction */
        current_pc += insn.length;
        insn_count++;
    }

    printf("[TRANS] ==================================================\n");
    printf("[TRANS] 📈 Translation complete: %d instructions\n", insn_count);
    printf("[TRANS] 📍 Final PC: 0x%lx\n", current_pc);
    printf("[TRANS] ==================================================\n");

    /* Ensure block ends with RET if not already */
    if (!terminated) {
        printf("[TRANS] ⚠️ Block not terminated, emitting RET\n");
        emit_ret(code_buf);
    }

    /* Get code size */
    size_t code_size = code_buffer_get_size_arm64(code_buf);

    printf("[TRANS] ==================================================\n");
    printf("[TRANS] 📊 Generated ARM64 code:\n");
    printf("[TRANS]   Size: %zu bytes\n", code_size);

    if (code_size > 0) {
        printf("[TRANS]   First bytes: ");
        uint8_t *code_bytes = (uint8_t *)code_cache;
        size_t show_bytes = code_size < 16 ? code_size : 16;
        for (size_t i = 0; i < show_bytes; i++) {
            printf("%02x ", code_bytes[i]);
        }
        if (code_size > 16) {
            printf("...");
        }
        printf("\n");
    } else {
        printf("[TRANS]   ❌ WARNING: Code size is 0!\n");
    }

    printf("[TRANS] ==================================================\n");

    /* Allocate permanent storage for translated code */
    printf("[TRANS] 💾 Allocating permanent storage (%zu bytes)\n", code_size);

    void *perm_code = refactored_code_cache_alloc(code_size);
    if (!perm_code) {
        /* Fallback to malloc */
        printf("[TRANS] ⚠️ Code cache alloc failed, using malloc\n");
        perm_code = malloc(code_size);
    }

    if (perm_code) {
        /* Copy generated code to permanent storage */
        printf("[TRANS] 📋 Copying code to %p\n", perm_code);
        memcpy(perm_code, code_cache, code_size);

        /* Make code executable */
        #ifdef __linux__
            long page_size = sysconf(_SC_PAGESIZE);
            void *aligned_ptr = (void *)(((uintptr_t)perm_code) & ~((uintptr_t)page_size - 1));
            size_t aligned_size = ((code_size + page_size - 1) & ~((uintptr_t)page_size - 1));

            printf("[TRANS] 🔒 Setting memory protection: addr=%p size=%zu (RWX)\n",
                   aligned_ptr, aligned_size);

            if (mprotect(aligned_ptr, aligned_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
                perror("[TRANS] ❌ mprotect failed");
            } else {
                printf("[TRANS] ✅ Memory protection set successfully\n");
            }
        #endif

        /* Insert into translation cache */
        printf("[TRANS] 💿 Inserting into translation cache: 0x%lx → %p\n",
               guest_pc, perm_code);
        refactored_translation_cache_insert(guest_pc, perm_code, code_size);

        printf("[TRANS] ✅ Translation complete: %p (%zu bytes)\n", perm_code, code_size);
    } else {
        printf("[TRANS] ❌ Failed to allocate permanent storage, using temp buffer\n");
        perm_code = code_cache;
    }

    printf("[TRANS] ==================================================\n");

    *out_size = code_size;
    return perm_code;
}

/* ============================================================================
 * Execution Context Management
 * ============================================================================ */

/**
 * Create execution context
 */
rosetta_exec_ctx_t *rosetta_exec_create(RosettaThreadState *state,
                                         rosetta_memmgr_t *memmgr)
{
    if (!state || !memmgr) {
        return NULL;
    }

    rosetta_exec_ctx_t *ctx = calloc(1, sizeof(rosetta_exec_ctx_t));
    if (!ctx) {
        return NULL;
    }

    ctx->state = state;
    ctx->memmgr = memmgr;
    ctx->guest_pc = 0;
    ctx->blocks_executed = 0;
    ctx->instructions_executed = 0;
    ctx->is_running = 0;
    ctx->exit_code = 0;

    return ctx;
}

/**
 * Destroy execution context
 */
void rosetta_exec_destroy(rosetta_exec_ctx_t *ctx)
{
    if (ctx) {
        free(ctx);
    }
}

/* ============================================================================
 * Single Instruction Execution
 * ============================================================================ */

/**
 * Execute single instruction
 */
int rosetta_execute_single(rosetta_exec_ctx_t *ctx)
{
    if (!ctx || !ctx->state || !ctx->memmgr) {
        return -1;
    }

    /* Set guest PC to current location */
    ctx->state->guest.rip = ctx->guest_pc;
    ctx->state->current_pc = ctx->guest_pc;

    /* Fetch instruction from guest memory */
    uint8_t insn_buf[15];
    ssize_t insn_len = rosetta_fetch_insn(ctx->memmgr, ctx->guest_pc,
                                         insn_buf, sizeof(insn_buf));
    if (insn_len < 0) {
        fprintf(stderr, "[EXEC] Failed to fetch instruction at 0x%lx\n",
                ctx->guest_pc);
        return -1;
    }

    /* Get instruction length */
    ssize_t len = rosetta_insn_len(insn_buf, insn_len);
    if (len < 0) {
        len = 1;  /* Minimum length */
    }

    /* Note: Debug output controlled by verbose flag for now */
    /* In a full implementation, we'd have proper debug config */
    (void)ctx;  /* Unused for now */

    /* Update PC */
    ctx->guest_pc += len;
    ctx->instructions_executed++;

    return 0;
}

/* ============================================================================
 * Block Execution
 * ============================================================================ */

/**
 * Translate and execute a basic block
 */
int rosetta_execute_block(rosetta_exec_ctx_t *ctx, uint64_t block_start)
{
    if (!ctx || !ctx->state || !ctx->memmgr) {
        return -1;
    }

    printf("[EXEC] Executing block at 0x%lx\n", block_start);

    /* Set guest PC to block start */
    ctx->state->guest.rip = block_start;
    ctx->state->current_pc = block_start;
    ctx->guest_pc = block_start;

    /* Translate the block using memory manager */
    size_t code_size = 0;
    void *translated_code = translate_block_with_memmgr(ctx->memmgr, block_start, &code_size);

    if (!translated_code) {
        fprintf(stderr, "[EXEC] Failed to translate block at 0x%lx\n", block_start);
        return -1;
    }

    printf("[EXEC] Translated code at %p (%zu bytes)\n", translated_code, code_size);

    /* Execute the translated ARM64 code */
    printf("[EXEC] Executing translated code...\n");

    typedef void (*translated_func_t)(void);
    ((translated_func_t)translated_code)();

    printf("[EXEC] Block execution complete\n");

    /* Update statistics */
    ctx->blocks_executed++;

    /* Note: In a full implementation, we would update the guest PC based on
     * the actual control flow. For now, we'll estimate the instructions executed. */
    ctx->instructions_executed += 10;  /* Rough estimate */

    return 0;
}

/* ============================================================================
 * Main Execution Loop
 * ============================================================================ */

/**
 * Execute x86_64 code until termination
 */
int rosetta_execute(rosetta_exec_ctx_t *ctx, uint64_t entry_point)
{
    if (!ctx || !ctx->state || !ctx->memmgr) {
        return -1;
    }

    printf("[EXEC] Starting execution at 0x%lx\n", entry_point);

    /* Set running state */
    ctx->is_running = 1;
    ctx->guest_pc = entry_point;
    ctx->instructions_executed = 0;
    ctx->exit_code = 0;

    /* Main execution loop */
    const uint64_t max_insns = 100000;  /* Prevent infinite loops */
    int ret = 0;

    while (ctx->is_running && ctx->instructions_executed < max_insns) {
        /* Execute a basic block */
        ret = rosetta_execute_block(ctx, ctx->guest_pc);
        if (ret < 0) {
            fprintf(stderr, "[EXEC] Execution error at 0x%lx\n",
                    ctx->guest_pc);
            ret = -1;
            break;
        }

        if (ret == 0) {
            /* No instructions executed - possibly hit invalid memory */
            fprintf(stderr, "[EXEC] No instructions executed, stopping\n");
            break;
        }

        /* Check if we should continue */
        /* For now, just run a limited number of instructions */
        if (ctx->instructions_executed >= 100) {
            printf("[EXEC] Reached instruction limit (100)\n");
            break;
        }
    }

    ctx->is_running = 0;

    printf("[EXEC] Execution complete:\n");
    printf("[EXEC]   Instructions executed: %lu\n", ctx->instructions_executed);
    printf("[EXEC]   Blocks executed: %lu\n", ctx->blocks_executed);
    printf("[EXEC]   Final PC: 0x%lx\n", ctx->guest_pc);
    printf("[EXEC]   Exit code: %d\n", ctx->exit_code);

    return ret;
}

/**
 * Stop execution
 */
int rosetta_execute_stop(rosetta_exec_ctx_t *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->is_running = 0;
    return 0;
}
