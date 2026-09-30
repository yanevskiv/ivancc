// (Test) Compiler error: [ERR_PAR_BITFIELD_NOT_CONSTANT]
// A bit-field width that is a variable.

int n;
struct S { int a : n; };

int main(void)
{
    return 0;
}
