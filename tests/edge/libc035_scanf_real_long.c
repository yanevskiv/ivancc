// (Test) Status: 0
// (Test) Output:
// | 1 1 10001 [,]
// | 1 1e-3999 10006 [,]
// | 1 inf 10004 [,]
// Non-standard: scanf should read a number of ten thousand digits whole.
// Liberty: [C99:7.19.6.2p9] an item has no greatest length, and an
// implementation may run out of room for one.

#include <stdio.h>
#include <string.h>

#define DIGITS 10000
#define FIRST  4000

int main(void)
{
    int count;
    int n = -7;
    char ch = '#';
    long double value = -7;
    static char input[DIGITS + 16];

    memset(input, '0', DIGITS);
    strcpy(input + DIGITS, "1,");
    count = sscanf(input, "%Lf%n%c", &value, &n, &ch);
    printf("%d %Lg %d [%c]\n", count - 1, value, n, ch);
    input[0] = '.';
    input[FIRST] = '1';
    strcpy(input + DIGITS, "e+0001,");
    count = sscanf(input, "%Lf%n%c", &value, &n, &ch);
    printf("%d %Lg %d [%c]\n", count - 1, value, n, ch);
    memset(input, '9', DIGITS);
    strcpy(input + DIGITS, "e+00,");
    count = sscanf(input, "%Lf%n%c", &value, &n, &ch);
    printf("%d %Lg %d [%c]\n", count - 1, value, n, ch);
    return 0;
}
