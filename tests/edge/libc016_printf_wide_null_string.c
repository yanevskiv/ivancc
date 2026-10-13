// (Test) Status: 0
// (Test) Output:
// | [(null)] 8
// | [    (null)] 12
// | [(null)    ] 12
// | [(null)] 8
// | [] 2
// | [        ] 10
// Non-standard: printf should write a null wide string as %s writes a null one.
// Liberty: (S7.19.6.1p8) %ls shall have an array's pointer, so it is undefined.

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    wchar_t *wide = NULL;

    printf(" %d\n", printf("[%ls]", wide));
    printf(" %d\n", printf("[%10ls]", wide));
    printf(" %d\n", printf("[%-10ls]", wide));
    printf(" %d\n", printf("[%.6ls]", wide));
    printf(" %d\n", printf("[%.5ls]", wide));
    printf(" %d\n", printf("[%8.5ls]", wide));
    return 0;
}
