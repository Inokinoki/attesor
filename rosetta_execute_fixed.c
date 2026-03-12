/* ============================================================================
 * Rosetta Execution Engine - ARM64 Translation Integration (FIXED)
 * ============================================================================
 *
 * Fixed version that properly integrates with rosetta_codegen.c code buffer
 * ============================================================================ */

#include "rosetta_execute.h"
#include "rosetta_refactored.h"
#include "rosetta_refactored_exec.h"
#include "rosetta_x86_insns.h"
#include "rosetta_codegen.h"
#include "rosetta_exec_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

/* Forward declarations for dispatcher to avoid header conflicts */
typedef struct {
    bool success;
    bool is_block_end;
    int insn_length;
} TranslateResult;

typedef enum {
    INSN_ALU,
    INSN_MEMORY,
    INSN_BRANCH,
    INSN_BIT,
    INSN_STRING,
    INSN_SPECIAL,
    INSN_UNKNOWN
} InsnCategory;

extern InsnCategory dispatch_classify_insn(const x86_insn_t *insn);
extern TranslateResult dispatch_translate_insn(
    void *code_buf, const x86_insn_t *insn,
    uint8_t arm_rd, uint8_t arm_rm, uint64_t block_pc);

/* Forward declarations */
extern void *refactored_translation_cache_lookup(uint64_t guest_pc);
extern void refactored_translation_cache_insert(uint64_t guest_pc, void *code, uint32_t size);
extern void *refactored_code_cache_alloc(size_t size);

/* Register mapping: x86_64 -> ARM64 */
static inline uint8_t map_x86_to_arm(uint8_t x86_reg)
{
    return x86_reg & 0x0F;  /* RAX-R15 -> X0-X15 */
}

/* ============================================================================
 * Block Translation with Memory Manager (FIXED VERSION)
 * ============================================================================
 */

/**
 * Translate a basic block using guest memory manager
 * This version properly uses code_buffer_t from rosetta_codegen.c
 */
