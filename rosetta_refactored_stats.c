/* ============================================================================
 * Rosetta Refactored - Statistics Implementation
 * ============================================================================
 *
 * This module implements statistics collection and reporting for the
 * Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_refactored_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* ============================================================================
 * Statistics State
 * ============================================================================ */

static rosetta_stats_t g_stats;
static rosetta_block_stats_t g_block_stats[ROS_STATS_MAX_BLOCKS];
static size_t g_block_count = 0;
static bool g_stats_initialized = false;

/* Histogram data */
static uint32_t g_insn_size_hist[ROS_STATS_HIST_BUCKETS];
static uint32_t g_block_size_hist[ROS_STATS_HIST_BUCKETS];

/* Timing */
static uint64_t g_start_time_us = 0;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ============================================================================
 * Statistics API Implementation
 * ============================================================================ */

/**
 * rosetta_stats_init - Initialize statistics subsystem
 * Returns: 0 on success, -1 on error
 */
int rosetta_stats_init(void)
{
    if (g_stats_initialized) {
        rosetta_stats_cleanup();
    }

    memset(&g_stats, 0, sizeof(g_stats));
    memset(g_block_stats, 0, sizeof(g_block_stats));
    memset(g_insn_size_hist, 0, sizeof(g_insn_size_hist));
    memset(g_block_size_hist, 0, sizeof(g_block_size_hist));

    g_block_count = 0;
    g_start_time_us = get_time_us();
    g_stats_initialized = true;

    return 0;
}

/**
 * rosetta_stats_cleanup - Cleanup statistics subsystem
 */
void rosetta_stats_cleanup(void)
{
    g_stats_initialized = false;
}

/**
 * rosetta_stats_reset - Reset all statistics
 */
void rosetta_stats_reset(void)
{
    memset(&g_stats, 0, sizeof(g_stats));
    memset(g_block_stats, 0, sizeof(g_block_stats));
    memset(g_insn_size_hist, 0, sizeof(g_insn_size_hist));
    memset(g_block_size_hist, 0, sizeof(g_block_size_hist));
    g_block_count = 0;
    g_start_time_us = get_time_us();
}

/**
 * rosetta_stats_get - Get current statistics
 * @stats: Pointer to statistics structure
 */
void rosetta_stats_get(rosetta_stats_t *stats)
{
    if (stats) {
        *stats = g_stats;
    }
}

/**
 * rosetta_stats_print - Print statistics
 * @verbose: Print detailed breakdown
 */
