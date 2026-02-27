/* ============================================================================
 * Rosetta Refactored - Block Translation Implementation
 * ============================================================================
 *
 * This module implements block-level translation functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#include "rosetta_refactored_block.h"
#include "rosetta_refactored_helpers.h"
#include "rosetta_trans_cache.h"
#include "rosetta_arm64_decode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Block Cache State
 * ============================================================================ */

#define ROS_BLOCK_CACHE_BITS    10
#define ROS_BLOCK_CACHE_SIZE    (1 << ROS_BLOCK_CACHE_BITS)
#define ROS_BLOCK_CACHE_MASK    (ROS_BLOCK_CACHE_SIZE - 1)

static rosetta_block_t g_block_cache[ROS_BLOCK_CACHE_SIZE];
static bool g_block_valid[ROS_BLOCK_CACHE_SIZE];
static uint32_t g_block_hits = 0;
static uint32_t g_block_misses = 0;
static bool g_block_initialized = false;

/* ============================================================================
 * Block Analysis Functions
 * ============================================================================ */

/**
 * rosetta_block_is_branch - Check if instruction is a branch
 * @encoding: ARM64 instruction encoding
 * Returns: true if branch, false otherwise
 */
bool rosetta_block_is_branch(uint32_t encoding)
{
    /* B, BL: 0x14000000, 0x94000000 */
    if ((encoding & 0xFC000000) == 0x14000000 ||
        (encoding & 0xFC000000) == 0x94000000) {
        return true;
    }

    /* BR, BLR, RET: 0xD61F0000, 0xD63F0000, 0xD65F0000 */
    if ((encoding & 0xFFFF0000) == 0xD61F0000 ||
        (encoding & 0xFFFF0000) == 0xD63F0000 ||
        (encoding & 0xFFFF0000) == 0xD65F0000) {
        return true;
    }

    /* B.cond: 0x54000000 */
    if ((encoding & 0xFF000000) == 0x54000000) {
        return true;
    }

    /* CBZ, CBNZ: 0x34000000, 0x35000000 */
    if ((encoding & 0x7F800000) == 0x34000000 ||
        (encoding & 0x7F800000) == 0x35000000) {
        return true;
    }

    /* TBZ, TBNZ: 0x36000000, 0x37000000 */
    if ((encoding & 0x7F000000) == 0x36000000 ||
        (encoding & 0x7F000000) == 0x37000000) {
        return true;
    }

    return false;
}

/**
 * rosetta_block_is_terminator - Check if instruction terminates block
 * @encoding: ARM64 instruction encoding
 * Returns: true if terminator, false otherwise
 */
bool rosetta_block_is_terminator(uint32_t encoding)
{
    /* RET: always terminates */
    if ((encoding & 0xFFFFFC00) == 0xD65F0000) {
        return true;
    }

    /* BR: register branch - terminates */
    if ((encoding & 0xFFFFFC00) == 0xD61F0000) {
        return true;
    }

    /* SVC: system call - terminates block */
    if ((encoding & 0xFFE00000) == 0xD4000000) {
        return true;
    }

    /* HLT, BRK: halt/break - terminates */
    if ((encoding & 0xFFE00000) == 0xD4200000) {
        return true;
    }

    return false;
}

/**
 * rosetta_block_get_branch_target - Get branch target address
 * @encoding: ARM64 instruction encoding
 * @pc: Current program counter
 * Returns: Target address
 */
uint64_t rosetta_block_get_branch_target(uint32_t encoding, uint64_t pc)
{
    int32_t imm;

    /* B, BL: 26-bit immediate */
    if ((encoding & 0xFC000000) == 0x14000000 ||
        (encoding & 0xFC000000) == 0x94000000) {
        imm = (int32_t)((encoding & 0x03FFFFFF) << 6) >> 4;
        return pc + imm;
    }

    /* B.cond: 19-bit immediate */
    if ((encoding & 0xFF000000) == 0x54000000) {
        imm = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 11;
        return pc + imm;
    }

    /* CBZ, CBNZ: 19-bit immediate */
    if ((encoding & 0x7F800000) == 0x34000000 ||
        (encoding & 0x7F800000) == 0x35000000) {
        imm = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 11;
        return pc + imm;
    }

    /* TBZ, TBNZ: 14-bit immediate */
    if ((encoding & 0x7F000000) == 0x36000000 ||
        (encoding & 0x7F000000) == 0x37000000) {
        imm = (int32_t)((encoding & 0x00FFFFE0) << 3) >> 13;
        return pc + imm;
    }

    /* BR, BLR, RET: target is in register - can't determine statically */
    return 0;
}