static void *translate_block_with_memmgr(rosetta_memmgr_t *memmgr,
                                          uint64_t guest_pc,
                                          size_t *out_size)
{
    fprintf(stderr, "[TRANS DEBUG] translate_block_with_memgr called: guest_pc=0x%lx\n", guest_pc);

    if (!memmgr || !out_size) {
        fprintf(stderr, "[TRANS DEBUG] NULL parameter!\n");
        return NULL;
    }

    fprintf(stderr, "[TRANS DEBUG] About to print translation header\n");
    printf("[TRANS] ==================================================\n");
    printf("[TRANS] Translating block at 0x%lx\n", guest_pc);
    printf("[TRANS] ==================================================\n");
    fprintf(stderr, "[TRANS DEBUG] Translation header printed\n");

    /* Check translation cache first */
    void *cached = refactored_translation_cache_lookup(guest_pc);
    if (cached) {
        printf("[TRANS] ✅ Block found in cache: %p\n", cached);
        *out_size = 0;
        return cached;
    }

    printf("[TRANS] 🔍 Block not in cache, translating...\n");

    /* Initialize code buffer for ARM64 emission */
    static uint8_t code_cache[65536];  /* 64KB code cache per block */
    static code_buffer_t code_buf_struct;
    code_buffer_t *code_buf = &code_buf_struct;

    printf("[TRANS] 📝 Initializing code buffer: buffer=%p, size=%d\n",
           code_cache, (int)sizeof(code_cache));

    int init_result = code_buffer_init(code_buf, code_cache, sizeof(code_cache));
    printf("[TRANS] 📊 code_buffer_init result: %d\n", init_result);

    if (init_result != 0) {
        printf("[TRANS] ❌ Failed to initialize code buffer (error=%d)\n", init_result);
        return NULL;
    }

    printf("[TRANS] ✅ Code buffer initialized (offset=%u)\n", code_buf->offset);

    /* Translate up to 64 instructions or until branch */
    uint64_t current_pc = guest_pc;
    int insn_count = 0;
    int terminated = 0;
    const int max_insns = 64;  /* Increased to test more translation */

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
        fprintf(stderr, "[TRANS DEBUG] Dispatching: opcode=0x%02x category=%d arm_rd=%d arm_rm=%d\n",
                insn.opcode, dispatch_classify_insn(&insn), arm_rd, arm_rm);

        TranslateResult result = dispatch_translate_insn(
            code_buf, &insn, arm_rd, arm_rm, guest_pc);

        printf("[TRANS] [%d] 📊 Dispatcher result: success=%d is_block_end=%d\n",
               insn_count, result.success, result.is_block_end);
        fprintf(stderr, "[TRANS DEBUG] After dispatch: code_buf->offset=%u\n", code_buf->offset);

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
    u32 code_size_u32 = code_buffer_get_size(code_buf);
    size_t code_size = code_size_u32;

    fprintf(stderr, "[TRANS DEBUG] Code size: %u bytes\n", code_size_u32);

    printf("[TRANS] ==================================================\n");
    printf("[TRANS] 📊 Generated ARM64 code:\n");
    printf("[TRANS]   Size: %u bytes (offset=%u)\n", code_size_u32, code_buf->offset);

    if (code_size > 0) {
        printf("[TRANS]   First bytes: ");
        u8 *code_bytes = (u8 *)code_cache;
        size_t show_bytes = code_size < 16 ? code_size : 16;
        for (size_t i = 0; i < show_bytes; i++) {
            printf("%02x ", code_bytes[i]);
            fprintf(stderr, "%02x ", code_bytes[i]);  /* Also print to stderr */
        }
        if (code_size > 16) {
            printf("...\n");
            printf("[TRANS]   Full dump (all %u bytes):\n", code_size);
            for (size_t i = 0; i < code_size; i++) {
                if (i % 16 == 0) printf("[TRANS]     %04zx: ", i);
                printf("%02x ", code_bytes[i]);
                if (i % 16 == 15 || i == code_size - 1) printf("\n");
            }
        } else {
            printf("\n");
        }
    } else {
        printf("[TRANS]   ❌ WARNING: Code size is 0!\n");
    }

    printf("[TRANS] ==================================================\n");
    fprintf(stderr, "[TRANS DEBUG] About to allocate permanent storage\n");

    /* Allocate permanent storage for translated code */
    printf("[TRANS] 💾 Allocating permanent storage (%u bytes)\n", code_size_u32);

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

        /* Make code executable (W^X compliant: write first, then make executable) */
        #ifdef __linux__
            long page_size = sysconf(_SC_PAGESIZE);
            void *aligned_ptr = (void *)(((uintptr_t)perm_code) & ~((uintptr_t)page_size - 1));
            size_t aligned_size = ((code_size + page_size - 1) & ~((uintptr_t)page_size - 1));

            fprintf(stderr, "[TRANS DEBUG] page_size=%ld aligned_ptr=%p aligned_size=%zu\n",
                    page_size, aligned_ptr, aligned_size);

            /* Memory should already be RW from allocation, now make it RX for execution */
            printf("[TRANS] 🔒 Setting memory protection: addr=%p size=%zu (RX)\n",
                   aligned_ptr, aligned_size);

            int mprotect_result = mprotect(aligned_ptr, aligned_size, PROT_READ | PROT_EXEC);
            fprintf(stderr, "[TRANS DEBUG] mprotect returned: %d\n", mprotect_result);

            if (mprotect_result != 0) {
                perror("[TRANS] ❌ mprotect failed");
            } else {
                printf("[TRANS] ✅ Memory protection set successfully\n");
                fprintf(stderr, "[TRANS DEBUG] Memory protection set successfully\n");
            }
        #endif

        /* Insert into translation cache */
        printf("[TRANS] 💿 Inserting into translation cache: 0x%lx → %p\n",
               guest_pc, perm_code);
        refactored_translation_cache_insert(guest_pc, perm_code, code_size_u32);

        printf("[TRANS] ✅ Translation complete: %p (%u bytes)\n", perm_code, code_size_u32);
    } else {
        printf("[TRANS] ❌ Failed to allocate permanent storage, using temp buffer\n");
        perm_code = code_cache;
    }

    printf("[TRANS] ==================================================\n");

    *out_size = code_size;
    return perm_code;
}

/* ============================================================================
 * Instruction Fetching
 * ============================================================================
 */

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
 */
