// (Test) Status: 0
// (Test) Output:
// | [nan|nan|nan] 13
// | [-nan|-nan|-nan] 16
// | [-NAN|-NAN|-NAN] 16
// | [+nan| nan] 11
// | [-nan|-nan] 11
// | [nan|-nan] 10
// Non-standard: printf should write a NaN as nan, after - if its sign is set.
// Liberty: (S7.19.6.1p8) a NaN's style is implementation-defined.

#include <stdio.h>
#include <string.h>

int main(void)
{
    double pos;
    double neg;
    unsigned long plus = 0x7ff8000000000000UL;
    unsigned long minus = 0xfff8000000000000UL;

    memcpy(&pos, &plus, sizeof(pos));
    memcpy(&neg, &minus, sizeof(neg));
    printf(" %d\n", printf("[%f|%e|%g]", pos, pos, pos));
    printf(" %d\n", printf("[%f|%e|%g]", neg, neg, neg));
    printf(" %d\n", printf("[%F|%E|%G]", neg, neg, neg));
    printf(" %d\n", printf("[%+f|% f]", pos, pos));
    printf(" %d\n", printf("[%+f|% f]", neg, neg));
    printf(" %d\n", printf("[%Lf|%Lf]", (long double) pos, (long double) neg));
    return 0;
}
