// (Test) Status: 200
// bug003. A switch compared its case constants with all 64 bits of the
// controlling expression, unconverted to its promoted type, so `case -1` never
// matched an unsigned switch, and `case 0x100000000L` matched a long 0.

int wide(long x)
{
    switch (x) {
        case 0x100000000L: return 1;
        case 0: return 2;
    }
    return 3;
}

int negative(unsigned x)
{
    switch (x) {
        case -1: return 1;
    }
    return 2;
}

int large(unsigned x)
{
    switch (x) {
        case 4000000000u: return 1;
    }
    return 2;
}

int main()
{
    if (wide(0) != 2 || wide(0x100000000L) != 1) return 1;
    if (negative(4294967295u) != 1) return 2;
    if (large(4000000000u) != 1) return 3;
    return 200;
}
