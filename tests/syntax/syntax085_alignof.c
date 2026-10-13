// (Test) Status: 42
// [C11 S6.5.3.4p3] _Alignof, C11's, as gcc takes it under C99. It yields a
// type's alignment requirement, a size_t and an integer constant, its operand
// not evaluated, and for an array type the alignment of the element type.

struct Mixed {
    char      mi_small;
    long long mi_large;
};

union Either {
    char   ei_bytes[3];
    double ei_real;
};

enum {
    INT_ALIGN = _Alignof(int)
};

char sized[_Alignof(long double)];

_Static_assert(_Alignof(char) == 1, "a char should be on any address");
_Static_assert(_Alignof(struct Mixed) == 8, "a struct should take its widest");
_Static_assert(_Alignof(union Either) == 8, "a union should take its widest");
_Static_assert(_Alignof(char[100]) == 1, "an array should take its element's");
_Static_assert(_Alignof(struct Mixed[3]) == 8, "an array of structs too");
_Static_assert(sizeof(_Alignof(char)) == sizeof(sizeof(char)), "a size_t");

int main(void)
{
    int n = 5;
    int sum = 0;

    sum += _Alignof(char) + _Alignof(short) + _Alignof(int);
    sum += _Alignof(long) + _Alignof(void *) + _Alignof(double);
    switch (_Alignof(int)) {
        case INT_ALIGN: {
            sum += 1;
        } break;
    }
    sum += _Alignof(int[n++]);
    return sum + (int) sizeof(sized) + n - 15;
}
