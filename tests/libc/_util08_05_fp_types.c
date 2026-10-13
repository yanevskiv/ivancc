// (Test) Status: 0
// A test file for <_fp.h> floats, long doubles and wide text converted.

// Include helpers.
#include "_util08_00_fp_helpers.inc"

// A number should round to its own type's mantissa and range, not a double's.
static void Test_FpTypes(void)
{
    const int wide[] = { ' ', '-', '2', '.', '5', 'e', '1', 0x20AC, 0 };
    unsigned long end = 0;

    _Err_Errno = 0;
    _TEST_FLOAT(_Fp_ToFloat("16777217", _FP_TEXT_NARROW, &end), 16777216.0F);
    _TEST_UINT(end, 8);
    _TEST_FLOAT(_Fp_ToFloat("16777219", _FP_TEXT_NARROW, &end), 16777220.0F);
    _TEST_FLOAT(_Fp_ToFloat("0.1", _FP_TEXT_NARROW, &end), 0.1F);
    _TEST_FLOAT(_Fp_ToFloat("0x1.fffffep127", _FP_TEXT_NARROW, &end), 0x1.fffffep127F);
    _TEST_FLOAT(_Fp_ToLongDouble("0.1", _FP_TEXT_NARROW, &end), 0.1L);
    _TEST_FLOAT(_Fp_ToLongDouble("18446744073709551617", _FP_TEXT_NARROW, &end), 18446744073709551616.0L);
    _TEST_UINT(end, 20);
    _TEST_FLOAT(_Fp_ToLongDouble("0x1.fffffffffffffffep16383", _FP_TEXT_NARROW, &end), 0x1.fffffffffffffffep16383L);
    _TEST_FLOAT(_Fp_ToLongDouble("1e400", _FP_TEXT_NARROW, &end), 1e400L);
    _TEST_FLOAT(_Fp_ToDouble(wide, _FP_TEXT_WIDE, &end), -25.0);
    _TEST_UINT(end, 7);
    _TEST_FLOAT(_Fp_ToFloat(wide, _FP_TEXT_WIDE, &end), -25.0F);
    _TEST_FLOAT(_Fp_ToLongDouble(wide, _FP_TEXT_WIDE, &end), -25.0L);
    _TEST_INT(_Err_Errno, 0);
    _TEST_FLOAT(_Fp_ToFloat("3.5e38", _FP_TEXT_NARROW, &end), 1.0F / 0.0F);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(_Fp_ToFloat("1e-50", _FP_TEXT_NARROW, &end), 0.0F);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_FLOAT(_Fp_ToLongDouble("1e5000", _FP_TEXT_NARROW, &end), 1.0L / 0.0L);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
}

int main(void)
{
    Test_FpTypes();
    return _Test_Status();
}
