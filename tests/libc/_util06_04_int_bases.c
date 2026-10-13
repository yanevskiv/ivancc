// (Test) Status: 0
// A test file for <_int.h> bases refused.

// Include helpers.
#include "_util06_00_int_helpers.inc"

// A base under 0, of 1 or past 36 should read nothing and set EINVAL.
static void Test_IntBases(void)
{
    _Err_Errno = 0;
    _TEST_TRUE(reads("11", 2, 3, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_TRUE(reads("11", 36, 37, 2, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_INT(_Err_Errno, 0);
    _TEST_TRUE(reads("11", 1, 0, 0, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_INT(_Err_Errno, _SYS_EINVAL);
    _Err_Errno = 0;
    _TEST_TRUE(reads("11", -1, 0, 0, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_INT(_Err_Errno, _SYS_EINVAL);
    _Err_Errno = 0;
    _TEST_TRUE(reads("11", 37, 0, 0, _INT_SIGN_PLUS, _INT_RANGE_INSIDE));
    _TEST_INT(_Err_Errno, _SYS_EINVAL);
}

int main(void)
{
    Test_IntBases();
    return _Test_Status();
}
