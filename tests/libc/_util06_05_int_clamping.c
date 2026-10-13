// (Test) Status: 0
// A test file for <_int.h> signed and unsigned values made of a magnitude.

// Include helpers.
#include "_util06_00_int_helpers.inc"

// A magnitude out of its type's range should clamp and set ERANGE, no other.
static void Test_IntClamping(void)
{
    unsigned long over = (unsigned long) _INT_SIGNED_MAX + 1;

    _Err_Errno = 0;
    _TEST_INT(_Int_MakeSigned(5, _INT_SIGN_PLUS, _INT_RANGE_INSIDE), 5);
    _TEST_INT(_Int_MakeSigned(5, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), -5);
    _TEST_INT(_Int_MakeSigned(0, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), 0);
    _TEST_INT(_Int_MakeSigned(over - 1, _INT_SIGN_PLUS, _INT_RANGE_INSIDE), _INT_SIGNED_MAX);
    _TEST_INT(_Int_MakeSigned(over, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), _INT_SIGNED_MIN);
    _TEST_UINT(_Int_MakeUnsigned(5, _INT_SIGN_PLUS, _INT_RANGE_INSIDE), 5);
    _TEST_UINT(_Int_MakeUnsigned(5, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), _INT_VALUE_MAX - 4);
    _TEST_UINT(_Int_MakeUnsigned(_INT_VALUE_MAX, _INT_SIGN_PLUS, _INT_RANGE_INSIDE), _INT_VALUE_MAX);
    _TEST_UINT(_Int_MakeUnsigned(_INT_VALUE_MAX, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), 1);
    _TEST_INT(_Err_Errno, 0);
    _TEST_INT(_Int_MakeSigned(over, _INT_SIGN_PLUS, _INT_RANGE_INSIDE), _INT_SIGNED_MAX);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_INT(_Int_MakeSigned(over + 1, _INT_SIGN_MINUS, _INT_RANGE_INSIDE), _INT_SIGNED_MIN);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_INT(_Int_MakeSigned(1, _INT_SIGN_PLUS, _INT_RANGE_OUTSIDE), _INT_SIGNED_MAX);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_INT(_Int_MakeSigned(1, _INT_SIGN_MINUS, _INT_RANGE_OUTSIDE), _INT_SIGNED_MIN);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
    _Err_Errno = 0;
    _TEST_UINT(_Int_MakeUnsigned(1, _INT_SIGN_MINUS, _INT_RANGE_OUTSIDE), _INT_VALUE_MAX);
    _TEST_INT(_Err_Errno, _SYS_ERANGE);
}

int main(void)
{
    Test_IntClamping();
    return _Test_Status();
}
