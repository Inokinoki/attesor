/*=============================================================================
 * Rosetta Performance Optimizations
 *=============================================================================
 *
 * This header contains performance optimization macros and utilities
 * for the Rosetta 2 binary translator.
 *
 * Optimizations:
 * 1. Branch Prediction Hints (likely/unlikely)
 * 2. Cache Line Alignment
 * 3. Memory Access Patterns
 * 4. Inline Hints
 *
 *=============================================================================*/

#ifndef ROSETTA_OPTIMIZATIONS_H
#define ROSETTA_OPTIMIZATIONS_H

#include <stdint.h>

/*=============================================================================
 * Branch Prediction Optimization
 *=============================================================================*/

/**
 * Branch prediction hints using GCC built-ins
 * These help the CPU's branch predictor make better decisions
 *
 * Usage:
 *   if (LIKELY(condition)) {
 *       // Fast path
 *   } else if (UNLIKELY(rare_condition)) {
 *       // Error path
 *   }
 */

#ifdef __GNUC__
# define LIKELY(x)   __builtin_expect(!!(x), 1)
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define LIKELY(x)   (x)
# define UNLIKELY(x) (x)
#endif

/*=============================================================================
 * Cache Line Optimization
 *=============================================================================*/

/**
 * Cache line size for x86_64 and ARM64
 * Both architectures typically use 64-byte cache lines
 */
#define CACHE_LINE_SIZE 64

/**
 * Align structures to cache line boundaries
 * Prevents false sharing and improves cache utilization
 */
#define CACHE_LINE_ALIGN __attribute__((aligned(CACHE_LINE_SIZE)))

/**
 * Prevent false sharing by padding to cache line size
 * Use for frequently accessed data structures
 */
#define CACHE_PAD_TO_LINE \
    char _pad[CACHE_LINE_SIZE - (sizeof(void*))]

/*=============================================================================
 * Memory Access Optimization
 *=============================================================================*/

/**
 * Prefetch hint for memory access
 * Tells the CPU to fetch data into cache before use
 */
#ifdef __GNUC__
# define PREFETCH_READ(addr) __builtin_prefetch(addr, 0, 3)
# define PREFETCH_WRITE(addr) __builtin_prefetch(addr, 1, 3)
#else
# define PREFETCH_READ(addr) ((void)0)
# define PREFETCH_WRITE(addr) ((void)0)
#endif

/**
 * Restrict pointer aliasing hint
 * Helps compiler optimize memory access
 */
#define RESTRICT __restrict__

/*=============================================================================
 * Inline Optimization
 *=============================================================================*/

/**
 * Inline hints for critical functions
 * Use for small, frequently called functions
 */
#ifdef __GNUC__
# define FORCE_INLINE __attribute__((always_inline)) inline
# define NO_INLINE __attribute__((noinline))
#else
# define FORCE_INLINE inline
# define NO_INLINE
#endif

/*=============================================================================
 * Loop Optimization
 *=============================================================================*/

/**
 * Hint that loop is likely to iterate many times
 * Helps with loop unrolling and vectorization
 */
#ifdef __GNUC__
# define ASSUME_ALIGNED(ptr, align) \
    ((ptr) = (__typeof__(ptr))__builtin_assume_aligned((ptr), (align)))
#else
# define ASSUME_ALIGNED(ptr, align) ((void)0)
#endif

/*=============================================================================
 * Hot Path Marking
 *=============================================================================*/

/**
 * Mark frequently executed code paths
 * These will get more optimization attention from the compiler
 */
#define HOT_PATH
#define COLD_PATH __attribute__((cold))

/**
 * Mark error paths as cold
 * Error handling is rarely executed, so optimize for the common case
 */
#define ERROR_PATH COLD_PATH

/*=============================================================================
 * Structure Packing
 *=============================================================================*/

/**
 * Pack structures tightly to save memory
 * Use carefully - can impact performance due to misalignment
 */
#define PACKED __attribute__((packed))

/**
 * Add padding to align to next power-of-two boundary
 * Useful for avoiding cache line splits
 */
#define ALIGN_TO(type, alignment) \
    __attribute__((aligned(alignment)))

/*=============================================================================
 * Optimization Validation
 *=============================================================================*/

/**
 * Compile-time assertion for optimization assumptions
 */
#define OPTIM_ASSERT(expr, msg) \
    typedef char OPTIM_ASSERT_##msg[(expr) ? 1 : -1]

/**
 * Validate that critical structures fit in cache line
 * This ensures we don't have unnecessary cache misses
 */
#define ASSERT_CACHE_LINE_FITS(type) \
    OPTIM_ASSERT(sizeof(type) <= CACHE_LINE_SIZE, \
                 type##_must_fit_in_cache_line)

/*=============================================================================
 * Memory Barrier Hints
 *=============================================================================*/

/**
 * Memory barrier to prevent compiler reordering
 * Use carefully - can impact performance
 */
#ifdef __GNUC__
# define BARRIER() __asm__ __volatile__("" ::: "memory")
#else
# define BARRIER() ((void)0)
#endif

/*=============================================================================
 * Prefetch Distance
 *=============================================================================*/

/**
 * Number of iterations ahead to prefetch
 * Tune based on memory latency and cache size
 */
#define PREFETCH_DISTANCE 4

/**
 * Prefetch loop iteration
 * Call this in a loop to prefetch data ahead of current iteration
 */
#define PREFETCH_LOOP_ITERATION(base, index, size) \
    do { \
        if (((index) + PREFETCH_DISTANCE) < (size)) { \
            PREFETCH_READ((base) + ((index) + PREFETCH_DISTANCE)); \
        } \
    } while (0)

#endif /* ROSETTA_OPTIMIZATIONS_H */
