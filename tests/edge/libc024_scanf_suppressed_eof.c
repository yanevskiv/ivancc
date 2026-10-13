// (Test) Status: 0
// (Test) Output:
// | -1
// | -1
// | 3
// | -1
// | -1
// | -1
// | 1
// | [one]
// Non-standard: scanf should return EOF if input ends before any assignment.
// Liberty: (S7.19.6.2p16) read as glibc does, a conversion that stores counted.

#include <stdio.h>

int main(void)
{
    int n = -7;
    char got[16] = "#";

    printf("%d\n", sscanf("one", "%*s%s", got));
    printf("%d\n", sscanf("one", "%*s%n%s", &n, got));
    printf("%d\n", n);
    printf("%d\n", sscanf("one", "%*s x"));
    printf("%d\n", sscanf("one", "%*3c%c", got));
    printf("%d\n", sscanf("one", "one%s", got));
    printf("%d\n", sscanf("one", "%s%s", got, got));
    printf("[%s]\n", got);
    return 0;
}
