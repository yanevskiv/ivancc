// (Test) Status: 0
// (Test) Output:
// | [~] 3
// | [ -1
// | [ -1
// | [ -1
// | [|ab] 5
// Non-standard: printf should hold "C" to ASCII, and write a tag as no byte.
// Liberty: [C99:S5.2.1p1] a locale's extended characters are locale-specific.

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    const wchar_t euro[] = {0x20ac, L'\0'};
    const wchar_t tagged[] = {L'a', 0xe0041, L'b', L'\0'};

    printf(" %d\n", printf("[%lc]", 0x7e));
    printf(" %d\n", printf("[%lc]", 0x80));
    printf(" %d\n", printf("[%lc]", 0xe9));
    printf(" %d\n", printf("[%ls]", euro));
    printf(" %d\n", printf("[%lc|%ls]", 0xe0041, tagged));
    return 0;
}