/**
 * rosetta_block_analyze - Analyze basic block boundaries
 * @guest_pc: Guest program counter
 * @max_insns: Maximum instructions to analyze
 * Returns: Number of instructions in block
 */
int rosetta_block_analyze(uint64_t guest_pc, int max_insns)
{
    const uint32_t *insn_ptr = (const uint32_t *)(uintptr_t)guest_pc;
    int count = 0;

    if (max_insns > ROS_BLOCK_MAX_INSNS) {
        max_insns = ROS_BLOCK_MAX_INSNS;
    }

    while (count < max_insns) {
        uint32_t encoding = *insn_ptr++;

        count++;

        /* Check for block terminator */
        if (rosetta_block_is_terminator(encoding)) {
            break;
        }

        /* Stop at unconditional branches (they end the block) */
        if ((encoding & 0xFC000000) == 0x14000000) {
            break;
        }
    }

    return count;
}

/* ============================================================================
 * Block Cache Operations
 * ============================================================================ */

/**
 * block_cache_hash - Compute hash for guest PC
 * @pc: Guest program counter
 * Returns: Hash index
 */
static inline uint32_t block_cache_hash(uint64_t pc)
{
    /* Simple hash: fold 64-bit address to cache index */
    uint32_t hash = (uint32_t)(pc ^ (pc >> 32));
    hash = hash * 2654435761u;  /* Knuth's multiplicative hash */
    return hash & ROS_BLOCK_CACHE_MASK;
}

/**
 * rosetta_block_lookup - Look up translated block
 * @guest_pc: Guest program counter
 * Returns: Block descriptor or NULL
 */
rosetta_block_t *rosetta_block_lookup(uint64_t guest_pc)
{
    uint32_t hash = block_cache_hash(guest_pc);

    if (!g_block_initialized) {
        return NULL;
    }

    if (g_block_valid[hash] && g_block_cache[hash].guest_pc == guest_pc) {
        g_block_hits++;
        g_block_cache[hash].hit_count++;
        return &g_block_cache[hash];
    }

    g_block_misses++;
    return NULL;
}

/**
 * rosetta_block_insert - Insert translated block into cache
 * @guest_pc: Guest program counter
 * @host_code: Translated code pointer
 * @size: Translated code size
 * @insn_count: Number of instructions
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_insert(uint64_t guest_pc, void *host_code,
                         size_t size, int insn_count)
{
    uint32_t hash;

    if (!g_block_initialized) {
        return -1;
    }

    hash = block_cache_hash(guest_pc);

    g_block_cache[hash].guest_pc = guest_pc;
    g_block_cache[hash].host_pc = (uint64_t)(uintptr_t)host_code;
    g_block_cache[hash].guest_size = insn_count * 4;
    g_block_cache[hash].host_size = (uint32_t)size;
    g_block_cache[hash].insn_count = (uint16_t)insn_count;
    g_block_cache[hash].flags = ROS_BLOCK_FLAG_VALID | ROS_BLOCK_FLAG_CACHED;
    g_block_cache[hash].hash = hash;
    g_block_cache[hash].refcount = 1;
    g_block_cache[hash].hit_count = 0;

    g_block_valid[hash] = true;

    return 0;
}

/**
 * rosetta_block_remove - Remove block from cache
 * @guest_pc: Guest program counter
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_remove(uint64_t guest_pc)
{
    uint32_t hash = block_cache_hash(guest_pc);

    if (!g_block_initialized) {
        return -1;
    }

    if (g_block_valid[hash] && g_block_cache[hash].guest_pc == guest_pc) {
        g_block_valid[hash] = false;
        g_block_cache[hash].flags = 0;
        return 0;
    }

    return -1;
}

/**
 * rosetta_block_invalidate - Invalidate all cached blocks
 */
void rosetta_block_invalidate(void)
{
    if (!g_block_initialized) {
        return;
    }

    memset(g_block_valid, 0, sizeof(g_block_valid));
    memset(g_block_cache, 0, sizeof(g_block_cache));
}