void rosetta_stats_print(bool verbose)
{
    printf("\n");
    printf("=== Rosetta Translation Statistics ===\n\n");

    /* Block statistics */
    printf("Block Translation:\n");
    printf("  Blocks translated:  %llu\n", (unsigned long long)g_stats.blocks_translated);
    printf("  Blocks cached:      %llu\n", (unsigned long long)g_stats.blocks_cached);
    printf("  Blocks evicted:     %llu\n", (unsigned long long)g_stats.blocks_evicted);
    printf("\n");

    /* Cache statistics */
    printf("Cache Statistics:\n");
    printf("  Cache lookups:      %llu\n", (unsigned long long)g_stats.cache_lookups);
    printf("  Cache hits:         %llu\n", (unsigned long long)g_stats.cache_hits);
    printf("  Cache misses:       %llu\n", (unsigned long long)g_stats.cache_misses);
    printf("  Cache inserts:      %llu\n", (unsigned long long)g_stats.cache_inserts);
    printf("  Cache flushes:      %llu\n", (unsigned long long)g_stats.cache_flushes);

    if (g_stats.cache_lookups > 0) {
        double hit_rate = (double)g_stats.cache_hits / g_stats.cache_lookups * 100.0;
        printf("  Hit rate:           %.2f%%\n", hit_rate);
    }
    printf("\n");

    /* Instruction statistics */
    printf("Instruction Statistics:\n");
    printf("  Total instructions: %llu\n", (unsigned long long)g_stats.insns_total);

    if (verbose) {
        printf("\n  ALU Instructions:   %llu\n", (unsigned long long)g_stats.insns_alu);
        printf("    ADD/SUB:          %llu\n",
               (unsigned long long)(g_stats.insns_alu_add + g_stats.insns_alu_sub));
        printf("    AND/ORR/EOR:      %llu\n",
               (unsigned long long)(g_stats.insns_alu_and + g_stats.insns_alu_orr + g_stats.insns_alu_eor));
        printf("    MVN:              %llu\n", (unsigned long long)g_stats.insns_alu_mvn);
        printf("    MUL:              %llu\n", (unsigned long long)g_stats.insns_alu_mul);
        printf("    DIV:              %llu\n", (unsigned long long)g_stats.insns_alu_div);
        printf("    Shift:            %llu\n", (unsigned long long)g_stats.insns_alu_shift);

        printf("\n  Memory Instructions: %llu\n", (unsigned long long)g_stats.insns_mem);
        printf("    LDR:              %llu\n", (unsigned long long)g_stats.insns_mem_ldr);
        printf("    STR:              %llu\n", (unsigned long long)g_stats.insns_mem_str);
        printf("    LDP:              %llu\n", (unsigned long long)g_stats.insns_mem_ldp);
        printf("    STP:              %llu\n", (unsigned long long)g_stats.insns_mem_stp);

        printf("\n  Branch Instructions: %llu\n", (unsigned long long)g_stats.insns_branch);
        printf("    B (unconditional): %llu\n", (unsigned long long)g_stats.insns_branch_b);
        printf("    BL (with link):   %llu\n", (unsigned long long)g_stats.insns_branch_bl);
        printf("    BR (register):    %llu\n", (unsigned long long)g_stats.insns_branch_br);
        printf("    RET (return):     %llu\n", (unsigned long long)g_stats.insns_branch_ret);
        printf("    B.cond (conditional): %llu\n", (unsigned long long)g_stats.insns_branch_cond);
        printf("    CBZ/CBNZ:         %llu\n",
               (unsigned long long)(g_stats.insns_branch_cbz + g_stats.insns_branch_cbnz));

        printf("\n  MOV Instructions:   %llu\n", (unsigned long long)g_stats.insns_mov);
        printf("  Compare:            %llu\n", (unsigned long long)g_stats.insns_compare);
        printf("  System:             %llu\n", (unsigned long long)g_stats.insns_system);
        printf("  NEON/SIMD:          %llu\n", (unsigned long long)g_stats.insns_neon);
        printf("  Unknown:            %llu\n", (unsigned long long)g_stats.insns_unknown);
    }
    printf("\n");

    /* Code size statistics */
    printf("Code Size Statistics:\n");
    printf("  Total code size:    %llu bytes\n", (unsigned long long)g_stats.code_size_total);
    printf("  ARM64 input:        %llu bytes\n", (unsigned long long)g_stats.code_size_arm64);
    printf("  x86_64 output:      %llu bytes\n", (unsigned long long)g_stats.code_size_x86);
    printf("  Peak memory:        %llu bytes\n", (unsigned long long)g_stats.code_size_peak);

    if (g_stats.code_size_arm64 > 0) {
        double expansion = (double)g_stats.code_size_x86 / g_stats.code_size_arm64;
        printf("  Expansion ratio:    %.2fx\n", expansion);
    }
    printf("\n");

    /* Performance statistics */
    printf("Performance Statistics:\n");
    printf("  Guest cycles:       %llu\n", (unsigned long long)g_stats.cycles_guest);
    printf("  Host cycles:        %llu\n", (unsigned long long)g_stats.cycles_host);
    printf("  Translation time:   %llu us\n", (unsigned long long)g_stats.translations_time_us);
    printf("  Execution time:     %llu us\n", (unsigned long long)g_stats.execution_time_us);

    if (g_stats.translations_time_us > 0) {
        double tput = (double)g_stats.insns_total / (g_stats.translations_time_us / 1000000.0);
        printf("  Translation throughput: %.0f insns/sec\n", tput);
    }
    printf("\n");

    /* Error statistics */
    printf("Error Statistics:\n");
    printf("  Translation errors: %llu\n", (unsigned long long)g_stats.errors_translation);
    printf("  Execution errors:   %llu\n", (unsigned long long)g_stats.errors_execution);
    printf("  Memory errors:      %llu\n", (unsigned long long)g_stats.errors_memory);
    printf("\n");

    /* Block statistics */
    if (verbose && g_block_count > 0) {
        printf("Tracked Blocks: %zu\n", g_block_count);

        /* Find and print hottest blocks */
        printf("\nTop 5 Hottest Blocks:\n");
        rosetta_block_stats_t hot[5];
        size_t count = rosetta_stats_get_hot_blocks(hot, 5);
        for (size_t i = 0; i < count; i++) {
            printf("  0x%016llx: %u hits, %u insns, %u bytes\n",
                   (unsigned long long)hot[i].guest_pc,
                   hot[i].hit_count,
                   hot[i].insn_count,
                   hot[i].x86_size);
        }
    }

    printf("=================================\n");
}

