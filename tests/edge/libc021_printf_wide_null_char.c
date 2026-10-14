// (Test) Status: 0
// (Test) Output:
// | [] 2
// | [   ] 5
// | [   ] 5
// | [ab] 4
// Standard: printf should write %lc of a null wide character as no byte.
// Unlike: glibc writes a null byte, so no libc test holds this.
// [C99:7.19.6.1p8]: printf's conversion specifiers

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    printf(" %d\n", printf("[%lc]", L'\0'));
    printf(" %d\n", printf("[%3lc]", L'\0'));
    printf(" %d\n", printf("[%-3lc]", L'\0'));
    printf(" %d\n", printf("[%lc%lc%lc]", L'a', L'\0', L'b'));
    return 0;
}
