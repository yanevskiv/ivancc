// (Test) Status: 0
// (Test) Output:
// | [x] 3
// | [xy] 4
// | [] 2
// | [ -1
// Non-standard: printf should not convert a wide character past the precision.
// Liberty: C99 has the array converted, and its end not read.
// [C99:7.19.6.1p8]: printf's conversion specifiers

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    const wchar_t wide[] = {L'x', L'y', 0xe9, L'\0'};

    printf(" %d\n", printf("[%.1ls]", wide));
    printf(" %d\n", printf("[%.2ls]", wide));
    printf(" %d\n", printf("[%.0ls]", wide + 2));
    printf(" %d\n", printf("[%.3ls]", wide));
    return 0;
}
