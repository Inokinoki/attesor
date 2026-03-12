/* ============================================================================
 * Rosetta Binary Runner - Main Execution Entry Point
 * ============================================================================
 *
 * This module provides the main entry point for loading and executing
 * x86_64 ELF binaries using the Rosetta binary translator.
 *
 * Architecture: x86_64 guest binary -> ARM64 host execution
 * ============================================================================ */

#ifndef ROSETTA_RUNNER_H
#define ROSETTA_RUNNER_H

#include "rosetta_types.h"
#include "rosetta_elf_loader.h"
#include "rosetta_memmgr.h"
#include "rosetta_execute.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Runner Configuration
 * ============================================================================ */

/**
 * Rosetta runner configuration
 */
typedef struct {
    int verbose;              /* Verbose output flag */
    int debug;                /* Debug output flag */
    int trace_syscalls;        /* Trace syscall execution */
    int dump_blocks;          /* Dump translated blocks */
    uint64_t max_instructions; /* Max instructions to execute (0 = unlimited) */
    char *translator_path;    /* Path to translator binary */
    char *interpreter_path;   /* Path to dynamic linker (if needed) */
} rosetta_runner_config_t;

/**
 * Rosetta runner state
 */
typedef struct {
    /* Loaded binary information */
    rosetta_elf_binary_t *binary;        /* ELF binary */

    /* Translation state */
    void *translation_cache;              /* Translation cache */
    uint64_t entry_point;                  /* Entry point address */

    /* Memory management */
    rosetta_memmgr_t *memmgr;             /* Guest memory manager */

    /* Execution state */
    RosettaThreadState *thread_state;     /* Thread execution state */
    uint64_t instructions_executed;        /* Instruction counter */

    /* Memory management */
    void *guest_base;                      /* Guest memory base */
    size_t guest_size;                     /* Guest memory size */

    /* Status */
    int is_running;                        /* Currently executing */
    int exit_code;                         /* Program exit code */

    /* Configuration */
    rosetta_runner_config_t config;        /* Runner configuration */

} rosetta_runner_t;

/* ============================================================================
 * Runner API Functions
 * ============================================================================ */

/**
 * Create a new Rosetta runner instance
 * @param config Configuration options (can be NULL for defaults)
 * @return Runner instance, or NULL on error
 */
rosetta_runner_t *rosetta_runner_create(const rosetta_runner_config_t *config);

/**
 * Destroy a Rosetta runner instance
 * @param runner Runner instance to destroy
 */
void rosetta_runner_destroy(rosetta_runner_t *runner);

/**
 * Load an ELF binary for execution
 * @param runner Runner instance
 * @param filename Path to x86_64 ELF binary
 * @return 0 on success, -1 on error
 */
int rosetta_runner_load_binary(rosetta_runner_t *runner, const char *filename);

/**
 * Prepare the execution environment
 * @param runner Runner instance
 * @return 0 on success, -1 on error
 */
int rosetta_runner_prepare(rosetta_runner_t *runner);

/**
 * Execute the loaded binary
 * @param runner Runner instance
 * @param argc Argument count
 * @param argv Argument vector
 * @param envp Environment vector
 * @return Exit code of the program
 */
int rosetta_runner_execute(rosetta_runner_t *runner, int argc, char **argv, char **envp);

/**
 * Run a binary with simplified interface
 * @param filename Path to x86_64 ELF binary
 * @param argv Argument vector (first element is program name)
 * @return Exit code of the program
 */
int rosetta_run(const char *filename, char **argv);

/**
 * Stop a running program
 * @param runner Runner instance
 * @return 0 on success, -1 on error
 */
int rosetta_runner_stop(rosetta_runner_t *runner);

/**
 * Get runner status
 * @param runner Runner instance
 * @return Current status (0 = stopped, 1 = running)
 */
int rosetta_runner_status(rosetta_runner_t *runner);

/**
 * Get execution statistics
 * @param runner Runner instance
 * @param instructions_executed Output: number of instructions executed
 * @return 0 on success, -1 on error
 */
int rosetta_runner_get_stats(rosetta_runner_t *runner, uint64_t *instructions_executed);

/* ============================================================================
 * Convenience Functions
 * ============================================================================ */

/**
 * Get default runner configuration
 * @return Default configuration structure
 */
rosetta_runner_config_t rosetta_runner_default_config(void);

/**
 * Print runner statistics
 * @param runner Runner instance
 */
void rosetta_runner_print_stats(rosetta_runner_t *runner);

#endif /* ROSETTA_RUNNER_H */
