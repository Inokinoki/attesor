/* ============================================================================
 * Rosetta 2 Performance Benchmark Suite
 * ============================================================================
 *
 * Comprehensive performance testing for the x86_64 → ARM64 translator
 * ============================================================================ */

#include "rosetta_types.h"
#include "rosetta_x86_decode.h"
#include "rosetta_elf_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

#define BENCHMARK_ITERATIONS 1000000

typedef struct {
    const char *name;
    double ops_per_sec;
    double latency_ns;
} benchmark_result_t;

static benchmark_result_t results[20];
static int result_count = 0;

/* Benchmark: Single instruction decoding */
double benchmark_single_instruction(void) {
    const uint8_t insn[] = {0x48, 0x89, 0xE5}; /* MOV RBP, RSP */
    
    clock_t start = clock();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        x86_insn_t decoded;
        decode_x86_insn(insn, &decoded);
    }
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return BENCHMARK_ITERATIONS / elapsed;
}

/* Benchmark: Complex instruction decoding */
double benchmark_complex_instructions(void) {
    const uint8_t complex_insns[][8] = {
        {0x48, 0x81, 0xC0, 0x01, 0x00, 0x00, 0x00}, /* ADD RAX, 1 */
        {0x48, 0x0F, 0xAF, 0xC0},                /* IMUL RAX, RAX */
        {0x48, 0x89, 0x84, 0x93, 0x10, 0x00, 0x00, 0x00} /* MOV RAX, [RBX+RDX*4+0x10] */
    };
    int num_insns = sizeof(complex_insns) / sizeof(complex_insns[0]);
    
    clock_t start = clock();
    for (int i = 0; i < BENCHMARK_ITERATIONS / 10; i++) {
        for (int j = 0; j < num_insns; j++) {
            x86_insn_t decoded;
            decode_x86_insn(complex_insns[j], &decoded);
        }
    }
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return (BENCHMARK_ITERATIONS / 10) / elapsed;
}

/* Benchmark: ELF loading */
double benchmark_elf_loading(void) {
    clock_t start = clock();
    for (int i = 0; i < 100; i++) {
        rosetta_elf_binary_t *binary = NULL;
        if (rosetta_elf_load("simple_x86_pure.x86_64", &binary) == 0) {
            rosetta_elf_unload(binary);
        }
    }
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return 100 / elapsed;
}

/* Benchmark: Symbol lookup */
double benchmark_symbol_lookup(void) {
    rosetta_elf_binary_t *binary = NULL;
    rosetta_elf_load("simple_x86_pure.x86_64", &binary);
    
    if (binary == NULL) return 0;
    
    clock_t start = clock();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        rosetta_elf_lookup_symbol(binary, "main");
        rosetta_elf_lookup_symbol(binary, "printf");
        rosetta_elf_lookup_symbol(binary, "malloc");
    }
    clock_t end = clock();
    
    rosetta_elf_unload(binary);
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return (BENCHMARK_ITERATIONS * 3) / elapsed;
}

/* Benchmark: /proc operations */
double benchmark_proc_operations(void) {
    clock_t start = clock();
    for (int i = 0; i < BENCHMARK_ITERATIONS / 100; i++) {
        rosetta_proc_get_path_type("/proc/cpuinfo");
        rosetta_proc_get_path_type("/proc/self/auxv");
        rosetta_proc_get_path_type("/proc/self/status");
    }
    clock_t end = clock();
    
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    return (BENCHMARK_ITERATIONS / 100) / elapsed;
}

/* Run benchmark and record result */
void run_benchmark(const char *name, double (*benchmark_func)(void)) {
    printf("Benchmarking: %-30s ... ", name);
    fflush(stdout);
    
    double throughput = benchmark_func();
    double latency = 1e9 / throughput;
    
    results[result_count].name = name;
    results[result_count].ops_per_sec = throughput;
    results[result_count].latency_ns = latency;
    result_count++;
    
    printf("%.2f M ops/sec (%.2f ns/op)\n", throughput / 1e6, latency);
}

int main() {
    printf("=================================================================\n");
    printf("Rosetta 2 Performance Benchmark Suite\n");
    printf("=================================================================\n\n");
    
    printf("Running comprehensive performance tests (%d iterations per test):\n\n", BENCHMARK_ITERATIONS);
    
    /* Run benchmarks */
    run_benchmark("Single instruction decode", benchmark_single_instruction);
    run_benchmark("Complex instruction decode", benchmark_complex_instructions);
    run_benchmark("ELF binary loading", benchmark_elf_loading);
    run_benchmark("Symbol lookup", benchmark_symbol_lookup);
    run_benchmark("/proc operations", benchmark_proc_operations);
    
    /* Print summary */
    printf("\n=================================================================\n");
    printf("Performance Summary:\n");
    printf("=================================================================\n");
    
    double avg_throughput = 0;
    for (int i = 0; i < result_count; i++) {
        printf("  %-30s %10.2f M ops/sec  %8.2f ns/op\n",
               results[i].name,
               results[i].ops_per_sec / 1e6,
               results[i].latency_ns);
        avg_throughput += results[i].ops_per_sec;
    }
    avg_throughput /= result_count;
    
    printf("  %-30s %10.2f M ops/sec  %8.2f ns/op\n",
           "AVERAGE", avg_throughput / 1e6, 1e9 / avg_throughput);
    
    printf("\nPerformance Class: ");
    if (avg_throughput > 50e6) {
        printf("EXCELLENT (>50 M ops/sec)\n");
    } else if (avg_throughput > 20e6) {
        printf("GOOD (>20 M ops/sec)\n");
    } else if (avg_throughput > 10e6) {
        printf("ACCEPTABLE (>10 M ops/sec)\n");
    } else {
        printf("NEEDS OPTIMIZATION\n");
    }
    
    printf("=================================================================\n");
    
    return 0;
}