/**
 * rosetta_stats_export_json - Export statistics to JSON
 * @buf: Output buffer
 * @buf_size: Buffer size
 * Returns: Bytes written
 */
int rosetta_stats_export_json(char *buf, size_t buf_size)
{
    double hit_rate = g_stats.cache_lookups > 0 ?
        (double)g_stats.cache_hits / g_stats.cache_lookups * 100.0 : 0.0;

    return snprintf(buf, buf_size,
        "{"
        "\"blocks_translated\":%llu,"
        "\"cache_lookups\":%llu,"
        "\"cache_hits\":%llu,"
        "\"cache_misses\":%llu,"
        "\"cache_hit_rate\":%.2f,"
        "\"insns_total\":%llu,"
        "\"insns_alu\":%llu,"
        "\"insns_mem\":%llu,"
        "\"insns_branch\":%llu,"
        "\"code_size_total\":%llu,"
        "\"expansion_ratio\":%.2f,"
        "\"errors_total\":%llu"
        "}",
        (unsigned long long)g_stats.blocks_translated,
        (unsigned long long)g_stats.cache_lookups,
        (unsigned long long)g_stats.cache_hits,
        (unsigned long long)g_stats.cache_misses,
        hit_rate,
        (unsigned long long)g_stats.insns_total,
        (unsigned long long)g_stats.insns_alu,
        (unsigned long long)g_stats.insns_mem,
        (unsigned long long)g_stats.insns_branch,
        (unsigned long long)g_stats.code_size_total,
        g_stats.code_size_arm64 > 0 ? (double)g_stats.code_size_x86 / g_stats.code_size_arm64 : 0.0,
        (unsigned long long)(g_stats.errors_translation + g_stats.errors_execution + g_stats.errors_memory));
}

/* ============================================================================
 * Statistics Recording Implementation
 * ============================================================================ */

/**
 * rosetta_stats_record_block - Record block translation
 */
void rosetta_stats_record_block(uint64_t guest_pc, uint64_t host_pc,
                                 uint32_t arm64_size, uint32_t x86_size,
                                 int insn_count)
{
    g_stats.blocks_translated++;
    g_stats.blocks_cached++;
    g_stats.code_size_total += x86_size;
    g_stats.code_size_arm64 += arm64_size;
    g_stats.code_size_x86 += x86_size;

    if (g_stats.code_size_total > g_stats.code_size_peak) {
        g_stats.code_size_peak = g_stats.code_size_total;
    }

    /* Record block statistics */
    if (g_block_count < ROS_STATS_MAX_BLOCKS) {
        rosetta_block_stats_t *bs = &g_block_stats[g_block_count++];
        bs->guest_pc = guest_pc;
        bs->host_pc = host_pc;
        bs->arm64_size = arm64_size;
        bs->x86_size = x86_size;
        bs->insn_count = (uint8_t)insn_count;
        bs->flags = ROS_BLOCK_VALID | ROS_BLOCK_CACHED;
        bs->hit_count = 0;
    }

    /* Update histograms */
    if (insn_count > 0 && insn_count <= ROS_STATS_HIST_BUCKETS) {
        g_block_size_hist[insn_count - 1]++;
    } else if (insn_count > ROS_STATS_HIST_BUCKETS) {
        g_block_size_hist[ROS_STATS_HIST_BUCKETS - 1]++;
    }
}

/**
 * rosetta_stats_record_cache_hit - Record cache hit
 */
void rosetta_stats_record_cache_hit(void)
{
    g_stats.cache_lookups++;
    g_stats.cache_hits++;
}

/**
 * rosetta_stats_record_cache_miss - Record cache miss
 */
