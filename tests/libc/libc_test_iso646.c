// (Test) Status: 0
// <iso646.h> spells the operators as macros, 7.9.

#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>

int main(void)
{
    int x = 6;

    if (not (1 and 2)) return 1;
    if (0 or 0) return 2;
    if (not 1) return 3;
    if ((x bitand 3) != 2) return 4;
    if ((x bitor 3) != 7) return 5;
    if ((x xor 3) != 5) return 6;
    if ((compl 0) != -1) return 7;
    if (1 not_eq 1) return 8;
    x and_eq 3;
    if (x != 2) return 9;
    x or_eq 5;
    if (x != 7) return 10;
    x xor_eq 2;
    if (x != 5) return 11;
    return 0;
}
