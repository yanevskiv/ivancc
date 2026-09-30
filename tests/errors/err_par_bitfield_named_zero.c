// (Test) Compiler error: [ERR_PAR_BITFIELD_NAMED_ZERO]
// A named bit-field of zero width.

struct S { int a : 0; };

int main(void)
{
    return 0;
}
