// (Test) Status: 42
// _Static_assert, C11's, as gcc takes it under C99. It is a declaration: at
// file scope, in a block among statements and declarations, and among a
// struct's or a union's members, where it adds none. Its condition is an
// integer constant expression, and its message one string or several.

struct Pair {
    char pa_small;
    _Static_assert(sizeof(char) == 1, "a char should be a byte");
    int  pa_large;
};

union Either {
    _Static_assert(1, "an assertion should stand first in a union");
    int  ei_number;
    char ei_bytes[4];
};

enum {
    FORTY = 40
};

_Static_assert(1, "an assertion should stand at file scope");
_Static_assert(sizeof(struct Pair) == 2 * sizeof(int), "an assertion should " "add no member");
_Static_assert(sizeof(union Either) == sizeof(int), "nor should it to a union");
_Static_assert(__builtin_offsetof(struct Pair, pa_large) == 4, "an offset should be a constant");
_Static_assert(FORTY + 2 == 42 && (FORTY > 0 ? 1 : 0), "an enumerator should be a constant");

int main(void)
{
    int sum = FORTY;

    _Static_assert(sizeof(sum) == 4, "an assertion should stand before a statement");
    sum += 1;
    _Static_assert(sizeof(int[3]) == 12, "an assertion should stand after a statement");
    for (int i = 0; i < 1; i++) {
        _Static_assert(-1 < 0, "an assertion should stand in a nested block");
        sum += 1;
    }
    return sum;
}
