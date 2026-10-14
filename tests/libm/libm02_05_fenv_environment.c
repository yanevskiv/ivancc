// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <fenv.h> environments saved, held, set and updated.

// Include helpers.
#include "libm02_00_fenv_helpers.inc"

// fesetenv should bring back the flags and direction fegetenv saved, without
// raising, and FE_DFL_ENV's no flags and to nearest; feholdexcept should save
// the environment and clear the flags, and feupdateenv should set it back
// with the flags raised since.
// [C99:7.6p8]: FE_DFL_ENV, the environment at program startup
// [C99:7.6.4.1p3]: fegetenv, zero once the environment is stored
// [C99:7.6.4.2p2]: feholdexcept, the environment saved and the flags cleared
// [C99:7.6.4.2p3]: feholdexcept, zero if and only if non-stop mode is installed
// [C99:7.6.4.3p2]: fesetenv, the flags installed, not raised
// [C99:7.6.4.3p3]: fesetenv, zero once the environment is established
// [C99:7.6.4.4p2]: feupdateenv, the environment installed, then the saved exceptions raised
// [C99:7.6.4.4p3]: feupdateenv, zero once all is carried out
static void Test_FenvEnvironment(void)
{
    fenv_t saved;
    fenv_t held;

    feclearexcept(FE_ALL_EXCEPT);
    feraiseexcept(FE_DIVBYZERO);
    fesetround(FE_UPWARD);
    _TEST_INT(fegetenv(&saved), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    _TEST_INT(fegetround(), FE_UPWARD);

    feclearexcept(FE_ALL_EXCEPT);
    feraiseexcept(FE_INVALID);
    fesetround(FE_TOWARDZERO);
    _TEST_INT(fesetenv(&saved), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    _TEST_INT(fegetround(), FE_UPWARD);
    _TEST_FLOAT(third(), 0x1.5555555555556p-2);

    _TEST_INT(fesetenv(FE_DFL_ENV), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_INT(fegetround(), FE_TONEAREST);

    feraiseexcept(FE_OVERFLOW);
    fesetround(FE_DOWNWARD);
    _TEST_INT(feholdexcept(&held), 0);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    _TEST_INT(fegetround(), FE_DOWNWARD);
    feraiseexcept(FE_INVALID);
    fesetround(FE_UPWARD);
    _TEST_INT(feupdateenv(&held), 0);
    _TEST_INT(fetestexcept(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO), FE_INVALID | FE_OVERFLOW);
    _TEST_INT(fegetround(), FE_DOWNWARD);

    feraiseexcept(FE_DIVBYZERO);
    _TEST_INT(feupdateenv(FE_DFL_ENV), 0);
    _TEST_INT(fetestexcept(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO), FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO);
    _TEST_INT(fegetround(), FE_TONEAREST);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    Test_FenvEnvironment();
    return _Test_Status();
}
