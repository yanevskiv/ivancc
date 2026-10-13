// (Test) Status: 0
// A test file for <stdbool.h> macros.

// Include helpers.
#include "libc02_00_stdbool_helpers.inc"

// [C99 S7.16p3] true and __bool_true_false_are_defined should be 1, false 0.
#define DIRECTIVE 0
#if (true == 1) \
    && (false == 0) \
    && (__bool_true_false_are_defined == 1)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// [C99 S7.16p3] The three macros should be the int constants 1, 0 and 1.
static void Test_StdboolMacros(void)
{
    _TEST_INT(true, 1);
    _TEST_INT(false, 0);
    _TEST_INT(__bool_true_false_are_defined, 1);
    _TEST_INT(DIRECTIVE, 1);
    _TEST_UINT(sizeof(true), sizeof(int));
    _TEST_UINT(sizeof(false), sizeof(int));
}

int main(void)
{
    Test_StdboolMacros();
    return _Test_Status();
}
