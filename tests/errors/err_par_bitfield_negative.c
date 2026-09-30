// (Test) Compiler error: [ERR_PAR_BITFIELD_NEGATIVE]
// A bit-field of negative width.

struct S { int a : -1; };

int main(void)
{
    return 0;
}
