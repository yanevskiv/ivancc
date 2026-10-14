// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <fenv.h> flags saved and set.

// Include helpers.
#include "libm02_00_fenv_helpers.inc"

// fesetexceptflag should set the flags of its excepts as fegetexceptflag saved
// them, set or clear, and leave the others as they are.
// [C99:7.6.2.2p3]: fegetexceptflag, zero once the flags are stored
// [C99:7.6.2.4p2]: fesetexceptflag, the flags of excepts as *flagp holds them
// [C99:7.6.2.4p3]: fesetexceptflag, zero once every flag is set
static void Test_FenvFlags(void)
{
    fexcept_t saved;
    fexcept_t clear;

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(fegetexceptflag(&clear, FE_ALL_EXCEPT), 0);
    feraiseexcept(FE_INVALID | FE_DIVBYZERO);
    _TEST_INT(fegetexceptflag(&saved, FE_ALL_EXCEPT), 0);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(fesetexceptflag(&saved, FE_ALL_EXCEPT), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID | FE_DIVBYZERO);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(fesetexceptflag(&saved, FE_INVALID), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);

    feraiseexcept(FE_DIVBYZERO);
    _TEST_INT(fesetexceptflag(&clear, FE_INVALID), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);

    _TEST_INT(fesetexceptflag(&saved, 0), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_FenvFlags();
    return _Test_Status();
}
