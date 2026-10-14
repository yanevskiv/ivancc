// (Test) Status: 0
// A test file for <stdlib.h> div, ldiv and lldiv and the types they return.

// Include helpers.
#include "libc17_00_stdlib_helpers.inc"

// [C99:S7.20.6.2p3] div_t, ldiv_t and lldiv_t should hold quot and rem of
// the type of div's, ldiv's and lldiv's arguments.
static void Test_StdlibDivisionTypes(void)
{
    div_t d = div(17, 5);
    ldiv_t l = ldiv(17L, -5L);
    lldiv_t ll = lldiv(-17LL, 5LL);
    int *dq = &d.quot;
    int *dr = &d.rem;
    long *lq = &l.quot;
    long *lr = &l.rem;
    long long *llq = &ll.quot;
    long long *llr = &ll.rem;

    _TEST_INT(*dq, 3);
    _TEST_INT(*dr, 2);
    _TEST_INT(*lq, -3);
    _TEST_INT(*lr, 2);
    _TEST_INT(*llq, -3);
    _TEST_INT(*llr, -2);
}

// [C99:S7.20.6.2p2] quot and rem should be numer / denom and numer % denom,
// so quot * denom + rem gives numer back.
static void Test_StdlibDivisionParts(void)
{
    div_t d = div(INT_MAX, -2);
    ldiv_t l = ldiv(LONG_MIN, 3L);
    lldiv_t ll = lldiv(LLONG_MAX, 10LL);

    _TEST_INT(d.quot, INT_MAX / -2);
    _TEST_INT(d.rem, INT_MAX % -2);
    _TEST_TRUE(d.quot * -2 + d.rem == INT_MAX);
    _TEST_INT(l.quot, LONG_MIN / 3L);
    _TEST_INT(l.rem, LONG_MIN % 3L);
    _TEST_TRUE(l.quot * 3L + l.rem == LONG_MIN);
    _TEST_INT(ll.quot, LLONG_MAX / 10LL);
    _TEST_INT(ll.rem, LLONG_MAX % 10LL);
    _TEST_TRUE(ll.quot * 10LL + ll.rem == LLONG_MAX);
}

int main(void)
{
    Test_StdlibDivisionTypes();
    Test_StdlibDivisionParts();
    return _Test_Status();
}
