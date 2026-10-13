// (Test) Status: 0
// (Test) Output:
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 0 -7
// | 1 100 [e]
// Standard: scanf should fail a number that ends inside its exponent or 0x.
// Unlike: [C99/S7.19.6.2p10] glibc stores what came before, so no libc test holds
// this, though [C99/S7.19.6.2p20] fails 100ergs so.

#include <stdio.h>

int main(void)
{
    int count;
    int n = -7;
    char ch = '#';
    double value = -7;

    count = sscanf("1e+", "%lf", &value);
    printf("%d %g\n", count, value);
    count = sscanf("1e", "%lf", &value);
    printf("%d %g\n", count, value);
    count = sscanf("100ergs", "%lf", &value);
    printf("%d %g\n", count, value);
    count = sscanf("0x1p", "%lf", &value);
    printf("%d %g\n", count, value);
    count = sscanf("0x.", "%la", &value);
    printf("%d %g\n", count, value);
    count = sscanf("0e", "%lg", &value);
    printf("%d %g\n", count, value);
    count = sscanf("0x1p-z", "%le", &value);
    printf("%d %g\n", count, value);
    count = sscanf("-.5E-", "%lF", &value);
    printf("%d %g\n", count, value);
    count = sscanf("1e+", "%*f%n", &n);
    printf("%d %d\n", count, n);
    count = sscanf("100 ergs", "%lf %c", &value, &ch);
    printf("%d %g [%c]\n", count - 1, value, ch);
    return 0;
}
