/* ============================================================================
 * Rosetta Refactored - Hash Utilities Implementation
 * ============================================================================
 *
 * This module implements hash functions for the Rosetta translation layer.
 * ============================================================================ */

#include "rosetta_hash_utils.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Hash Functions
 * ============================================================================ */

/**
 * hash_address - Hash function for addresses using XOR folding
 */
u32 hash_address(u64 addr)
{
    return (u32)(addr ^ (addr >> 32));
}

/**
 * hash_string - DJB2 string hash function
 */
u32 hash_string(const char *s)
{
    u32 hash = 5381;
    int c;

    while ((c = *s++) != 0) {
        hash = ((hash << 5) + hash) + (u32)c;  /* hash * 33 + c */
    }
    return hash;
}

/**
 * hash_compute - Generic hash using DJB2 variant
 */
u32 hash_compute(const void *data, size_t len)
{
    const u8 *p = (const u8 *)data;
    u32 hash = 5381;
    size_t i;

    for (i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + p[i];  /* hash * 33 + byte */
    }
    return hash;
}

/* ============================================================================
 * Translation Cache (stub implementations)
 *
 * Note: Full implementation uses rosetta_transcache.c and rosetta_cache.c
 * These are placeholder implementations for backward compatibility.
 * ============================================================================ */

void *translation_lookup(u64 guest_pc)
{
    (void)guest_pc;
    return NULL;  /* Stub - use rosetta_transcache for full implementation */
}

void translation_insert(u64 guest, u64 host, size_t sz)
{
    (void)guest;
    (void)host;
    (void)sz;
    /* Stub - use rosetta_transcache for full implementation */
}

/* ============================================================================
 * Hash Table Implementation
 * ============================================================================ */

/* Hash table entry */
typedef struct hash_entry {
    u64 key;
    void *value;
    struct hash_entry *next;
} hash_entry_t;

/* Hash table structure */
typedef struct hash_table {
    u32 capacity;
    u32 size;
    hash_entry_t **buckets;
} hash_table_t;

/**
 * hash_table_create - Create a new hash table
 */
void *hash_table_create(u32 capacity)
{
    hash_table_t *ht;
    size_t bucket_size;

    ht = (hash_table_t *)malloc(sizeof(hash_table_t));
    if (!ht) {
        return NULL;
    }

    ht->capacity = capacity;
    ht->size = 0;

    bucket_size = capacity * sizeof(hash_entry_t *);
    ht->buckets = (hash_entry_t **)malloc(bucket_size);
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    memset(ht->buckets, 0, bucket_size);

    return ht;
}

/**
 * hash_table_destroy - Destroy a hash table and free all entries
 */
void hash_table_destroy(void *ht_ptr)
{
    hash_table_t *ht = (hash_table_t *)ht_ptr;
    u32 i;

    if (!ht) {
        return;
    }

    /* Free all entries */
    for (i = 0; i < ht->capacity; i++) {
        hash_entry_t *entry = ht->buckets[i];
        while (entry) {
            hash_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    /* Free buckets array */
    free(ht->buckets);

    /* Free hash table structure */
    free(ht);
}

/**
 * hash_table_insert - Insert key-value pair into hash table
 */
int hash_table_insert(void *ht_ptr, u64 key, void *value)
{
    hash_table_t *ht = (hash_table_t *)ht_ptr;
    hash_entry_t *entry;
    u32 hash;

    if (!ht) {
        return -1;
    }

    /* Compute bucket index */
    hash = hash_address(key) % ht->capacity;

    /* Check if key already exists */
    entry = ht->buckets[hash];
    while (entry) {
        if (entry->key == key) {
            entry->value = value;  /* Update existing */
            return 0;
        }
        entry = entry->next;
    }

    /* Create new entry */
    entry = (hash_entry_t *)malloc(sizeof(hash_entry_t));
    if (!entry) {
        return -1;
    }

    entry->key = key;
    entry->value = value;
    entry->next = ht->buckets[hash];
    ht->buckets[hash] = entry;
    ht->size++;

    return 0;
}

/**
 * hash_table_lookup - Look up value in hash table
 */
void *hash_table_lookup(void *ht_ptr, u64 key)
{
    hash_table_t *ht = (hash_table_t *)ht_ptr;
    hash_entry_t *entry;
    u32 hash;

    if (!ht) {
        return NULL;
    }

    /* Compute bucket index */
    hash = hash_address(key) % ht->capacity;

    /* Search chain */
    entry = ht->buckets[hash];
    while (entry) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;  /* Not found */
}

/**
 * hash_table_delete - Delete key-value pair from hash table
 */
int hash_table_delete(void *ht_ptr, u64 key)
{
    hash_table_t *ht = (hash_table_t *)ht_ptr;
    hash_entry_t *entry, *prev;
    u32 hash;

    if (!ht) {
        return -1;
    }

    /* Compute bucket index */
    hash = hash_address(key) % ht->capacity;

    /* Search chain */
    prev = NULL;
    entry = ht->buckets[hash];
    while (entry) {
        if (entry->key == key) {
            /* Found - remove from chain */
            if (prev) {
                prev->next = entry->next;
            } else {
                ht->buckets[hash] = entry->next;
            }
            free(entry);
            ht->size--;
            return 0;
        }
        prev = entry;
        entry = entry->next;
    }

    return -1;  /* Not found */
}
