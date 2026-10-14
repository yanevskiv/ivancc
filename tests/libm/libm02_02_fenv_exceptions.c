// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <fenv.h> exceptions raised, tested and cleared.

// Include helpers.
#include "libm02_00_fenv_helpers.inc"

// A zero the compiler cannot fold.
static volatile double zero = 0.0;

// feraiseexcept should set each exception's flag, the overflow and underflow
// perhaps with inexact, fetestexcept should find it, and feclearexcept should
// clear what it is given alone; double's and long double's arithmetic should
// set the flags fetestexcept finds too.
// [C99:7.6.2.1p3]: feclearexcept, zero once every flag is cleared
// [C99:7.6.2.3p2]: feraiseexcept, inexact with overflow or underflow implementation-defined
// [C99:7.6.2.3p3]: feraiseexcept, zero once every exception is raised
// [C99:7.6.2.5p3]: fetestexcept, the OR of the flags of excepts set
static void Test_FenvExceptions(void)
{
    volatile double sink;
    volatile long double lsink;

    for (int i = 0; i < EXCEPTS; i++) {
        _TEST_INT(feclearexcept(FE_ALL_EXCEPT), 0);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
        _TEST_INT(feraiseexcept(excepts[i]), 0);
        _TEST_INT(fetestexcept(excepts[i]), excepts[i]);
        _TEST_INT(fetestexcept(FE_ALL_EXCEPT & ~excepts[i] & ~FE_INEXACT), 0);
        _TEST_INT(feclearexcept(excepts[i]), 0);
        _TEST_INT(fetestexcept(excepts[i]), 0);
    }
    _TEST_INT(feraiseexcept(0), 0);
    _TEST_INT(feclearexcept(0), 0);
    _TEST_INT(fetestexcept(0), 0);

    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(feraiseexcept(FE_INVALID | FE_OVERFLOW), 0);
    _TEST_INT(fetestexcept(FE_INVALID | FE_OVERFLOW | FE_DIVBYZERO), FE_INVALID | FE_OVERFLOW);
    _TEST_INT(feclearexcept(FE_INVALID), 0);
    _TEST_INT(fetestexcept(FE_INVALID | FE_OVERFLOW), FE_OVERFLOW);

    feclearexcept(FE_ALL_EXCEPT);
    sink = one / zero;
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    sink = zero / zero;
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INVALID);
    feclearexcept(FE_ALL_EXCEPT);
    sink = third();
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    sink = DBL_MAX * (one + one);
    _TEST_INT(fetestexcept(FE_OVERFLOW | FE_UNDERFLOW), FE_OVERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    sink = DBL_MIN / three;
    _TEST_INT(fetestexcept(FE_OVERFLOW | FE_UNDERFLOW), FE_UNDERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    lsink = lone / (lone - lone);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_DIVBYZERO);
    feclearexcept(FE_ALL_EXCEPT);
    lsink = lthird();
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), FE_INEXACT);
    feclearexcept(FE_ALL_EXCEPT);
    lsink = LDBL_MAX * (lone + lone);
    _TEST_INT(fetestexcept(FE_OVERFLOW | FE_UNDERFLOW), FE_OVERFLOW);
    feclearexcept(FE_ALL_EXCEPT);
    _TEST_INT(fetestexcept(FE_ALL_EXCEPT), 0);
    (void) sink;
    (void) lsink;
}

int main(void)
{
    Test_FenvExceptions();
    return _Test_Status();
}
