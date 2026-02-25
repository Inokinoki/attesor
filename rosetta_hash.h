/*
 * Rosetta Hash Functions Header
 *
 * This module provides hash functions for translation cache lookups,
 * string hashing, and arbitrary data hashing.
 */

#ifndef ROSETTA_HASH_H
#define ROSETTA_HASH_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Address Hashing
 * ============================================================================ */

/**
 * Hash a 64-bit address for translation cache lookup
 * @param addr The address to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_address(uint64_t addr);

/* ============================================================================
 * String Hashing
 * ============================================================================ */

/**
 * Hash a null-terminated string (DJB2 algorithm)
 * @param s Null-terminated string to hash
 * @return Hash value (32-bit)
 */
uint32_t hash_string(const char *s);

/**
 * Hash a string with length limit
 * @param str String to hash
 * @param len Maximum length to hash (0 = unlimited)
 * @return Hash value (32-bit)
 */
uint32_t hash_string_len(const char *str, size_t len);

/* ============================================================================
 * Data Hashing
 * ============================================================================ */

/**
 * Compute hash over arbitrary data (rolling hash)
 * @param data Pointer to data to hash
 * @param len Length of data in bytes
 * @return Hash value (32-bit)
 */
uint32_t hash_compute(const void *data, size_t len);

/**
 * FNV-1a hash for data
 * @param data Data to hash
 * @param len Length of data
 * @return Hash value (64-bit)
 */
uint64_t hash_fnv1a(const void *data, size_t len);

/**
 * CRC32 hash for data
 * @param crc Initial CRC value (or 0 for new hash)
 * @param data Data to hash
 * @param len Length of data
 * @return Updated CRC32 value
 */
uint32_t hash_crc32(uint32_t crc, const void *data, size_t len);

/* ============================================================================
 * ELF Symbol Hashing
 * ============================================================================ */

/**
 * ELF hash function for symbol lookup
 * @param name Symbol name to hash
 * @return Hash value (32-bit)
 */
uint32_t elf_hash_symbol(const char *name);

/**
 * GNU hash function for symbol lookup
 * @param name Symbol name to hash
 * @return Hash value (32-bit)
 */
uint32_t elf_gnu_hash_symbol(const char *name);

/* ============================================================================
 * Hash Utilities
 * ============================================================================ */

/**
 * MurmurHash3 finalizer
 * @param h Hash value to finalize
 * @return Finalized hash (64-bit)
 */
uint64_t hash_murmur_finalizer(uint64_t h);

#endif /* ROSETTA_HASH_H */
