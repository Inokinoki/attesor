/*
 * Rosetta Hash Functions
 *
 * This module provides hash functions for translation cache lookups,
 * string hashing, and arbitrary data hashing.
 */

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * Hash a 64-bit address for translation cache lookup
 *
 * Uses a simple multiplicative hash function optimized for
 * addresses that are typically aligned to 4-byte boundaries.
 *
 * @param addr The address to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_address(uint64_t addr)
{
    /* Golden ratio multiplicative hash */
    uint64_t hash = addr * 2654435761ULL;
    return (uint32_t)(hash >> 32);
}

/**
 * Hash a null-terminated string
 *
 * Uses DJB2 hash algorithm for string hashing.
 * Commonly used for symbol names and file paths.
 *
 * @param s Null-terminated string to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_string(const char *s)
{
    uint32_t hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/**
 * Compute hash over arbitrary data
 *
 * Uses a simple rolling hash suitable for hashing
 * blocks of code or data structures.
 *
 * @param data Pointer to data to hash
 * @param len Length of data in bytes
 * @return Hash value (32-bit)
 */
uint32_t hash_compute(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        hash = hash * 31 + bytes[i];
    }

    return hash;
}

/**
 * ELF hash function for symbol lookup
 *
 * Standard ELF hash function used in ELF hash tables.
 *
 * @param name Symbol name to hash
 * @return Hash value (32-bit)
 */
uint32_t elf_hash_symbol(const char *name)
{
    uint32_t h = 0;
    uint32_t g;
    const unsigned char *p;

    for (p = (const unsigned char *)name; *p; p++) {
        h = (h << 4) + *p;
        g = h & 0xf0000000;
        if (g)
            h ^= g >> 24;
        h &= ~g;
    }

    return h;
}

/**
 * GNU hash function for symbol lookup
 *
 * GNU hash function used in .gnu.hash sections.
 *
 * @param name Symbol name to hash
 * @return Hash value (32-bit)
 */
uint32_t elf_gnu_hash_symbol(const char *name)
{
    uint32_t h = 5381;
    const unsigned char *p;

    for (p = (const unsigned char *)name; *p; p++) {
        h = (h << 5) + h + *p;
    }

    return h;
}

/**
 * DJB2 hash variant for strings
 *
 * Another variant of the DJB2 hash algorithm.
 *
 * @param str String to hash
 * @param len Maximum length to hash (0 = unlimited)
 * @return Hash value (32-bit)
 */
uint32_t hash_string_len(const char *str, size_t len)
{
    uint32_t hash = 5381;
    size_t i = 0;

    while (str[i] && (len == 0 || i < len)) {
        hash = ((hash << 5) + hash) + (unsigned char)str[i];
        i++;
    }

    return hash;
}

/**
 * FNV-1a hash for data
 *
 * Fowler-Noll-Vo hash function, good for general purpose hashing.
 *
 * @param data Data to hash
 * @param len Length of data
 * @return Hash value (64-bit)
 */
uint64_t hash_fnv1a(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 14695981039346656037ULL;
    size_t i;

    for (i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }

    return hash;
}

/**
 * MurmurHash3 finalizer
 *
 * Finalizer function for MurmurHash3 algorithm.
 *
 * @param h Hash value to finalize
 * @return Finalized hash (64-bit)
 */
uint64_t hash_murmur_finalizer(uint64_t h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

/**
 * CRC32 hash for data
 *
 * CRC32 checksum suitable for error detection and hashing.
 *
 * @param crc Initial CRC value (or 0 for new hash)
 * @param data Data to hash
 * @param len Length of data
 * @return Updated CRC32 value
 */
uint32_t hash_crc32(uint32_t crc, const void *data, size_t len)
{
    static const uint32_t crc32_table[256] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
        /* Full table would be 256 entries - truncated for brevity */
    };

    const uint8_t *bytes = (const uint8_t *)data;
    size_t i;

    crc ^= 0xffffffff;
    for (i = 0; i < len; i++) {
        crc = (crc >> 4) ^ crc32_table[(crc ^ bytes[i]) & 0x0f];
        crc = (crc >> 4) ^ crc32_table[(crc ^ (bytes[i] >> 4)) & 0x0f];
    }
    return crc ^ 0xffffffff;
}
