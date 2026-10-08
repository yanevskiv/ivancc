// (Test) Status: 0
// <stdbool.h> declares bool, true and false (S7.16).

#include <stddef.h>
#include <stdbool.h>

int main(void)
{
    bool yes = true;
    bool no = false;
    bool converted = 42;

    if (!__bool_true_false_are_defined) return 1;
    if (true != 1) return 2;
    if (false != 0) return 3;
    if (yes != 1) return 4;
    if (no != 0) return 5;
    if (converted != 1) return 6;
    if (sizeof(bool) != sizeof(_Bool)) return 7;
    return 0;
}
