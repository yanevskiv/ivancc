// (Test) Status: 0
// (Test) Output:
// | ab -1
// | ab -1
// | ab -1
// | 1|s| -1
// | ab[x] 5
// Non-standard: printf should write a format up to the directive that fails.
// Liberty: [C99 S7.19.6.1p14] C99 says only that the value returned is negative.

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    const wchar_t wide[] = {L'x', L'y', 0xe9, L'\0'};

    printf(" %d\n", printf("ab%lccd", 0xe9));
    printf(" %d\n", printf("ab%lscd", wide));
    printf(" %d\n", printf("ab%9lccd", 0xe9));
    printf(" %d\n", printf("%d|%s|%lc|%d", 1, "s", 0xe9, 2));
    printf(" %d\n", printf("ab[%lc]", L'x'));
    return 0;
}
