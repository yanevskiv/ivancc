// (Test) Status: 0
// A test file for <inttypes.h> imaxdiv_t.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// [C99:S7.8p2] imaxdiv_t should hold a quotient and a remainder, each intmax_t.
static void Test_InttypesDivision(void)
{
    imaxdiv_t result;

    result.quot = INTMAX_MAX;
    result.rem = INTMAX_MIN;
    _TEST_UINT(sizeof(result.quot), sizeof(intmax_t));
    _TEST_UINT(sizeof(result.rem), sizeof(intmax_t));
    _TEST_INT(result.quot, INTMAX_MAX);
    _TEST_INT(result.rem, INTMAX_MIN);
}

int main(void)
{
    Test_InttypesDivision();
    return _Test_Status();
}
