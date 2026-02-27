/* ============================================================================
 * Rosetta Refactored - Utility Functions Header
 * ============================================================================
 *
 * This module provides general utility functions for the Rosetta
 * translation layer.
 * ============================================================================ */

#ifndef ROSETTA_REFACTORED_UTILS_H
#define ROSETTA_REFACTORED_UTILS_H

#include "rosetta_refactored_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * String Utility Functions
 * ============================================================================ */

/**
 * Convert integer to ASCII string
 * @param value Value to convert
 * @param buffer Output buffer
 * @param base Number base (10 for decimal, 16 for hex)
 * @return Pointer to buffer
 */
char *rosetta_utils_itoa(int64_t value, char *buffer, int base);

/**
 * Convert unsigned integer to ASCII string
 * @param value Value to convert
 * @param buffer Output buffer
 * @param base Number base (10 for decimal, 16 for hex)
 * @return Pointer to buffer
 */
char *rosetta_utils_utoa(uint64_t value, char *buffer, int base);

/**
 * Convert ASCII string to integer
 * @param str Input string
 * @return Integer value
 */
int64_t rosetta_utils_atoi(const char *str);

/* ============================================================================
 * Memory Utility Functions
 * ============================================================================ */

/**
 * Swap two memory regions
 * @param a First memory region
 * @param b Second memory region
 * @param size Size in bytes
 */
void rosetta_utils_memswap(void *a, void *b, size_t size);

/**
 * Find byte pattern in memory
 * @param data Data to search
 * @param size Size of data
 * @param pattern Pattern to find
 * @param pattern_size Size of pattern
 * @return Pointer to first occurrence, or NULL if not found
 */
void *rosetta_utils_memfind(const void *data, size_t size,
                            const void *pattern, size_t pattern_size);

/**
 * Fill memory with word pattern
 * @param dest Destination buffer
 * @param pattern Word pattern to fill
 * @param size Size in bytes
 * @return Pointer to destination
 */
void *rosetta_utils_memfill_word(void *dest, uint64_t pattern, size_t size);

/* ============================================================================
 * Bit Manipulation Utility Functions
 * ============================================================================ */

/**
 * Count set bits in 64-bit value
 * @param value Value to count bits in
 * @return Number of set bits
 */
int rosetta_utils_popcount(uint64_t value);

/**
 * Count leading zeros in 64-bit value
 * @param value Value to count leading zeros
 * @return Number of leading zeros (64 if value is 0)
 */
int rosetta_utils_clz(uint64_t value);

/**
 * Count trailing zeros in 64-bit value
 * @param value Value to count trailing zeros
 * @return Number of trailing zeros (64 if value is 0)
 */
int rosetta_utils_ctz(uint64_t value);

/**
 * Byte swap 32-bit value
 * @param value Value to swap
 * @return Byte-swapped value
 */
uint32_t rosetta_utils_bswap32(uint32_t value);

/**
 * Byte swap 64-bit value
 * @param value Value to swap
 * @return Byte-swapped value
 */
uint64_t rosetta_utils_bswap64(uint64_t value);

/**
 * Reverse bit order in 64-bit value
 * @param value Value to reverse
 * @return Bit-reversed value
 */
uint64_t rosetta_utils_rbit(uint64_t value);

/**
 * Check if value is power of 2
 * @param value Value to check
 * @return true if power of 2, false otherwise
 */
bool rosetta_utils_is_power_of_2(uint64_t value);

/**
 * Round up to nearest power of 2
 * @param value Value to round
 * @return Next power of 2 >= value
 */
uint64_t rosetta_utils_round_up_pow2(uint64_t value);

/* ============================================================================
 * Format Utility Functions
 * ============================================================================ */

/**
 * Format byte count as human-readable string
 * @param bytes Number of bytes
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Pointer to buffer
 */
char *rosetta_utils_format_bytes(uint64_t bytes, char *buffer, size_t buffer_size);

/**
 * Format value as hex string with 0x prefix
 * @param value Value to format
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Pointer to buffer
 */
char *rosetta_utils_format_hex(uint64_t value, char *buffer, size_t buffer_size);

/**
 * Format value as binary string
 * @param value Value to format
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Pointer to buffer
 */
char *rosetta_utils_format_binary(uint64_t value, char *buffer, size_t buffer_size);

#endif /* ROSETTA_REFACTORED_UTILS_H */