void rosetta_stats_record_cache_miss(void)
{
    g_stats.cache_lookups++;
    g_stats.cache_misses++;
}

/**
 * rosetta_stats_record_insn - Record instruction translation
 * @type: Instruction type
 */
void rosetta_stats_record_insn(const char *type)
{
    g_stats.insns_total++;

    if (strcmp(type, "ALU") == 0) {
        g_stats.insns_alu++;
    } else if (strcmp(type, "MEM") == 0) {
        g_stats.insns_mem++;
    } else if (strcmp(type, "BRANCH") == 0) {
        g_stats.insns_branch++;
    } else if (strcmp(type, "MOV") == 0) {
        g_stats.insns_mov++;
    } else if (strcmp(type, "COMPARE") == 0) {
        g_stats.insns_compare++;
    } else if (strcmp(type, "SYSTEM") == 0) {
        g_stats.insns_system++;
    } else if (strcmp(type, "NEON") == 0) {
        g_stats.insns_neon++;
    } else {
        g_stats.insns_unknown++;
    }
}

/**
 * rosetta_stats_record_alu - Record ALU instruction
 * @subtype: ALU subtype
 */
void rosetta_stats_record_alu(const char *subtype)
{
    g_stats.insns_alu++;

    if (strcmp(subtype, "add") == 0) {
        g_stats.insns_alu_add++;
    } else if (strcmp(subtype, "sub") == 0) {
        g_stats.insns_alu_sub++;
    } else if (strcmp(subtype, "and") == 0) {
        g_stats.insns_alu_and++;
    } else if (strcmp(subtype, "orr") == 0) {
        g_stats.insns_alu_orr++;
    } else if (strcmp(subtype, "eor") == 0) {
        g_stats.insns_alu_eor++;
    } else if (strcmp(subtype, "mvn") == 0) {
        g_stats.insns_alu_mvn++;
    } else if (strcmp(subtype, "mul") == 0) {
        g_stats.insns_alu_mul++;
    } else if (strcmp(subtype, "div") == 0) {
        g_stats.insns_alu_div++;
    } else if (strcmp(subtype, "shift") == 0) {
        g_stats.insns_alu_shift++;
    }
}

/**
 * rosetta_stats_record_mem - Record memory instruction
 * @subtype: Memory subtype
 */
void rosetta_stats_record_mem(const char *subtype)
{
    g_stats.insns_mem++;

    if (strcmp(subtype, "ldr") == 0) {
        g_stats.insns_mem_ldr++;
    } else if (strcmp(subtype, "str") == 0) {
        g_stats.insns_mem_str++;
    } else if (strcmp(subtype, "ldp") == 0) {
        g_stats.insns_mem_ldp++;
    } else if (strcmp(subtype, "stp") == 0) {
        g_stats.insns_mem_stp++;
    }
}

/**
 * rosetta_stats_record_branch - Record branch instruction
 * @subtype: Branch subtype
 */
void rosetta_stats_record_branch(const char *subtype)
{
    g_stats.insns_branch++;

    if (strcmp(subtype, "b") == 0) {
        g_stats.insns_branch_b++;
    } else if (strcmp(subtype, "bl") == 0) {
        g_stats.insns_branch_bl++;
    } else if (strcmp(subtype, "br") == 0) {
        g_stats.insns_branch_br++;
    } else if (strcmp(subtype, "ret") == 0) {
        g_stats.insns_branch_ret++;
    } else if (strcmp(subtype, "cond") == 0) {
        g_stats.insns_branch_cond++;
    } else if (strcmp(subtype, "cbz") == 0) {
        g_stats.insns_branch_cbz++;
    } else if (strcmp(subtype, "cbnz") == 0) {
        g_stats.insns_branch_cbnz++;
    }
}

/**
 * rosetta_stats_record_error - Record error
 * @error_code: Error code
 */
void rosetta_stats_record_error(const char *error_code)
{
    if (strcmp(error_code, "translation") == 0) {
        g_stats.errors_translation++;
    } else if (strcmp(error_code, "execution") == 0) {
        g_stats.errors_execution++;
    } else if (strcmp(error_code, "memory") == 0) {
        g_stats.errors_memory++;
    }
}

/**
 * rosetta_stats_record_execution_time - Record execution time
 * @time_us: Time in microseconds
 */
