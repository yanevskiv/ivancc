// (Test) Status: 134
// (Test) Error:
// | a.out: libc_test_assert.c:92: fail: Assertion `str[0] == '"' || strcmp(str, "a\\b\"c") == 0' failed.
// <assert.h> defines assert, which does nothing under NDEBUG and otherwise reports a false argument and ends the program by SIGABRT (S7.2).

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>

#ifndef assert
#error "assert is not a macro"
#endif

static int count;

static int bump(void)
{
    count++;
    return count;
}

static bool active(void)
{
    count = 0;
    assert(bump() == 1);
    return count == 1;
}

#define NDEBUG
#include <assert.h>

static bool disabled(void)
{
    count = 0;
    assert(0);
    assert(bump() == 0);
    assert(count = 10);
    return count == 0 and (assert(0), 5) == 5;
}

#undef assert
#ifdef assert
#error "assert survived #undef"
#endif
#undef NDEBUG
#include <assert.h>

static bool enabled(void)
{
    count = 0;
    assert(bump() == 1);
    assert(bump() == 2);
    assert(count = 10);
    return count == 10;
}

static bool scalars(void)
{
    int i = -1;
    char ch = 'c';
    unsigned long ul = ULONG_MAX;
    double dbl = 0.5;
    const char *str = "s";
    int *ptr = &i;

    assert(i);
    assert(ch);
    assert(ul);
    assert(dbl);
    assert(str);
    assert(ptr);
    assert(*str == 's');
    assert((0, 1));
    assert(i < 0 ? 1 : 0);
    assert(i
           == -1);
    return (assert(1), 5) == 5;
}

static void fail(const char *str)
{
    assert(str[0] == '"' || strcmp(str, "a\\b\"c") == 0);
}

int main(void)
{
    if (not active()) return 1;
    if (not disabled()) return 2;
    if (not enabled()) return 3;
    if (not scalars()) return 4;
    fail("b");
    return 5;
}
