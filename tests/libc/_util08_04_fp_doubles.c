// (Test) Status: 0
// A test file for <_fp.h> doubles converted from text.

// Include helpers.
#include "_util08_00_fp_helpers.inc"

// A number should convert to the nearest double, ties to even, to its end.
static void Test_FpDoubles(void)
{
    unsigned long end = 0;
    double value;

    _Err_Errno = 0;
    _TEST_TRUE(converts("1.5", 1.5, 3));
    _TEST_TRUE(converts(" \t-2.25e2x", -225.0, 9));
    _TEST_TRUE(converts("+.5", 0.5, 3));
    _TEST_TRUE(converts("5.", 5.0, 2));
    _TEST_TRUE(converts("1e", 1.0, 1));
    _TEST_TRUE(converts("1e+", 1.0, 1));
    _TEST_TRUE(converts("0x1.8p1", 3.0, 7));
    _TEST_TRUE(converts("-0X.8P+2", -2.0, 8));
    _TEST_TRUE(converts("0x", 0.0, 1));
    _TEST_TRUE(converts("0x1p", 1.0, 3));
    _TEST_TRUE(converts("0.1", 0.1, 3));
    _TEST_TRUE(converts("9007199254740993", 9007199254740992.0, 16));
    _TEST_TRUE(converts("9007199254740995", 9007199254740996.0, 16));
    _TEST_TRUE(converts("9007199254740993.0000000000000000001", 9007199254740994.0, 36));
    _TEST_TRUE(converts("1.7976931348623157e308", 0x1.fffffffffffffp1023, 22));
    _TEST_TRUE(converts("", 0.0, 0));
    _TEST_TRUE(converts("abc", 0.0, 0));
    _TEST_TRUE(converts(".", 0.0, 0));
    _TEST_TRUE(converts("-", 0.0, 0));
    _TEST_TRUE(converts("+.e1", 0.0, 0));
    _TEST_TRUE(converts("infinit", 1.0 / 0.0, 3));
    _TEST_TRUE(converts("-INFINITY!", -1.0 / 0.0, 9));
    value = _Fp_ToDouble("nan(12)x", _FP_TEXT_NARROW, &end);
    _TEST_TRUE(value != value);
    _TEST_UINT(end, 7);
    value = _Fp_ToDouble("NAN(", _FP_TEXT_NARROW, &end);
    _TEST_TRUE(value != value);
    _TEST_UINT(end, 3);
    _TEST_INT(_Err_Errno, 0);
    _TEST_TRUE(converts("1e400", 1.0 / 0.0, 5));
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_TRUE(converts("-1e-400", -0.0, 7));
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_TRUE(converts("4.9406564584124654e-324", 0x1p-1074, 23));
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
}

int main(void)
{
    Test_FpDoubles();
    return _Test_Status();
}
