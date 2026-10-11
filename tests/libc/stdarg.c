// (Test) Status: 0
// <stdarg.h> declares va_list and defines va_start, va_arg, va_copy and va_end (S7.15p1-p2), which walk the arguments a function takes past its last parameter (S7.15.1.4p4):
// va_arg yielding each in turn as its type after the default argument promotions (S7.15.1.1p2), a struct and a long double among them, however many there are,
// va_copy making a list that stands where its source does and walks on apart from it (S7.15.1.2p2), va_start walking the arguments again after va_end (S7.15.1.4p3),
// and a list passed on to a function that walks it (S7.15p3), by a pointer too, where its owner then walks on from where that function stopped.

#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>

#define MISMATCH (-1)
#define HALVES   2
#define SEVEN    7
#define WIDE     3
#define BASE     10

struct Pair {
    int pa_first;
    long pa_second;
};

struct Wide {
    long wi_numbers[WIDE];
};

// Return the sum of count int arguments.
static int sum(int count, ...)
{
    int total = 0;
    va_list ap;

    va_start(ap, count);
    while (count > 0) {
        total += va_arg(ap, int);
        count--;
    }
    va_end(ap);
    return total;
}

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

// A list and its copy should agree: return the sum of the ints past the first.
static int rest(int count, ...)
{
    int copied = 0;
    int total = 0;
    va_list copy;
    va_list ap;
    int i;

    va_start(ap, count);
    (void) va_arg(ap, int);
    va_copy(copy, ap);
    for (i = 1; i < count; i++) {
        total += va_arg(ap, int);
    }
    va_end(ap);
    for (i = 1; i < count; i++) {
        copied += va_arg(copy, int);
    }
    va_end(copy);
    return copied == total ? total : MISMATCH;
}

// Two walks should agree: return the sum of count int arguments.
static int again(int count, ...)
{
    int second = 0;
    int first = 0;
    va_list ap;
    int i;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        first += va_arg(ap, int);
    }
    va_end(ap);
    va_start(ap, count);
    for (i = 0; i < count; i++) {
        second += va_arg(ap, int);
    }
    va_end(ap);
    return first == second ? first : MISMATCH;
}

// Return the sum of the count int arguments a list still holds.
static int walk(va_list ap, int count)
{
    int total = 0;

    while (count > 0) {
        total += va_arg(ap, int);
        count--;
    }
    return total;
}

// Return the sum of count int arguments, walked by another function.
static int forward(int count, ...)
{
    va_list ap;
    int total;

    va_start(ap, count);
    total = walk(ap, count);
    va_end(ap);
    return total;
}

// Return the next int argument of a list its owner walks on with.
static int take(va_list *ap)
{
    return va_arg(*ap, int);
}

// Return the int arguments as decimal digits, each other one taken apart.
static long shared(int count, ...)
{
    long digits = 0;
    va_list ap;
    int i;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        digits = digits * BASE + (i % 2 == 0 ? take(&ap) : va_arg(ap, int));
    }
    va_end(ap);
    return digits;
}

// (S7.15.1.1p2) va_arg should yield each int in turn, in registers or not.
static void Test_StdargInts(void)
{
    _TEST_INT(sum(0), 0);
    _TEST_INT(sum(1, SEVEN), SEVEN);
    _TEST_INT(sum(3, 1, 2, 3), 6);
    _TEST_INT(sum(5, 1, 2, 3, 4, 5), 15);
    _TEST_INT(sum(6, 1, 2, 3, 4, 5, 6), 21);
    _TEST_INT(sum(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
    _TEST_INT(sum(3, INT_MAX, INT_MIN, -1), -2);
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

// (S7.15.1.2p2) va_copy's list should stand where its source does, and apart.
static void Test_StdargCopy(void)
{
    _TEST_INT(rest(1, 1), 0);
    _TEST_INT(rest(4, 1, 2, 3, 4), 9);
    _TEST_INT(rest(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 54);
}

// (S7.15.1.4p3) va_start after va_end should walk the same arguments again.
static void Test_StdargAgain(void)
{
    _TEST_INT(again(0), 0);
    _TEST_INT(again(3, 1, 2, 3), 6);
    _TEST_INT(again(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
}

// (S7.15p3) Another function should walk a list it is passed, by pointer too.
static void Test_StdargPassed(void)
{
    _TEST_INT(forward(0), 0);
    _TEST_INT(forward(3, 4, 5, 6), 15);
    _TEST_INT(forward(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
    _TEST_INT(shared(4, 1, 2, 3, 4), 1234);
    _TEST_INT(shared(9, 1, 2, 3, 4, 5, 6, 7, 8, 9), 123456789);
}

int main(void)
{
    Test_StdargInts();
    Test_StdargKinds();
    Test_StdargCopy();
    Test_StdargAgain();
    Test_StdargPassed();
    return _Test_Status();
}
