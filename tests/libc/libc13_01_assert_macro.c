// (Test) Status: 0
// A test file for <assert.h> assert as a macro.

// Include helpers.
#include "libc13_00_assert_helpers.inc"

// [C99:7.2p2] assert should be a macro.
#define DEFINED 0
#if defined(assert)
#undef DEFINED
#define DEFINED 1
#endif

#undef assert

// [C99:6.10.3.5p2] assert should be no macro after #undef.
#define SURVIVED 0
#if defined(assert)
#undef SURVIVED
#define SURVIVED 1
#endif

// [C99:7.2p2] assert should be a macro, which #undef removes.
static void Test_AssertMacro(void)
{
    _TEST_INT(DEFINED, 1);
    _TEST_INT(SURVIVED, 0);
}

int main(void)
{
    Test_AssertMacro();
    return _Test_Status();
}
