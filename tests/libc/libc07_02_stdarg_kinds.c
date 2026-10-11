// (Test) Status: 0
// A test file for <stdarg.h> arguments of any type.

// Include helpers.
#include "libc07_00_stdarg_helpers.inc"

#define HALVES 2
#define WIDE   3

struct Pair {
    int pa_first;
    long pa_second;
};

struct Wide {
    long wi_numbers[WIDE];
};

// Return a string's length.
static long length(const char *str)
{
    long len = 0;

    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Return twice the sum of the arguments, one of the kind each letter names.
static long halves(const char *kinds, ...)
{
    long double total = 0;
    struct Pair pair;
    struct Wide wide;
    va_list ap;

    va_start(ap, kinds);
    for (; *kinds != '\0'; kinds++) {
        if (*kinds == 'i') {
            total += va_arg(ap, int);
        } else if (*kinds == 'u') {
            total += va_arg(ap, unsigned int);
        } else if (*kinds == 'l') {
            total += va_arg(ap, long);
        } else if (*kinds == 'q') {
            total += va_arg(ap, long long);
        } else if (*kinds == 'd') {
            total += va_arg(ap, double);
        } else if (*kinds == 'e') {
            total += va_arg(ap, long double);
        } else if (*kinds == 'p') {
            total += *va_arg(ap, int *);
        } else if (*kinds == 's') {
            total += length(va_arg(ap, char *));
        } else if (*kinds == 't') {
            pair = va_arg(ap, struct Pair);
            total += pair.pa_first + pair.pa_second;
        } else if (*kinds == 'w') {
            wide = va_arg(ap, struct Wide);
            total += wide.wi_numbers[0] + wide.wi_numbers[1] + wide.wi_numbers[2];
        }
    }
    va_end(ap);
    return (long) (total * HALVES);
}

// (S7.15.1.1p2) va_arg should yield an argument of any type, as promoted.
static void Test_StdargKinds(void)
{
    struct Wide wide = {{1, 2, 3}};
    struct Pair pair = {1, 2};
    float half = 0.5F;
    short small = -3;
    int seven = SEVEN;
    char ch = 'A';

    _TEST_INT(halves(""), 0);
    _TEST_INT(halves("idl", 1, 2.5, 3L), 13);
    _TEST_INT(halves("pi", &seven, 1), 16);
    _TEST_INT(halves("dddddddddd", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0), 110);
    _TEST_INT(halves("idididididididid", 1, 0.5, 2, 0.5, 3, 0.5, 4, 0.5, 5, 0.5, 6, 0.5, 7, 0.5, 8, 0.5), 80);
    _TEST_INT(halves("iid", ch, small, half), 125);
    _TEST_INT(halves("u", UINT_MAX), 8589934590L);
    _TEST_INT(halves("ql", 1LL << 40, -1L), (1L << 41) - 2);
    _TEST_INT(halves("s", "abc"), 6);
    _TEST_INT(halves("sisi", "", 1, "ab", 2), 10);
    _TEST_INT(halves("e", 2.5L), 5);
    _TEST_INT(halves("ieid", 1, 2.5L, 3, 0.5), 14);
    _TEST_INT(halves("iiiiiiiedei", 1, 2, 3, 4, 5, 6, 7, 0.5L, 1.5, 2.5L, 8), 81);
    _TEST_INT(halves("t", pair), 6);
    _TEST_INT(halves("tit", pair, 1, pair), 14);
    _TEST_INT(halves("iiiiiti", 1, 2, 3, 4, 5, pair, 6), 48);
    _TEST_INT(halves("w", wide), 12);
    _TEST_INT(halves("iwdwi", 1, wide, 0.5, wide, 2), 31);
}

int main(void)
{
    Test_StdargKinds();
    return _Test_Status();
}
