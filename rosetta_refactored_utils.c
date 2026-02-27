/* ============================================================================
 * Rosetta Refactored - Utility Functions Implementation
 * ============================================================================
 *
 * This module provides general utility functions for the Rosetta
 * translation layer including string operations, memory helpers,
 * and common computations.
 * ============================================================================ */

#include "rosetta_refactored_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================
 * String Utility Functions
 * ============================================================================ */

/**
 * rosetta_utils_itoa - Convert integer to ASCII string
 * @value: Value to convert
 * @buffer: Output buffer
 * @base: Number base (10 for decimal, 16 for hex)
 * Returns: Pointer to buffer
 */
char *rosetta_utils_itoa(int64_t value, char *buffer, int base)
{
    char *ptr = buffer;
    char *start = buffer;
    char tmp;
    int negative = 0;
    uint64_t uvalue;

    if (value < 0 && base == 10) {
        negative = 1;
        uvalue = (uint64_t)(-value);
    } else {
        uvalue = (uint64_t)value;
    }

    /* Convert to string (in reverse order) */
    do {
        int digit = uvalue % base;
        *ptr++ = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        uvalue /= base;
    } while (uvalue > 0);

    if (negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    /* Reverse the string */
    while (start < --ptr) {
        tmp = *start;
        *start++ = *ptr;
        *ptr = tmp;
    }

    return buffer;
}

/**
 * rosetta_utils_utoa - Convert unsigned integer to ASCII string
 * @value: Value to convert
 * @buffer: Output buffer
 * @base: Number base (10 for decimal, 16 for hex)
 * Returns: Pointer to buffer
 */
char *rosetta_utils_utoa(uint64_t value, char *buffer, int base)
{
    char *ptr = buffer;
    char *start = buffer;
    char tmp;

    /* Convert to string (in reverse order) */
    do {
        int digit = value % base;
        *ptr++ = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        value /= base;
    } while (value > 0);

    *ptr = '\0';

    /* Reverse the string */
    while (start < --ptr) {
        tmp = *start;
        *start++ = *ptr;
        *ptr = tmp;
    }

    return buffer;
}

/**
 * rosetta_utils_atoi - Convert ASCII string to integer
 * @str: Input string
 * Returns: Integer value
 */
int64_t rosetta_utils_atoi(const char *str)
{
    int64_t result = 0;
    int negative = 0;

    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    /* Check for sign */
    if (*str == '-') {
        negative = 1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    /* Convert digits */
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return negative ? -result : result;
}

/* ============================================================================
 * Memory Utility Functions
 * ============================================================================ */

/**
 * rosetta_utils_memswap - Swap two memory regions
 * @a: First memory region
 * @b: Second memory region
 * @size: Size in bytes
 */
void rosetta_utils_memswap(void *a, void *b, size_t size)
{
    uint8_t *pa = (uint8_t *)a;
    uint8_t *pb = (uint8_t *)b;
    uint8_t tmp;

    while (size-- > 0) {
        tmp = pa[0];
        pa[0] = pb[0];
        pb[0] = tmp;
        pa++;
        pb++;
    }
}

/**
 * rosetta_utils_memfind - Find byte pattern in memory
 * @data: Data to search
 * @size: Size of data
 * @pattern: Pattern to find
 * @pattern_size: Size of pattern
 * Returns: Pointer to first occurrence, or NULL if not found
 */
void *rosetta_utils_memfind(const void *data, size_t size,
                            const void *pattern, size_t pattern_size)
{
    const uint8_t *pdata = (const uint8_t *)data;
    const uint8_t *ppattern = (const uint8_t *)pattern;

    if (pattern_size > size) {
        return NULL;
    }

    for (size_t i = 0; i <= size - pattern_size; i++) {
        if (memcmp(&pdata[i], ppattern, pattern_size) == 0) {
            return (void *)&pdata[i];
        }
    }

    return NULL;
}

/**
 * rosetta_utils_memfill - Fill memory with word pattern
 * @dest: Destination buffer
 * @pattern: Word pattern to fill
 * @size: Size in bytes
 * Returns: Pointer to destination
 */
void *rosetta_utils_memfill_word(void *dest, uint64_t pattern, size_t size)
{
    uint64_t *ptr = (uint64_t *)dest;
    size_t count = size / sizeof(uint64_t);

    while (count-- > 0) {
        *ptr++ = pattern;
    }

    /* Handle remaining bytes */
    uint8_t *remainder = (uint8_t *)ptr;
    size_t remaining = size % sizeof(uint64_t);
    uint8_t *pattern_bytes = (uint8_t *)&pattern;

    for (size_t i = 0; i < remaining; i++) {
        remainder[i] = pattern_bytes[i];
    }

    return dest;
}

/* ============================================================================
 * Bit Manipulation Utility Functions
 * ============================================================================ */

/**
 * rosetta_utils_popcount - Count set bits in 64-bit value
 * @value: Value to count bits in
 * Returns: Number of set bits
 */
int rosetta_utils_popcount(uint64_t value)
{
    int count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

/**
 * rosetta_utils_clz - Count leading zeros in 64-bit value
 * @value: Value to count leading zeros
 * Returns: Number of leading zeros (64 if value is 0)
 */
int rosetta_utils_clz(uint64_t value)
{
    int count = 0;

    if (value == 0) {
        return 64;
    }

    while ((value & (1ULL << 63)) == 0) {
        count++;
        value <<= 1;
    }

    return count;
}

/**
 * rosetta_utils_ctz - Count trailing zeros in 64-bit value
 * @value: Value to count trailing zeros
 * Returns: Number of trailing zeros (64 if value is 0)
 */
int rosetta_utils_ctz(uint64_t value)
{
    int count = 0;

    if (value == 0) {
        return 64;
    }

    while ((value & 1) == 0) {
        count++;
        value >>= 1;
    }

    return count;
}

/**
 * rosetta_utils_bswap32 - Byte swap 32-bit value
 * @value: Value to swap
 * Returns: Byte-swapped value
 */
uint32_t rosetta_utils_bswap32(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}

/**
 * rosetta_utils_bswap64 - Byte swap 64-bit value
 * @value: Value to swap
 * Returns: Byte-swapped value
 */
uint64_t rosetta_utils_bswap64(uint64_t value)
{
    return ((value & 0x00000000000000FFULL) << 56) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x00000000FF000000ULL) << 8)  |
           ((value & 0x000000FF00000000ULL) >> 8)  |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0xFF00000000000000ULL) >> 56);
}

