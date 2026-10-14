// (Test) Compiler flags: -lm
// (Test) Status: 0
// A test file for <fenv.h> rounding directions.

// Include helpers.
#include "libm02_00_fenv_helpers.inc"

// Defines.
#define BAD_DIRECTION (-1)

// The quotients of 1 by 3 in each direction, by type and sign.
static const float fup = 0x1.555556p-2f;
static const float fdown = 0x1.555554p-2f;
static const double dup = 0x1.5555555555556p-2;
static const double ddown = 0x1.5555555555555p-2;
static const long double lup = 0x1.5555555555555556p-2L;
static const long double ldown = 0x1.5555555555555554p-2L;

static volatile float fone = 1.0f;
static volatile float fthree = 3.0f;

// Each direction should be set and got, and should round float's, double's
// and long double's quotients its way; a value no macro names should change
// nothing.
// [C99:7.6.3.1p3]: fegetround, the macro of the current direction
// [C99:7.6.3.2p2]: fesetround, a value of no macro changes nothing
// [C99:7.6.3.2p3]: fesetround, zero if and only if the direction is set
// [C99:F.3p1]: the / operator, IEC 60559's divide
static void Test_FenvRounding(void)
{
    _TEST_INT(fegetround(), FE_TONEAREST);
    for (int i = 0; i < MODES; i++) {
        _TEST_INT(fesetround(modes[i]), 0);
        _TEST_INT(fegetround(), modes[i]);
        _TEST_TRUE(fesetround(BAD_DIRECTION) != 0);
        _TEST_INT(fegetround(), modes[i]);
    }

    fesetround(FE_TONEAREST);
    _TEST_FLOAT(fone / fthree, fup);
    _TEST_FLOAT(third(), ddown);
    _TEST_FLOAT(-one / three, -ddown);
    _TEST_FLOAT(lthird(), lup);
    _TEST_FLOAT(-lone / lthree, -lup);
    fesetround(FE_UPWARD);
    _TEST_FLOAT(fone / fthree, fup);
    _TEST_FLOAT(third(), dup);
    _TEST_FLOAT(-one / three, -ddown);
    _TEST_FLOAT(lthird(), lup);
    _TEST_FLOAT(-lone / lthree, -ldown);
    fesetround(FE_DOWNWARD);
    _TEST_FLOAT(fone / fthree, fdown);
    _TEST_FLOAT(third(), ddown);
    _TEST_FLOAT(-one / three, -dup);
    _TEST_FLOAT(lthird(), ldown);
    _TEST_FLOAT(-lone / lthree, -lup);
    fesetround(FE_TOWARDZERO);
    _TEST_FLOAT(fone / fthree, fdown);
    _TEST_FLOAT(third(), ddown);
    _TEST_FLOAT(-one / three, -ddown);
    _TEST_FLOAT(lthird(), ldown);
    _TEST_FLOAT(-lone / lthree, -ldown);
    fesetround(FE_TONEAREST);
}

int main(void)
{
    Test_FenvRounding();
    return _Test_Status();
}
