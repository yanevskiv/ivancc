// (Test) Status: 0
// (Test) Output:
// | abc[] 5 4
// | abcd[] 6 5
// | abcde[] 7 6
// | abcdef[|7] 10 7
// | abcdefg[|7] 11 8
// Non-standard: %n should take flags, a width and a precision, and ignore them.
// Liberty: (S7.19.6.1p8) %n with any of them is undefined behaviour.

#include <stdio.h>

int main(void)
{
    int count = -1;

    printf(" %d", printf("abc[%5n]", &count));
    printf(" %d\n", count);
    printf(" %d", printf("abcd[%-+ #0n]", &count));
    printf(" %d\n", count);
    printf(" %d", printf("abcde[%.3n]", &count));
    printf(" %d\n", count);
    printf(" %d", printf("abcdef[%*n|%d]", 9, &count, 7));
    printf(" %d\n", count);
    printf(" %d", printf("abcdefg[%*.*n|%d]", 9, 9, &count, 7));
    printf(" %d\n", count);
    return 0;
}
