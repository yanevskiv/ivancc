// (Test) Compiler error: [ERR_SEM_ADDRESS_BITFIELD]
// Can't take the address of a bit-field.

struct S { int a : 3; } s;

int main(void)
{
    int *p = &s.a;

    return *p;
}
