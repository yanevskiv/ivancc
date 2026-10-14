// (Test) Status: 0
// An integer constant right operand is folded and loaded after the left one:
// every operator of gen's binary case, against constants spelled as literals,
// casts, negations, sizeof, enumerators and conditionals, with 64-bit values,
// narrowing casts, and a right operand holding a division, which is not folded.

enum { SEVEN = 7 };

int calls;

long seven(void)
{
    calls++;
    return 7;
}

int minus(void)
{
    calls++;
    return -9;
}

unsigned big(void)
{
    calls++;
    return 4000000000U;
}

long long wide(void)
{
    calls++;
    return 0x123456789LL;
}

int main(void)
{
    int arr[4] = {10, 20, 30, 40};
    int *p = arr;
    volatile unsigned ones = 0xFFFFFFFFU;
    volatile signed char low = -56;
    volatile unsigned char byte = 44;

    if (seven() + 1 != 8 || seven() - 10 != -3 || seven() * -3 != -21) {
        return 1;
    }
    if (minus() / 2 != -4 || minus() % 2 != -1 || big() / 3U != 1333333333U || big() % 7 != 4000000000U % 7) {
        return 2;
    }
    if ((seven() & 3) != 3 || (seven() | 8) != 15 || (seven() ^ -1) != -8) {
        return 3;
    }
    if (! (minus() < 0) || ! (minus() <= -9) || minus() == 9 || ! (minus() != 0)) {
        return 4;
    }
    if (big() < 1U || ! (big() <= 4000000000U) || big() == 0 || ! (big() > 3999999999U)) {
        return 5;
    }
    if (wide() + 0x100000000LL != 0x223456789LL || (wide() & 0xF00000000LL) != 0x100000000LL || ! (wide() < 0x12345678ALL)) {
        return 6;
    }
    if (seven() != (long) 7 || minus() != -(9) || seven() == (-2147483647 - 1) || seven() != SEVEN) {
        return 7;
    }
    if (seven() != sizeof(long) - 1 || seven() != (1 ? 7 : 8) || seven() + 'a' != 104) {
        return 8;
    }
    if (ones != (unsigned) -1 || low != (signed char) 200 || byte != (unsigned char) 300 || big() == (unsigned) -1) {
        return 9;
    }
    if (seven() + 6 / 2 != 10 || minus() != 1 % 1 - 9 || seven() * (8 / 4) != 14) {
        return 10;
    }
    if (*(p + 2) != 30 || p + 3 - 1 != &arr[2] || &arr[3] - 1 != p + 2) {
        return 11;
    }
    if (calls != 32) {
        return 12;
    }
    return 0;
}
