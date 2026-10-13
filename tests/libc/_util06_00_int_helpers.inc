// Includable helper file for <_int.h> tests.

// Includes.
#include <_test.h>
#include <_sys.h>
#include <_err.h>
#include <_int.h>

// The number at the start of str, in base, should be want and end at length.
static _Bool reads(const char *str, int base, unsigned long want, unsigned long length, _Int_Sign wsign, _Int_Range wrange)
{
    int lost = 0;
    _Int_Sign sign = _INT_SIGN_MINUS;
    _Int_Range range = _INT_RANGE_OUTSIDE;
    unsigned long end = _INT_VALUE_MAX;

    lost += ! _TEST_UINT(_Int_ToNumber(str, _INT_TEXT_NARROW, &end, base, &sign, &range), want);
    lost += ! _TEST_UINT(end, length);
    lost += ! _TEST_INT(sign, wsign);
    lost += ! _TEST_INT(range, wrange);
    return lost == 0;
}
