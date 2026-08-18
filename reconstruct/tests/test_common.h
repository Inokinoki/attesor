#ifndef OAH_TEST_H
#define OAH_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_tests_run;
static int g_tests_failed;

#define CHECK(cond)                                                             \
    do {                                                                        \
        g_tests_run++;                                                          \
        if (!(cond)) {                                                          \
            g_tests_failed++;                                                   \
            fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                       \
    } while (0)

#define CHECK_EQ_U64(a, b)                                                      \
    do {                                                                        \
        unsigned long long _va = (unsigned long long)(a);                       \
        unsigned long long _vb = (unsigned long long)(b);                       \
        g_tests_run++;                                                          \
        if (_va != _vb) {                                                       \
            g_tests_failed++;                                                   \
            fprintf(stderr, "  FAIL %s:%d: %s (%llu) != %s (%llu)\n",           \
                    __FILE__, __LINE__, #a, _va, #b, _vb);                      \
        }                                                                       \
    } while (0)

#define CHECK_STREQ(a, b)                                                       \
    do {                                                                        \
        const char *_sa = (a);                                                  \
        const char *_sb = (b);                                                  \
        g_tests_run++;                                                          \
        if (!_sa || !_sb || strcmp(_sa, _sb) != 0) {                            \
            g_tests_failed++;                                                   \
            fprintf(stderr, "  FAIL %s:%d: %s (%s) != %s (%s)\n",               \
                    __FILE__, __LINE__, #a, _sa ? _sa : "(null)",               \
                    #b, _sb ? _sb : "(null)");                                  \
        }                                                                       \
    } while (0)

static int test_report(const char *suite)
{
    if (g_tests_failed) {
        fprintf(stderr, "%s: %d/%d failed\n", suite, g_tests_failed, g_tests_run);
        return 1;
    }
    printf("%s: %d checks passed\n", suite, g_tests_run);
    return 0;
}

#endif
