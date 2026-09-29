// (Test) Status: 200
// Floating literals. A decimal literal needs a point or an exponent, and a
// hexadecimal one needs a binary exponent after `p`. With no suffix a literal
// is a double; `f` makes it a float and `l` a long double, each rounded once,
// straight from its digits, to the precision of its own type.

double third = 1.0 / 3;
float quarter = 0x1p-2f;
long double tenth = 0.1L;
double folded = (1.5 + 2.5) * 4 - -1.0;
int truncated = (int) 2.99 + (int) -2.99;
char small = 65.9;

int main()
{
    // The same value, spelled every decimal way.
    if (1.5 != 15e-1) return 1;
    if (1.5 != .15e1) return 2;
    if (1.5 != 150.0E-2) return 3;
    if (1.5 != 0.0015e+3) return 4;
    if (5. != 5) return 5;
    if (.5 != 0.5) return 6;
    if (1e3 != 1000) return 7;
    if (1E0 != 1) return 8;

    // Hexadecimal digits scaled by a power of two.
    if (0x1p0 != 1) return 9;
    if (0x1.8p1 != 3.0) return 10;
    if (0X10P-4 != 1.0) return 11;
    if (0x.8p1 != 1.0) return 12;
    if (0xAp0 != 10) return 13;
    if (0x1.fp3 != 15.5) return 14;
    if (0x1p-1074 == 0) return 15;
    if (0x1p-1075 != 0) return 16;

    // The suffix picks the type, and the type picks the size.
    if (sizeof(1.0) != 8) return 17;
    if (sizeof(1.0f) != 4) return 18;
    if (sizeof(1.0F) != 4) return 19;
    if (sizeof(1.0l) != 16) return 20;
    if (sizeof(1.0L) != 16) return 21;
    if (sizeof(0x1p0f) != 4) return 22;
    if (sizeof(1e0L) != 16) return 23;

    // A literal rounds to its own type, not to a wider one first.
    if (0.1f == 0.1) return 24;
    if (0.1f != (float) 0.1) return 25;
    if (0.1L == 0.1) return 26;
    if (16777217.0f != 16777216.0f) return 27;
    if (9007199254740993.0 != 9007199254740992.0) return 28;
    if (9007199254740993.0L == 9007199254740992.0L) return 29;

    // A literal too large for its type is infinite.
    if (1e39f != 1e39f * 2) return 30;
    if (1e309 <= 1.7976931348623157e308) return 31;
    if (1e309L == 1e309L * 2) return 32;

    // Constant expressions fold at compile time with the same rounding.
    if (third != 1.0 / 3) return 33;
    if (third * 3 != 1) return 34;
    if (quarter != 0.25) return 35;
    if (tenth != 0.1L) return 36;
    if (folded != 17) return 37;
    if (truncated != 0) return 38;
    if (small != 'A') return 39;

    return 200;
}
