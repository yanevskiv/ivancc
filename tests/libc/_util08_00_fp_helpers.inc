// Includable helper file for <_fp.h> tests.

// Includes.
#include <_test.h>
#include <_sys.h>
#include <_err.h>
#include <_fp.h>

// The number at the start of str should be want as a double, ending at length.
static _Bool converts(const char *str, double want, unsigned long length)
{
    int lost = 0;
    unsigned long end = 99;

    lost += ! _TEST_FLOAT(_Fp_ToDouble(str, _FP_TEXT_NARROW, &end), want);
    lost += ! _TEST_UINT(end, length);
    return lost == 0;
}

// An exponent after letter at str's start should be want, ending at length.
static _Bool raises(const char *str, int letter, long want, unsigned long length)
{
    int lost = 0;
    long exp = 99;

    lost += ! _TEST_UINT(_Fp_ReadExponent(str, _FP_TEXT_NARROW, 0, letter, &exp), length);
    lost += ! _TEST_INT(exp, want);
    return lost == 0;
}