ssize_t rosetta_insn_len(const uint8_t *insn_buf, size_t buf_len)
{
    if (!insn_buf || buf_len == 0) {
        return -1;
    }

    /* Simplified decoder */
    uint8_t rex = 0;
    size_t len = 0;

    /* Check for REX prefix (0x40-0x4F) */
    if (buf_len > 0 && insn_buf[0] >= 0x40 && insn_buf[0] <= 0x4F) {
        rex = insn_buf[0];
        len = 1;
    }
    (void)rex;  /* REX prefix not used in length calculation */

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
        return buf_len;
    }

    /* Get opcode */
    uint8_t opcode = insn_buf[len];
    len++;

    /* Simple length estimation */
    switch (opcode) {
        /* Simple instructions */
        case 0x90: /* NOP */
        case 0xC3: /* RET */
            return len;

        /* MOV r32, r/m32 (0x89 /r) */
        case 0x89:
            len += 2;
            return len;

        /* MOV r32, imm32 (0xB8+r) */
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
            len += 4;
            return len;

        /* CALL rel32 (0xE8) */
        case 0xE8:
            len += 4;
            return len;

        /* JMP rel8 (0xEB) */
        case 0xEB:
            len += 1;
            return len;

        /* JMP rel32 (0xE9) */
        case 0xE9:
            len += 4;
            return len;

        /* Conditional jumps (0x70-0x7F) */
        default:
            if (opcode >= 0x70 && opcode <= 0x7F) {
                len += 1;
                return len;
            }
            /* Unknown instruction */
            return len;
    }

    return len;
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
 * Block Execution
 * ============================================================================ */

/**
 * Translate and execute a basic block
 */