/**
 * rosetta_utils_rbit - Reverse bit order in 64-bit value
 * @value: Value to reverse
 * Returns: Bit-reversed value
 */
uint64_t rosetta_utils_rbit(uint64_t value)
{
    uint64_t result = 0;
    int i;

    for (i = 0; i < 64; i++) {
        if (value & (1ULL << i)) {
            result |= (1ULL << (63 - i));
        }
    }

    return result;
}

/**
 * rosetta_utils_is_power_of_2 - Check if value is power of 2
 * @value: Value to check
 * Returns: true if power of 2, false otherwise
 */
bool rosetta_utils_is_power_of_2(uint64_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

/**
 * rosetta_utils_round_up_pow2 - Round up to nearest power of 2
 * @value: Value to round
 * Returns: Next power of 2 >= value
 */
uint64_t rosetta_utils_round_up_pow2(uint64_t value)
{
    if (value == 0) return 1;
    if (rosetta_utils_is_power_of_2(value)) return value;

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;

    return value;
}

/* ============================================================================
 * Format Utility Functions
 * ============================================================================ */

/**
 * rosetta_utils_format_bytes - Format byte count as human-readable string
 * @bytes: Number of bytes
 * @buffer: Output buffer
 * @buffer_size: Size of output buffer
 * Returns: Pointer to buffer
 */
char *rosetta_utils_format_bytes(uint64_t bytes, char *buffer, size_t buffer_size)
{
    const char *units[] = { "B", "KB", "MB", "GB", "TB" };
    int unit_index = 0;
    double value = (double)bytes;

    while (value >= 1024.0 && unit_index < 4) {
        value /= 1024.0;
        unit_index++;
    }

    snprintf(buffer, buffer_size, "%.2f %s", value, units[unit_index]);
    return buffer;
}

/**
 * rosetta_utils_format_hex - Format value as hex string with 0x prefix
 * @value: Value to format
 * @buffer: Output buffer
 * @buffer_size: Size of output buffer
 * Returns: Pointer to buffer
 */
char *rosetta_utils_format_hex(uint64_t value, char *buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size, "0x%016llx", (unsigned long long)value);
    return buffer;
}

/**
 * rosetta_utils_format_binary - Format value as binary string
 * @value: Value to format
 * @buffer: Output buffer
 * @buffer_size: Size of output buffer
 * Returns: Pointer to buffer
 */
char *rosetta_utils_format_binary(uint64_t value, char *buffer, size_t buffer_size)
{
    if (buffer_size < 65) return NULL;

    for (int i = 63; i >= 0; i--) {
        buffer[63 - i] = (value & (1ULL << i)) ? '1' : '0';
    }
    buffer[64] = '\0';

    return buffer;
}
