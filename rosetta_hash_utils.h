/* ============================================================================
 * Rosetta Refactored - Hash Utilities Header
 * ============================================================================
 *
 * This header declares hash functions for the Rosetta translation layer.
 *
 * Functions include:
 * - hash_address: Hash for addresses (guest PC hashing)
 * - hash_string: String hash function
 * - hash_compute: Generic data hash
 * - Translation cache lookup/insert
 * ============================================================================ */

#ifndef ROSETTA_HASH_UTILS_H
#define ROSETTA_HASH_UTILS_H

#include "rosetta_types.h"
#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * hash_address - Hash function for addresses
 * @addr: 64-bit address to hash
 * Returns: 32-bit hash value
 *
 * Uses XOR folding: addr ^ (addr >> 32)
 */
u32 hash_address(u64 addr);

/**
 * hash_string - Hash function for strings (DJB2 variant)
 * @s: Null-terminated string
 * Returns: 32-bit hash value
 */
u32 hash_string(const char *s);

/**
 * hash_compute - Generic hash function for arbitrary data
 * @data: Pointer to data buffer
 * @len: Length of data in bytes
 * Returns: 32-bit hash value
 */
u32 hash_compute(const void *data, size_t len);

/* ============================================================================
 * Translation Cache
 * ============================================================================ */

/**
 * translation_lookup - Look up translated code in cache
 * @guest_pc: Guest PC address
 * Returns: Pointer to translated code, or NULL if not found
 */
void *translation_lookup(u64 guest_pc);

/**
 * translation_insert - Insert translation into cache
 * @guest: Guest PC address
 * @host: Host code pointer
 * @sz: Size of translated code in bytes
 */
void translation_insert(u64 guest, u64 host, size_t sz);

/* ============================================================================
 * Hash Table API (for future implementation)
 * ============================================================================ */

/**
 * hash_table_create - Create a new hash table
 * @capacity: Initial capacity (number of buckets)
 * Returns: Pointer to hash table, or NULL on failure
 */
void *hash_table_create(u32 capacity);

/**
 * hash_table_destroy - Destroy a hash table
 * @ht: Hash table to destroy
 */
void hash_table_destroy(void *ht);

/**
 * hash_table_insert - Insert key-value pair into hash table
 * @ht: Hash table
 * @key: Key (guest PC)
 * @value: Value (host code pointer)
 * Returns: 0 on success, -1 on failure
 */
int hash_table_insert(void *ht, u64 key, void *value);

/**
 * hash_table_lookup - Look up value in hash table
 * @ht: Hash table
 * @key: Key (guest PC)
 * Returns: Value if found, NULL if not found
 */
void *hash_table_lookup(void *ht, u64 key);

/**
 * hash_table_delete - Delete key-value pair from hash table
 * @ht: Hash table
 * @key: Key to delete
 * Returns: 0 on success, -1 if not found
 */
int hash_table_delete(void *ht, u64 key);

#endif /* ROSETTA_HASH_UTILS_H */
