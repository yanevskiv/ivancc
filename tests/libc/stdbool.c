// (Test) Status: 0
// <stdbool.h> defines bool, true, false and __bool_true_false_are_defined (S7.16p2-p3), true, false and the last as the int constants 1, 0 and 1, each fit for #if,
// and bool as _Bool, an unsigned type which holds 0 or 1 and takes any value that compares unequal to 0 as 1 (S6.3.1.2p1), a fraction, an address and what an increment or a decrement leaves among them.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>

// (S7.16p3) true and __bool_true_false_are_defined should be 1, false 0.
#define DIRECTIVE 0
#if (true == 1) \
    && (false == 0) \
    && (__bool_true_false_are_defined == 1)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

#define LARGE 256

// (S7.16p3) The three macros should be the int constants 1, 0 and 1.
static void Test_StdboolMacros(void)
{
    _TEST_INT(true, 1);
    _TEST_INT(false, 0);
    _TEST_INT(__bool_true_false_are_defined, 1);
    _TEST_INT(DIRECTIVE, 1);
    _TEST_UINT(sizeof(true), sizeof(int));
    _TEST_UINT(sizeof(false), sizeof(int));
}

// (S7.16p2) bool should be _Bool, an unsigned type that holds true and false.
static void Test_StdboolBool(void)
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

// (S6.3.1.2p1) A value unequal to 0 should convert to 1, and one equal to 0.
static void Test_StdboolConversion(void)
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

// (S6.3.1.2p1) true incremented should stay true, false decremented be true.
static void Test_StdboolSteps(void)
{
    bool flag = false;

    flag++;
    _TEST_INT(flag, 1);
    flag++;
    _TEST_INT(flag, 1);
    flag--;
    _TEST_INT(flag, 0);
    flag--;
    _TEST_INT(flag, 1);
}

int main(void)
{
    Test_StdboolMacros();
    Test_StdboolBool();
    Test_StdboolConversion();
    Test_StdboolSteps();
    return _Test_Status();
}
