// (Test) Status: 0
// (Test) Output:
// | 0 -7 [#]
// | 0 -7 [#]
// | 0 -7 [#]
// | 0 -7 [#]
// | 2 0 [x]
// | 2 1 [5]
// Standard: scanf should fail a number its width cuts inside an exponent or 0x.
// Unlike: [C99/S7.19.6.2p9] glibc stores what came before, and reads no x that a
// width leaves no digit after, so no libc test holds this.

#include <stdio.h>

int main(void)
{
    int count;
    char ch = '#';
    double value = -7;

    count = sscanf("-1e5", "%3lf%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    count = sscanf("1e5", "%2lf%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    count = sscanf("0x1p3", "%2lf%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    count = sscanf("0x1p3", "%4la%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    count = sscanf("0x1p3", "%1lf%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    count = sscanf("1e5", "%1lf%*c%c", &value, &ch);
    printf("%d %g [%c]\n", count, value, ch);
    return 0;
}
