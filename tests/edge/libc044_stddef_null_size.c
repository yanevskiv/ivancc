// (Test) Status: 0
// (Test) Output:
// | 8 8
// Non-standard: NULL should be a pointer to void and never an integer zero.
// Liberty: which null pointer constant NULL expands to is
// implementation-defined.
// [C99:7.17p3]: NULL and offsetof

#include <stdio.h>
#include <stddef.h>

int main(void)
{
    printf("%d %d\n", (int) sizeof(NULL), (int) sizeof(void *));
    return 0;
}