/**
 * rosetta_block_get_info - Get block information
 * @guest_pc: Guest program counter
 * @block: Block descriptor to fill
 * Returns: 0 on success, -1 if not found
 */
int rosetta_block_get_info(uint64_t guest_pc, rosetta_block_t *block)
{
    rosetta_block_t *info;

    if (!block) {
        return -1;
    }

    info = rosetta_block_lookup(guest_pc);
    if (!info) {
        return -1;
    }

    *block = *info;
    return 0;
}

/**
 * rosetta_block_print - Print block information
 * @guest_pc: Guest program counter
 */
void rosetta_block_print(uint64_t guest_pc)
{
    rosetta_block_t block;

    if (rosetta_block_get_info(guest_pc, &block) == 0) {
        printf("Block at 0x%016llx:\n", (unsigned long long)guest_pc);
        printf("  Host PC:    0x%016llx\n", (unsigned long long)block.host_pc);
        printf("  Size:       %u bytes (guest), %u bytes (host)\n",
               block.guest_size, block.host_size);
        printf("  Insns:      %u\n", block.insn_count);
        printf("  Flags:      0x%04x\n", block.flags);
        printf("  Hits:       %u\n", block.hit_count);
    } else {
        printf("Block not found at 0x%016llx\n", (unsigned long long)guest_pc);
    }
}

/* ============================================================================
 * Block Translation
 * ============================================================================ */

/* External translation functions */
extern int translate_alu_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
extern int translate_compare_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state, uint64_t *pstate);
extern int translate_mov_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
extern int translate_mem_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
extern int translate_branch_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state,
                                     uint64_t pc, int *terminated);
extern int translate_system_dispatch(uint32_t encoding, code_buf_t *code_buf, uint64_t *guest_state);
extern void emit_x86_nop(code_buf_t *code_buf);
extern void emit_x86_ret(code_buf_t *code_buf);

/* External state access */
extern ThreadState *rosetta_get_state(void);
extern void *refactored_translation_cache_lookup(uint64_t guest_pc);
extern void *refactored_code_cache_alloc(size_t size);

/**
 * rosetta_block_translate - Translate a basic block
 * @guest_pc: Guest program counter
 * @max_insns: Maximum instructions to translate
 * Returns: Pointer to translated code, or NULL on error
 */
void *rosetta_block_translate(uint64_t guest_pc, int max_insns)
{
    const uint32_t *insn_ptr = (const uint32_t *)(uintptr_t)guest_pc;
    code_buf_t code_buf;
    static uint8_t code_cache[ROS_BLOCK_MAX_SIZE * 4];  /* x86 expansion buffer */
    int terminated = 0;
    int insn_count = 0;

    /* Check cache first */
    void *cached = refactored_translation_cache_lookup(guest_pc);
    if (cached) {
        return cached;
    }

    /* Limit max instructions */
    if (max_insns > ROS_BLOCK_MAX_INSNS) {
        max_insns = ROS_BLOCK_MAX_INSNS;
    }
    if (max_insns < ROS_BLOCK_MIN_INSNS) {
        max_insns = ROS_BLOCK_MIN_INSNS;
    }

    /* Initialize code buffer */
    code_buf_init(&code_buf, code_cache, sizeof(code_cache));

    /* Get thread state */
    ThreadState *state = rosetta_get_state();

    /* Analyze block to determine size */
    int block_size = rosetta_block_analyze(guest_pc, max_insns);
    if (block_size < 1) {
        block_size = 1;
    }

    /* Translate each instruction */
    while (insn_count < block_size && !terminated) {
        uint32_t encoding = *insn_ptr++;
        uint64_t current_pc = guest_pc + (insn_count * 4);

        /* Try modular dispatch - ALU instructions */
        if (translate_alu_dispatch(encoding, &code_buf, state->guest.x) == 0) {
            insn_count++;
            continue;
        }

        /* Try compare instructions */
        if (translate_compare_dispatch(encoding, &code_buf, state->guest.x,
                                       &state->guest.pstate) == 0) {
            insn_count++;
            continue;
        }

        /* Try MOV instructions */
        if (translate_mov_dispatch(encoding, &code_buf, state->guest.x) == 0) {
            insn_count++;
            continue;
        }

        /* Try memory instructions */
        if (translate_mem_dispatch(encoding, &code_buf, state->guest.x) == 0) {
            insn_count++;
            continue;
        }

        /* Try branch instructions */
        if (translate_branch_dispatch(encoding, &code_buf, state->guest.x,
                                      current_pc, &terminated) == 0) {
            insn_count++;
            continue;
        }

        /* Try system instructions */
        if (translate_system_dispatch(encoding, &code_buf, state->guest.x) == 0) {
            terminated = 1;
            insn_count++;
            continue;
        }

        /* Unknown/unimplemented instruction - emit NOP */
        emit_x86_nop(&code_buf);
        insn_count++;
    }

    /* Ensure block ends with RET if not terminated */
    if (!terminated) {
        emit_x86_ret(&code_buf);
    }

    /* Allocate permanent storage for translated code */
    size_t code_size = code_buf_get_size(&code_buf);
    void *perm_code = refactored_code_cache_alloc(code_size);
    if (!perm_code) {
        perm_code = malloc(code_size);
    }
    if (perm_code) {
        memcpy(perm_code, code_cache, code_size);
    }

    /* Insert into block cache */
    if (rosetta_block_insert(guest_pc, perm_code ? perm_code : code_cache,
                             code_size, insn_count) == 0) {
        /* Successfully cached */
    }

    return perm_code ? perm_code : code_cache;
}