void rosetta_stats_record_execution_time(uint64_t time_us)
{
    g_stats.execution_time_us += time_us;
}

/**
 * rosetta_stats_record_translation_time - Record translation time
 * @time_us: Time in microseconds
 */
void rosetta_stats_record_translation_time(uint64_t time_us)
{
    g_stats.translations_time_us += time_us;
}

/* ============================================================================
 * Block Statistics Implementation
 * ============================================================================ */

/**
 * rosetta_stats_get_block - Get block statistics
 * @index: Block index
 * @stats: Output structure
 * Returns: 0 on success, -1 on error
 */
int rosetta_stats_get_block(size_t index, rosetta_block_stats_t *stats)
{
    if (index >= g_block_count || !stats) {
        return -1;
    }
    *stats = g_block_stats[index];
    return 0;
}

/**
 * rosetta_stats_get_block_count - Get number of tracked blocks
 * Returns: Block count
 */
size_t rosetta_stats_get_block_count(void)
{
    return g_block_count;
}

/**
 * rosetta_stats_get_hot_blocks - Find hottest blocks
 * @blocks: Output array
 * @count: Number to find
 * Returns: Number found
 */
size_t rosetta_stats_get_hot_blocks(rosetta_block_stats_t *blocks, size_t count)
{
    if (!blocks || count == 0 || g_block_count == 0) {
        return 0;
    }

    /* Simple bubble sort to find top N */
    rosetta_block_stats_t temp[ROS_STATS_MAX_BLOCKS];
    memcpy(temp, g_block_stats, sizeof(g_block_stats));

    for (size_t i = 0; i < g_block_count && i < count; i++) {
        size_t max_idx = i;
        for (size_t j = i + 1; j < g_block_count; j++) {
            if (temp[j].hit_count > temp[max_idx].hit_count) {
                max_idx = j;
            }
        }
        if (max_idx != i) {
            rosetta_block_stats_t t = temp[i];
            temp[i] = temp[max_idx];
            temp[max_idx] = t;
        }
        blocks[i] = temp[i];
    }

    return (count < g_block_count) ? count : g_block_count;
}

/**
 * rosetta_stats_get_cold_blocks - Find coldest blocks
 * @blocks: Output array
 * @count: Number to find
 * Returns: Number found
 */
size_t rosetta_stats_get_cold_blocks(rosetta_block_stats_t *blocks, size_t count)
{
    if (!blocks || count == 0 || g_block_count == 0) {
        return 0;
    }

    rosetta_block_stats_t temp[ROS_STATS_MAX_BLOCKS];
    memcpy(temp, g_block_stats, sizeof(g_block_stats));

    for (size_t i = 0; i < g_block_count && i < count; i++) {
        size_t min_idx = i;
        for (size_t j = i + 1; j < g_block_count; j++) {
            if (temp[j].hit_count < temp[min_idx].hit_count) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            rosetta_block_stats_t t = temp[i];
            temp[i] = temp[min_idx];
            temp[min_idx] = t;
        }
        blocks[i] = temp[i];
    }

    return (count < g_block_count) ? count : g_block_count;
}

/* ============================================================================
 * Histogram and Profiling Implementation
 * ============================================================================ */

/**
 * rosetta_stats_get_insn_size_histogram - Get instruction size histogram
 * @hist: Output array
 * @count: Number of buckets
 */
void rosetta_stats_get_insn_size_histogram(uint32_t *hist, size_t count)
{
    if (!hist || count == 0) {
        return;
    }

    size_t copy = (count < ROS_STATS_HIST_BUCKETS) ? count : ROS_STATS_HIST_BUCKETS;
    memcpy(hist, g_insn_size_hist, copy * sizeof(uint32_t));
}

/**
 * rosetta_stats_get_block_size_histogram - Get block size histogram
 * @hist: Output array
 * @count: Number of buckets
 */
void rosetta_stats_get_block_size_histogram(uint32_t *hist, size_t count)
{
    if (!hist || count == 0) {
        return;
    }

    size_t copy = (count < ROS_STATS_HIST_BUCKETS) ? count : ROS_STATS_HIST_BUCKETS;
    memcpy(hist, g_block_size_hist, copy * sizeof(uint32_t));
}

