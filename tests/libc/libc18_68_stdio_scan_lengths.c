// (Test) Status: 0
// A test file for <stdio.h> the length modifiers of scanf's n conversion.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

#define PAIR 2

// (S7.19.6.2p11) %n should store through the type its length modifier names.
static void Test_StdioScanLengths(void)
{
    size_t z = 0;
    long l = NONE;
    intmax_t j = NONE;
    ptrdiff_t t = NONE;
    long long ll = NONE;
    short h[PAIR] = {NONE, NONE};
    signed char hh[PAIR] = {NONE, NONE};

    _TEST_INT(sscanf("abcdefg", "a%hhnb%hnc%lnd%llne%jnf%zng%tn", hh, h, &l, &ll, &j, &z, &t), 0);
    _TEST_INT(hh[0], 1);
    _TEST_INT(hh[1], NONE);
    _TEST_INT(h[0], 2);
    _TEST_INT(h[1], NONE);
    _TEST_INT(l, 3);
    _TEST_INT(ll, 4);
    _TEST_INT(j, 5);
    _TEST_UINT(z, 6);
    _TEST_INT(t, 7);
}

int main(void)
{
    Test_StdioScanLengths();
    return _Test_Status();
}
