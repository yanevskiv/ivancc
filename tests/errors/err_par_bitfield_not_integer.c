// (Test) Compiler error: [ERR_PAR_BITFIELD_NOT_INTEGER]
// A bit-field of a floating type.

struct S { double a : 3; };

int main(void)
{
    return 0;
}
