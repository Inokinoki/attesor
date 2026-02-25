/*
 * Rosetta String and Memory Utilities Header
 *
 * This module provides standard string and memory operations
 * for use within the Rosetta translator.
 */

#ifndef ROSETTA_UTILS_H
#define ROSETTA_UTILS_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * String Length Operations
 * ============================================================================ */

size_t rosetta_strlen(const char *s);
size_t rosetta_strnlen(const char *s, size_t maxlen);

/* ============================================================================
 * String Copy Operations
 * ============================================================================ */

char *rosetta_strcpy(char *dest, const char *src);
char *rosetta_strncpy(char *dest, const char *src, size_t n);

/* ============================================================================
 * String Concatenation Operations
 * ============================================================================ */

char *rosetta_strcat(char *dest, const char *src);
char *rosetta_strncat(char *dest, const char *src, size_t n);

/* ============================================================================
 * String Compare Operations
 * ============================================================================ */

int rosetta_strcmp(const char *s1, const char *s2);
int rosetta_strncmp(const char *s1, const char *s2, size_t n);
int rosetta_strcasecmp(const char *s1, const char *s2);

/* ============================================================================
 * String Search Operations
 * ============================================================================ */

char *rosetta_strchr(const char *s, int c);
char *rosetta_strrchr(const char *s, int c);
char *rosetta_strstr(const char *haystack, const char *needle);
size_t rosetta_strspn(const char *s, const char *accept);
size_t rosetta_strcspn(const char *s, const char *reject);

/* ============================================================================
 * Memory Operations
 * ============================================================================ */

void *rosetta_memcpy(void *dest, const void *src, size_t n);
void *rosetta_memmove(void *dest, const void *src, size_t n);
int rosetta_memcmp(const void *s1, const void *s2, size_t n);
void *rosetta_memset(void *s, int c, size_t n);
void *rosetta_memchr(const void *s, int c, size_t n);

#endif /* ROSETTA_UTILS_H */
