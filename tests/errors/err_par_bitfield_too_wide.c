// (Test) Compiler error: [ERR_PAR_BITFIELD_TOO_WIDE]
// An `int` bit-field of 33 bits.

struct S { int a : 33; };

int main(void)
{
    return 0;
}
