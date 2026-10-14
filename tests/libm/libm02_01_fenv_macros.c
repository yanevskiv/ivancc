// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <fenv.h> macros.

// Include helpers.
#include "libm02_00_fenv_helpers.inc"

// The exception and rounding macros should be integer constants in #if, the
// exceptions distinct bits whose OR is FE_ALL_EXCEPT, and the directions
// distinct and nonnegative.
// [C99:7.6p5]: the exceptions, ORs distinct and ANDs zero
// [C99:7.6p6]: FE_ALL_EXCEPT, the OR of every exception
// [C99:7.6p7]: the directions, distinct nonnegative values
#define DIRECTIVE 0
#if ((FE_DIVBYZERO & FE_INEXACT) == 0) \
    && ((FE_DIVBYZERO & FE_INVALID) == 0) \
    && ((FE_DIVBYZERO & FE_OVERFLOW) == 0) \
    && ((FE_DIVBYZERO & FE_UNDERFLOW) == 0) \
    && ((FE_INEXACT & FE_INVALID) == 0) \
    && ((FE_INEXACT & FE_OVERFLOW) == 0) \
    && ((FE_INEXACT & FE_UNDERFLOW) == 0) \
    && ((FE_INVALID & FE_OVERFLOW) == 0) \
    && ((FE_INVALID & FE_UNDERFLOW) == 0) \
    && ((FE_OVERFLOW & FE_UNDERFLOW) == 0) \
    && (FE_ALL_EXCEPT == (FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)) \
    && (FE_DOWNWARD >= 0) \
    && (FE_TONEAREST >= 0) \
    && (FE_TOWARDZERO >= 0) \
    && (FE_UPWARD >= 0) \
    && (FE_DOWNWARD != FE_TONEAREST) \
    && (FE_DOWNWARD != FE_TOWARDZERO) \
    && (FE_DOWNWARD != FE_UPWARD) \
    && (FE_TONEAREST != FE_TOWARDZERO) \
    && (FE_TONEAREST != FE_UPWARD) \
    && (FE_TOWARDZERO != FE_UPWARD)
#undef DIRECTIVE
#define DIRECTIVE 1
#endif

// Each exception should be nonzero, and FE_DFL_ENV a pointer to const fenv_t
// other than any object's.
// [C99:7.6p5]: the exceptions, ORs distinct and ANDs zero
// [C99:7.6p8]: FE_DFL_ENV, a pointer to const-qualified fenv_t
static void Test_FenvMacros(void)
{
    const fenv_t *dfl = FE_DFL_ENV;
    fenv_t env;

    for (int i = 0; i < EXCEPTS; i++) {
        _TEST_TRUE(excepts[i] != 0);
        _TEST_INT(excepts[i] & FE_ALL_EXCEPT, excepts[i]);
    }
    _TEST_TRUE(dfl != &env);
    _TEST_UINT(sizeof(*FE_DFL_ENV), sizeof(env));
    _TEST_INT(DIRECTIVE, 1);
}

int main(void)
{
    Test_FenvMacros();
    return _Test_Status();
}
