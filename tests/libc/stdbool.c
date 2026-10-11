// (Test) Status: 0
// <stdbool.h> defines bool, true, false and __bool_true_false_are_defined (S7.16p2-p3), true, false and the last as the int constants 1, 0 and 1, each fit for #if,
// and bool as _Bool, an unsigned type which holds 0 or 1 and takes any value that compares unequal to 0 as 1 (S6.3.1.2p1), a fraction and an address among them.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>

#if true == 1 && false == 0 && __bool_true_false_are_defined == 1
#define DIRECTIVE 1
#else
#define DIRECTIVE 0
#endif

#define LARGE 256

// The three macros are the int constants 1, 0 and 1, in a directive too.
static void test_macros(void)
{
    _TEST_INT(true, 1);
    _TEST_INT(false, 0);
    _TEST_INT(__bool_true_false_are_defined, 1);
    _TEST_INT(DIRECTIVE, 1);
    _TEST_UINT(sizeof(true), sizeof(int));
    _TEST_UINT(sizeof(false), sizeof(int));
}

// bool is _Bool, an unsigned type that holds true and false.
static void test_bool(void)
{
    bool no = false;
    bool yes = true;

    _TEST_UINT(sizeof(bool), sizeof(_Bool));
    _TEST_INT(no, 0);
    _TEST_INT(yes, 1);
    _TEST_TRUE(yes);
    _TEST_TRUE(! no);
    _TEST_TRUE((bool) -1 > 0);
}

// A value that compares unequal to 0 converts to 1, and one equal to 0.
static void test_conversion(void)
{
    bool zero = 0;
    bool none = 0.0;
    bool null = NULL;
    bool large = LARGE;
    bool number = 42;
    bool negative = -1;
    bool fraction = 0.5;
    bool address = &zero;

    _TEST_INT(zero, 0);
    _TEST_INT(none, 0);
    _TEST_INT(null, 0);
    _TEST_INT(large, 1);
    _TEST_INT(number, 1);
    _TEST_INT(negative, 1);
    _TEST_INT(fraction, 1);
    _TEST_INT(address, 1);
}

int main(void)
{
    test_macros();
    test_bool();
    test_conversion();
    return _Test_Status();
}