/**
 * rosetta_block_translate_fast - Fast translation (cache lookup)
 * @guest_pc: Guest program counter
 * Returns: Cached translation or NULL
 */
void *rosetta_block_translate_fast(uint64_t guest_pc)
{
    rosetta_block_t *block = rosetta_block_lookup(guest_pc);
    if (block) {
        return (void *)(uintptr_t)block->host_pc;
    }
    return NULL;
}

/* ============================================================================
 * Cache Statistics
 * ============================================================================ */

/**
 * rosetta_block_cache_stats - Get cache statistics
 * @total_blocks: Total blocks cached
 * @total_hits: Total cache hits
 * @total_misses: Total cache misses
 */
void rosetta_block_cache_stats(uint32_t *total_blocks,
                               uint32_t *total_hits,
                               uint32_t *total_misses)
{
    uint32_t blocks = 0;
    size_t i;

    if (!g_block_initialized) {
        if (total_blocks) *total_blocks = 0;
        if (total_hits) *total_hits = 0;
        if (total_misses) *total_misses = 0;
        return;
    }

    for (i = 0; i < ROS_BLOCK_CACHE_SIZE; i++) {
        if (g_block_valid[i]) {
            blocks++;
        }
    }

    if (total_blocks) *total_blocks = blocks;
    if (total_hits) *total_hits = g_block_hits;
    if (total_misses) *total_misses = g_block_misses;
}

/**
 * rosetta_block_cache_hit_rate - Get cache hit rate
 * Returns: Hit rate as percentage (0.0 - 100.0)
 */
double rosetta_block_cache_hit_rate(void)
{
    uint32_t total = g_block_hits + g_block_misses;
    if (total == 0) {
        return 0.0;
    }
    return (double)g_block_hits / total * 100.0;
}

/**
 * rosetta_block_cache_count - Get number of cached blocks
 * Returns: Number of blocks in cache
 */
size_t rosetta_block_cache_count(void)
{
    size_t count = 0;
    size_t i;

    if (!g_block_initialized) {
        return 0;
    }

    for (i = 0; i < ROS_BLOCK_CACHE_SIZE; i++) {
        if (g_block_valid[i]) {
            count++;
        }
    }

    return count;
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * rosetta_block_init - Initialize block translation subsystem
 * Returns: 0 on success, -1 on error
 */
int rosetta_block_init(void)
{
    if (g_block_initialized) {
        rosetta_block_cleanup();
    }

    memset(g_block_valid, 0, sizeof(g_block_valid));
    memset(g_block_cache, 0, sizeof(g_block_cache));
    g_block_hits = 0;
    g_block_misses = 0;
    g_block_initialized = true;

    return 0;
}

/**
 * rosetta_block_cleanup - Cleanup block translation subsystem
 */
void rosetta_block_cleanup(void)
{
    g_block_initialized = false;
}
