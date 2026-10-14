// (Test) Status: 0
// A test file for <inttypes.h> imaxdiv.

// Include helpers.
#include "libc11_00_inttypes_helpers.inc"

// imaxdiv should give the quotient and the remainder given.
// [C99:7.8.2.2p2]: imaxdiv
static bool divides(intmax_t numer, intmax_t denom, intmax_t quot, intmax_t rem)
{
    int lost = 0;
    imaxdiv_t result = imaxdiv(numer, denom);

    lost += ! _TEST_INT(result.quot, quot);
    lost += ! _TEST_INT(result.rem, rem);
    return lost == 0;
}

// imaxdiv should truncate toward zero, the remainder to match.
// [C99:7.8.2.2p2]: imaxdiv
static void Test_InttypesDivide(void)
{
    _TEST_TRUE(divides(7, 2, 3, 1));
    _TEST_TRUE(divides(-7, 2, -3, -1));
    _TEST_TRUE(divides(7, -2, -3, 1));
    _TEST_TRUE(divides(-7, -2, 3, -1));
    _TEST_TRUE(divides(0, 5, 0, 0));
    _TEST_TRUE(divides(INTMAX_MIN, 1, INTMAX_MIN, 0));
    _TEST_TRUE(divides(INTMAX_MIN, INTMAX_MAX, -1, -1));
    _TEST_TRUE(divides(INTMAX_MAX, INTMAX_MIN, 0, INTMAX_MAX));
}

int main(void)
{
    Test_InttypesDivide();
    return _Test_Status();
}