int rosetta_execute_block(rosetta_exec_ctx_t *ctx, uint64_t block_start)
{
    fprintf(stderr, "[EXEC BLOCK DEBUG] rosetta_execute_block called: block_start=0x%lx\n", block_start);

    if (!ctx || !ctx->state || !ctx->memmgr) {
        fprintf(stderr, "[EXEC BLOCK DEBUG] NULL parameter\n");
        return -1;
    }

    fprintf(stderr, "[EXEC BLOCK DEBUG] About to print executing message\n");
    printf("[EXEC] Executing block at 0x%lx\n", block_start);
    fprintf(stderr, "[EXEC BLOCK DEBUG] Printed executing message\n");

    /* Set guest PC to block start */
    ctx->state->guest.rip = block_start;
    ctx->state->current_pc = block_start;
    ctx->guest_pc = block_start;

    fprintf(stderr, "[EXEC BLOCK DEBUG] PC set, about to call translate_block_with_memmgr\n");

    /* Translate the block using memory manager */
    size_t code_size = 0;
    void *translated_code = translate_block_with_memmgr(ctx->memmgr, block_start, &code_size);

    fprintf(stderr, "[EXEC BLOCK DEBUG] translate_block_with_memmgr returned: code=%p size=%zu\n",
            translated_code, code_size);

    if (!translated_code) {
        fprintf(stderr, "[EXEC] Failed to translate block at 0x%lx\n", block_start);
        return -1;
    }

    printf("[EXEC] Translated code at %p (%zu bytes)\n", translated_code, code_size);
    fprintf(stderr, "[EXEC BLOCK DEBUG] Code address: %p (alignment: %s)\n",
            translated_code, ((uintptr_t)translated_code % 16 == 0) ? "OK" : "BAD");

    if (code_size == 0) {
        fprintf(stderr, "[EXEC] ❌ ERROR: Generated code size is 0!\n");
        return -1;
    }

    /* Disassemble first few instructions */
    fprintf(stderr, "[EXEC DEBUG] About to disassemble instructions\n");
    fflush(stderr);
    printf("[EXEC]   ARM64 instructions (first 10):\n");
    fflush(stdout);
    uint32_t *insns = (uint32_t *)translated_code;
    size_t num_insns = code_size / 4;
    for (size_t i = 0; i < num_insns && i < 10; i++) {
        /* Instructions are stored in memory in little-endian format */
        /* When read as uint32_t on little-endian CPU, we get ARM manual notation */
        uint32_t insn = insns[i];
        printf("[EXEC]     [%2zu] %08x  ", i, insn);

        /* Decode common instructions */
        if (insn == 0xD503201F) {  // NOP
            printf("NOP\n");
            fflush(stdout);
        } else if (insn == 0xD65F03C0) {  // RET
            printf("RET\n");
            fflush(stdout);
        /* MOVZ - Move wide with zero */
        } else if ((insn & 0xFFE00000) == 0xD2800000) {
            uint8_t rd = insn & 0x1F;
            uint16_t imm16 = (insn >> 5) & 0xFFFF;
            uint8_t hw = (insn >> 21) & 0x3;
            printf("MOVZ X%d, #0x%x (shift=%d)\n", rd, imm16, hw << 4);
            fflush(stdout);
        /* MOVK - Move wide with keep */
        } else if ((insn & 0xFFE00000) == 0x72800000) {
            uint8_t rd = insn & 0x1F;
            uint16_t imm16 = (insn >> 5) & 0xFFFF;
            uint8_t hw = (insn >> 21) & 0x3;
            printf("MOVK X%d, #0x%x (shift=%d)\n", rd, imm16, hw << 4);
            fflush(stdout);
        /* LDR/STR - Load/Store register (unsigned offset) */
        } else if ((insn & 0xFFC00000) == 0xF9400000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 10) & 0xFFF) << 3;
            printf("LDR X%d, [X%d, #0x%x]\n", rt, rn, offset);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xF9000000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 10) & 0xFFF) << 3;
            printf("STR X%d, [X%d, #0x%x]\n", rt, rn, offset);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xB9400000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 10) & 0xFFF) << 2;
            printf("LDR W%d, [X%d, #0x%x]\n", rt, rn, offset);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xB9000000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 10) & 0xFFF) << 2;
            printf("STR W%d, [X%d, #0x%x]\n", rt, rn, offset);
            fflush(stdout);
        /* LDP/STP - Load/Store pair */
        } else if ((insn & 0xFFC00000) == 0xA9400000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rt2 = (insn >> 10) & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 15) & 0x7F) << 3;
            printf("LDP X%d, X%d, [X%d, #0x%x]\n", rt, rt2, rn, offset);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xA9000000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rt2 = (insn >> 10) & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t offset = ((insn >> 15) & 0x7F) << 3;
            printf("STP X%d, X%d, [X%d, #0x%x]\n", rt, rt2, rn, offset);
            fflush(stdout);
        } else if ((insn & 0x7FE00000) == 0xAA000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rm = (insn >> 16) & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            if (rn == 31) {
                printf("MOV X%d, X%d\n", rd, rm);
            } else {
                printf("ORR X%d, X%d, X%d\n", rd, rn, rm);
            }
            fflush(stdout);
        /* ORR with shift */
        } else if ((insn & 0x7F800000) == 0x2A000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rm = (insn >> 16) & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint8_t shift = (insn >> 22) & 0x3;
            const char *shifts[] = {"LSL", "LSR", "ASR", "ROR"};
            uint8_t shift_amount = (insn >> 10) & 0x3F;
            printf("ORR X%d, X%d, X%d, %s #%d\n", rd, rn, rm, shifts[shift], shift_amount);
            fflush(stdout);
        } else if ((insn & 0x7FE00000) == 0x0B000000) {
            printf("ADD X%d, X%d, X%d\n", insn & 0x1F, (insn >> 5) & 0x1F, (insn >> 16) & 0x1F);
            fflush(stdout);
        /* ADD/SUB immediate */
        } else if ((insn & 0xFFC00000) == 0x91000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t imm12 = ((insn >> 10) & 0xFFF);
            const char *rd_name = (rd == 31) ? "SP" : "";
            const char *rn_name = (rn == 31) ? "SP" : "";
            printf("ADD %s%s%d, %s%s%d, #0x%x\n", rd_name, rd_name[0] ? "" : "X", rd, rn_name, rn_name[0] ? "" : "X", rn, imm12);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xD1000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t imm12 = ((insn >> 10) & 0xFFF);
            const char *rd_name = (rd == 31) ? "SP" : "";
            const char *rn_name = (rn == 31) ? "SP" : "";
            printf("SUB %s%s%d, %s%s%d, #0x%x\n", rd_name, rd_name[0] ? "" : "X", rd, rn_name, rn_name[0] ? "" : "X", rn, imm12);
            fflush(stdout);
        /* ADDS/SUBS immediate (with flags) */
        } else if ((insn & 0xFFC00000) == 0xB1000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t imm12 = ((insn >> 10) & 0xFFF);
            printf("ADDS X%d, X%d, #0x%x\n", rd, rn, imm12);
            fflush(stdout);
        } else if ((insn & 0xFFC00000) == 0xF1000000) {
            uint8_t rd = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint16_t imm12 = ((insn >> 10) & 0xFFF);
            const char *rd_name = (rd == 31) ? "SP" : "";
            const char *rn_name = (rn == 31) ? "SP" : "";
            printf("SUBS %s%s%d, %s%s%d, #0x%x\n", rd_name, rd_name[0] ? "" : "X", rd, rn_name, rn_name[0] ? "" : "X", rn, imm12);
            fflush(stdout);
        /* Load/Store register (unscaled offset) */
        } else if ((insn & 0x3FE00000) == 0x38000000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            uint8_t size = (insn >> 30) & 0x3;
            int16_t offset = ((insn >> 12) & 0x1FF) | (((insn >> 26) & 1) << 8);
            if (offset & (1 << 8)) offset |= ~((1 << 9) - 1);  // Sign extend
            const char *regs[] = {"B", "H", "", "X"};
            const char *reg = (size == 3) ? "X" : (size == 2) ? "W" : regs[size];
            if (insn & (1 << 22)) {  // Load
                printf("LDR %s%d, [X%d, #%d]\n", reg, rt, rn, offset);
            } else {  // Store
                printf("STR %s%d, [X%d, #%d]\n", reg, rt, rn, offset);
            }
            fflush(stdout);
        /* PC-relative addressing */
        } else if ((insn & 0x9F000000) == 0x10000000) {
            uint8_t rd = insn & 0x1F;
            uint32_t immhi = ((insn >> 5) & 0x7FFFF) << 2;
            uint32_t immlo = ((insn >> 29) & 0x3);
            int32_t offset = (immhi | immlo);
            if (offset & (1 << 20)) offset |= ~((1 << 21) - 1);  // Sign extend
            if ((insn & (1 << 31)) == 0) {  // ADR
                printf("ADR X%d, +#0x%x\n", rd, offset);
            } else {  // ADRP
                printf("ADRP X%d, +#0x%x\n", rd, offset);
            }
            fflush(stdout);
        } else if ((insn & 0xFFE00C00) == 0xF8000000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            int16_t offset = ((insn >> 12) & 0x1FF) | (((insn >> 26) & 1) << 8);
            if (offset & (1 << 8)) offset |= ~((1 << 9) - 1);  // Sign extend
            if (insn & (1 << 22)) {  // Load
                printf("LDR X%d, [X%d, #%d]\n", rt, rn, offset);
            } else {  // Store
                printf("STR X%d, [X%d, #%d]\n", rt, rn, offset);
            }
            fflush(stdout);
        } else if ((insn & 0xFFE00C00) == 0xF8400000) {
            uint8_t rt = insn & 0x1F;
            uint8_t rn = (insn >> 5) & 0x1F;
            int16_t offset = ((insn >> 12) & 0x1FF) | (((insn >> 26) & 1) << 8);
            if (offset & (1 << 8)) offset |= ~((1 << 9) - 1);  // Sign extend
            if (insn & (1 << 22)) {  // Load
                printf("LDR W%d, [X%d, #%d]\n", rt, rn, offset);
            } else {  // Store
                printf("STR W%d, [X%d, #%d]\n", rt, rn, offset);
            }
            fflush(stdout);
        /* Conditional branch */
        } else if ((insn & 0xFF000010) == 0x54000000) {
            uint8_t cond = (insn >> 4) & 0xF;
            int32_t offset = ((insn >> 5) & 0x7FFFF) | (((insn >> 24) & 1) << 18);
            if (offset & (1 << 18)) offset |= ~((1 << 19) - 1);  // Sign extend
            const char *conds[] = {"EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL", "NV"};
            printf("B.%s +0x%x\n", conds[cond], offset << 2);
            fflush(stdout);
        } else {
            printf("UNKNOWN (may cause segfault)\n");
            fprintf(stderr, "[EXEC WARNING] Unknown instruction 0x%08x at offset %zu\n", insn, i*4);
        }
    }

    /* Execute the translated ARM64 code */
    fprintf(stderr, "[EXEC BLOCK DEBUG] About to execute translated code\n");
    printf("[EXEC] ⚙️ Executing translated code...\n");

    /* Set up execution context for translated code */
    /* Allocate on heap for stability */
    rosetta_exec_context_t *exec_ctx = calloc(1, sizeof(rosetta_exec_context_t));
    if (!exec_ctx) {
        fprintf(stderr, "[EXEC] Failed to allocate execution context\n");
        return -1;
    }

    exec_ctx->guest_mem_base = ctx->memmgr->host_base;
    exec_ctx->guest_mem_size = ctx->memmgr->total_size;
    exec_ctx->state = ctx->state;

    printf("[EXEC] 📋 Execution context setup:\n");
    printf("[EXEC]   context ptr: %p\n", exec_ctx);
    printf("[EXEC]   guest_mem_base: %p\n", exec_ctx->guest_mem_base);
    printf("[EXEC]   guest_mem_size: 0x%lx\n", exec_ctx->guest_mem_size);
    printf("[EXEC]   state: %p\n", exec_ctx->state);

    /* Call translated code with X18 pointing to execution context */
    /* X18 is reserved as platform register in ARM64 ABI */
    printf("[EXEC] ⚙️ Calling translated code (X18 = %p)...\n", exec_ctx);

    /* Clear instruction cache - required before executing dynamically generated code on ARM64 */
    fprintf(stderr, "[EXEC DEBUG] Clearing instruction cache for %p (%zu bytes)\n",
            translated_code, code_size);
    __builtin___clear_cache((char *)translated_code, (char *)translated_code + code_size);
    fprintf(stderr, "[EXEC DEBUG] Instruction cache cleared\n");

    typedef void (*translated_func_t)(void);

    /* Set X18 to point to execution context, then call translated code */
    /* Use inline assembly to set X18 before the call */
    __asm__ volatile(
        "mov x18, %0\n"       /* Set X18 to execution context */
        "blr %1\n"            /* Call translated code */
        :
        : "r"(exec_ctx), "r"(translated_code)
        : "x18", "memory"
    );

    free(exec_ctx);

    fprintf(stderr, "[EXEC BLOCK DEBUG] Executed translated code\n");
    printf("[EXEC] ✅ Block execution complete\n");

    /* Update statistics */
    ctx->blocks_executed++;

    /* Estimate instructions executed */
    ctx->instructions_executed += 10;

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
    fprintf(stderr, "[EXEC DEBUG] rosetta_execute called: ctx=%p entry_point=%lx\n", ctx, entry_point);

    if (!ctx || !ctx->state || !ctx->memmgr) {
        fprintf(stderr, "[EXEC DEBUG] NULL parameter detected\n");
        return -1;
    }

    fprintf(stderr, "[EXEC DEBUG] Parameters validated\n");
    fprintf(stderr, "[EXEC DEBUG] About to print start message\n");

    printf("[EXEC] Starting execution at 0x%lx\n", entry_point);

    fprintf(stderr, "[EXEC DEBUG] Start message printed\n");

    /* Set running state */
    ctx->is_running = 1;
    ctx->guest_pc = entry_point;
    ctx->instructions_executed = 0;
    ctx->exit_code = 0;

    fprintf(stderr, "[EXEC DEBUG] State initialized\n");

    /* Main execution loop */
    const uint64_t max_insns = 10000;  /* Increased for more comprehensive testing */
    int ret = 0;

    fprintf(stderr, "[EXEC DEBUG] About to enter execution loop\n");

    while (ctx->is_running && ctx->instructions_executed < max_insns) {
        fprintf(stderr, "[EXEC DEBUG] Loop iteration: is_running=%d insns_executed=%lu\n",
                ctx->is_running, ctx->instructions_executed);

        /* Execute a basic block */
        fprintf(stderr, "[EXEC DEBUG] About to call rosetta_execute_block(pc=0x%lx)\n", ctx->guest_pc);
        ret = rosetta_execute_block(ctx, ctx->guest_pc);
        fprintf(stderr, "[EXEC DEBUG] rosetta_execute_block returned: %d\n", ret);

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
        if (ctx->instructions_executed >= 1000) {
            printf("[EXEC] Reached instruction limit (1000)\n");
            break;
        }
    }

    fprintf(stderr, "[EXEC DEBUG] Exit execution loop\n");

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