/**
 * rosetta_stats_get_cache_hit_rate - Get cache hit rate
 * Returns: Hit rate percentage
 */
double rosetta_stats_get_cache_hit_rate(void)
{
    if (g_stats.cache_lookups == 0) {
        return 0.0;
    }
    return (double)g_stats.cache_hits / g_stats.cache_lookups * 100.0;
}

/**
 * rosetta_stats_get_avg_block_size - Get average block size
 * Returns: Average block size
 */
double rosetta_stats_get_avg_block_size(void)
{
    if (g_stats.blocks_translated == 0) {
        return 0.0;
    }
    return (double)g_stats.insns_total / g_stats.blocks_translated;
}

/**
 * rosetta_stats_get_expansion_ratio - Get expansion ratio
 * Returns: x86 bytes per ARM64 byte
 */
double rosetta_stats_get_expansion_ratio(void)
{
    if (g_stats.code_size_arm64 == 0) {
        return 0.0;
    }
    return (double)g_stats.code_size_x86 / g_stats.code_size_arm64;
}

/* ============================================================================
 * Local Statistics Wrappers (for backward compatibility)
 * ============================================================================ */

/* Local statistics state */
static uint64_t g_local_stats_translations = 0;
static uint64_t g_local_stats_cache_hits = 0;
static uint64_t g_local_stats_cache_misses = 0;
static uint64_t g_local_stats_total_insns = 0;
static uint64_t g_local_stats_alu = 0;
static uint64_t g_local_stats_mem = 0;
static uint64_t g_local_stats_branch = 0;
static uint64_t g_local_stats_system = 0;

/**
 * rosetta_stats_get_local - Get local statistics
 * @translations: Output translations count
 * @cache_hits: Output cache hits
 * @cache_misses: Output cache misses
 * @total_insns: Output total instructions
 * @alu: Output ALU instructions
 * @mem: Output memory instructions
 * @branch: Output branch instructions
 * @system: Output system instructions
 */
void rosetta_stats_get_local(uint64_t *translations, uint64_t *cache_hits,
                              uint64_t *cache_misses, uint64_t *total_insns,
                              uint64_t *alu, uint64_t *mem,
                              uint64_t *branch, uint64_t *system)
{
    if (translations) *translations = g_local_stats_translations;
    if (cache_hits) *cache_hits = g_local_stats_cache_hits;
    if (cache_misses) *cache_misses = g_local_stats_cache_misses;
    if (total_insns) *total_insns = g_local_stats_total_insns;
    if (alu) *alu = g_local_stats_alu;
    if (mem) *mem = g_local_stats_mem;
    if (branch) *branch = g_local_stats_branch;
    if (system) *system = g_local_stats_system;
}

/**
 * rosetta_stats_reset_local - Reset local statistics
 */
void rosetta_stats_reset_local(void)
{
    g_local_stats_translations = 0;
    g_local_stats_cache_hits = 0;
    g_local_stats_cache_misses = 0;
    g_local_stats_total_insns = 0;
    g_local_stats_alu = 0;
    g_local_stats_mem = 0;
    g_local_stats_branch = 0;
    g_local_stats_system = 0;
}

/**
 * rosetta_stats_record_local_translation - Record local translation
 */
void rosetta_stats_record_local_translation(void)
{
    g_local_stats_translations++;
}

/**
 * rosetta_stats_record_local_cache_hit - Record local cache hit
 */
void rosetta_stats_record_local_cache_hit(void)
{
    g_local_stats_cache_hits++;
}

/**
 * rosetta_stats_record_local_cache_miss - Record local cache miss
 */
void rosetta_stats_record_local_cache_miss(void)
{
    g_local_stats_cache_misses++;
}

/**
 * rosetta_stats_record_local_insn - Record local instruction
 * @type: Instruction type
 */
void rosetta_stats_record_local_insn(const char *type)
{
    g_local_stats_total_insns++;

    if (strcmp(type, "ALU") == 0) {
        g_local_stats_alu++;
    } else if (strcmp(type, "MEM") == 0) {
        g_local_stats_mem++;
    } else if (strcmp(type, "BRANCH") == 0) {
        g_local_stats_branch++;
    } else if (strcmp(type, "SYSTEM") == 0) {
        g_local_stats_system++;
    }
}
