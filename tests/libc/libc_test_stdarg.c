// (Test) Status: 0
// <stdarg.h> declares va_list and the macros that walk a variable argument list, 7.15.

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>

static int sum(int count, ...)
{
    va_list ap;
    int total = 0;

    va_start(ap, count);
    while (count-- > 0) total += va_arg(ap, int);
    va_end(ap);
    return total;
}

static int twice(int count, ...)
{
    va_list ap;
    va_list copy;
    int first = 0;
    int second = 0;

    va_start(ap, count);
    va_copy(copy, ap);
    while (count-- > 0) {
        first += va_arg(ap, int);
        second += va_arg(copy, int);
    }
    va_end(copy);
    va_end(ap);
    return first == second ? first : -1;
}

static double mixed(const char *kinds, ...)
{
    va_list ap;
    double total = 0;

    va_start(ap, kinds);
    for (; *kinds != '\0'; kinds++) {
        if (*kinds == 'i') total += va_arg(ap, int);
        else if (*kinds == 'l') total += va_arg(ap, long);
        else if (*kinds == 'd') total += va_arg(ap, double);
        else if (*kinds == 'p') total += *va_arg(ap, int *);
    }
    va_end(ap);
    return total;
}

static int forward(va_list ap, int count)
{
    int total = 0;

    while (count-- > 0) total += va_arg(ap, int);
    return total;
}

static int outer(int count, ...)
{
    va_list ap;
    int total;

    va_start(ap, count);
    total = forward(ap, count);
    va_end(ap);
    return total;
}

int main(void)
{
    int seven = 7;

    if (sum(0) != 0) return 1;
    if (sum(3, 1, 2, 3) != 6) return 2;
    if (sum(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) != 55) return 3;
    if (twice(4, 1, 2, 3, 4) != 10) return 4;
    if (mixed("idl", 1, 2.5, 3L) != 6.5) return 5;
    if (mixed("pi", &seven, 1) != 8) return 6;
    if (mixed("dddddddddd", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) != 55.0) return 7;
    if (outer(3, 4, 5, 6) != 15) return 8;
    return 0;
}
