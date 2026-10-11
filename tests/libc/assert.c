// (Test) Status: 134
// (Test) Error:
// | a.out: assert.c:121: fail: Assertion `str[0] == '"' || strcmp(str, "a\\b\"c") == 0' failed.
// <assert.h> defines assert, a macro that #undef removes and each inclusion defines again by whether NDEBUG is defined there (S7.2p1),
// one that under NDEBUG is a void expression evaluating nothing, and otherwise a void expression that evaluates a scalar argument of any type once (S7.2.1.1p2),
// and for a false one writes its text, file, line and function to standard error and ends the program by SIGABRT, the checks before it reporting nothing.

#include <_test.h>
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

// (S7.2p2) assert should be a macro.
#define DEFINED 0
#if defined(assert)
#undef DEFINED
#define DEFINED 1
#endif

#define FIVE 5
#define TEN  10

static int count;

// Return the number of calls made since count was cleared, this one counted.
static int bump(void)
{
    count++;
    return count;
}

// With NDEBUG never defined, assert evaluates its argument once.
static void Test_AssertActive(void)
{
    count = 0;
    assert(bump() == 1);
    _TEST_INT(count, 1);
}

#define NDEBUG
#include <assert.h>

// Under NDEBUG, assert evaluates nothing, and is an expression still.
static void Test_AssertDisabled(void)
{
    count = 0;
    assert(0);
    assert(bump() == 0);
    assert(count = TEN);
    _TEST_INT(count, 0);
    _TEST_INT((assert(0), FIVE), FIVE);
}

#undef assert

// (S6.10.3.5p2) assert should be no macro after #undef.
#define SURVIVED 0
#if defined(assert)
#undef SURVIVED
#define SURVIVED 1
#endif

#undef NDEBUG
#include <assert.h>

// assert is a macro, which #undef removes.
static void Test_AssertMacro(void)
{
    _TEST_INT(DEFINED, 1);
    _TEST_INT(SURVIVED, 0);
}

// Included again without NDEBUG, assert evaluates its argument once more.
static void Test_AssertEnabled(void)
{
    count = 0;
    assert(bump() == 1);
    assert(bump() == 2);
    _TEST_INT(count, 2);
    assert(count = TEN);
    _TEST_INT(count, TEN);
}

// assert takes a true scalar of any type, over lines too, and is an expression.
static void Test_AssertScalars(void)
{
    int i = -1;
    char ch = 'c';
    int *ptr = &i;
    double dbl = 0.5;
    const char *str = "s";
    unsigned long ul = ULONG_MAX;

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
    _TEST_INT((assert(1), FIVE), FIVE);
}

// Assert a string opens with a quote or is one with a quote and a backslash.
static void fail(const char *str)
{
    assert(str[0] == '"' || strcmp(str, "a\\b\"c") == 0);
}

// A false argument reports itself, quoted as written, and ends the program.
static void Test_AssertFails(void)
{
    fail("\"");
    fail("a\\b\"c");
    fail("b");
}

int main(void)
{
    Test_AssertActive();
    Test_AssertDisabled();
    Test_AssertMacro();
    Test_AssertEnabled();
    Test_AssertScalars();
    Test_AssertFails();
    return _Test_Status();
}
