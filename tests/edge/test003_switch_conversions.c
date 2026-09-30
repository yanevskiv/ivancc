// (Test) Status: 200
// Switches on every integer type. Each case constant is converted to the
// promoted type of the controlling expression, so it may wrap or never match,
// and the constants that need all 64 bits are compared in full.

int on_uchar(unsigned char c)
{
    switch (c) {
        case -1: return 1;
        case 255: return 2;
        case 256: return 3;
    }
    return 4;
}

int on_schar(signed char c)
{
    switch (c) {
        case 255: return 1;
        case -1: return 2;
    }
    return 3;
}

int on_ushort(unsigned short s)
{
    switch (s) {
        case -1: return 1;
        case 65535: return 2;
    }
    return 3;
}

int on_bool(_Bool b)
{
    switch (b) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
    }
    return 4;
}

int on_int(int i)
{
    switch (i) {
        case 0x100000001L: return 1;
        case 4294967295u: return 2;
        case -2147483647 - 1: return 3;
        case 2147483647: return 4;
    }
    return 5;
}

int on_unsigned(unsigned u)
{
    switch (u) {
        case -2: return 1;
        case 0x80000000: return 2;
        case 0x1ffffffffL: return 3;
        case 0: return 4;
    }
    return 5;
}

int on_long(long l)
{
    switch (l) {
        case 0x100000000L: return 1;
        case -0x100000000L: return 2;
        case 4294967295u: return 3;
        case -1: return 4;
        case 0x7fffffffffffffffL: return 5;
        case -0x7fffffffffffffffL - 1: return 6;
        case 2147483648L: return 7;
        case -2147483647L - 1: return 8;
    }
    return 9;
}

int on_ulong(unsigned long l)
{
    switch (l) {
        case -1: return 1;
        case 0x8000000000000000UL: return 2;
        case 4294967295u: return 3;
        case -4294967296L: return 4;
    }
    return 5;
}

int nested(long outer, unsigned inner)
{
    switch (outer) {
        case 0x100000000L:
            switch (inner) {
                case -1: return 1;
                default: return 2;
            }
        case 0: return 3;
        default: return 4;
    }
}

int check_narrow(void)
{
    if (on_uchar(255) != 2 || on_uchar(0) != 4) return 1;
    if (on_schar(-1) != 2 || on_schar(1) != 3) return 2;
    if (on_ushort(65535) != 2 || on_ushort(1) != 3) return 3;
    if (on_bool(0) != 1 || on_bool(1) != 2) return 4;
    return 0;
}

int check_int(void)
{
    if (on_int(1) != 1 || on_int(-1) != 2 || on_int(-2147483647 - 1) != 3 || on_int(2147483647) != 4) return 11;
    if (on_int(0) != 5) return 12;
    if (on_unsigned(4294967294u) != 1 || on_unsigned(0x80000000u) != 2 || on_unsigned(4294967295u) != 3) return 13;
    if (on_unsigned(0) != 4 || on_unsigned(1) != 5) return 14;
    return 0;
}

int check_long(void)
{
    if (on_long(0x100000000L) != 1 || on_long(-0x100000000L) != 2 || on_long(4294967295L) != 3) return 21;
    if (on_long(-1) != 4 || on_long(0x7fffffffffffffffL) != 5 || on_long(-0x7fffffffffffffffL - 1) != 6) return 22;
    if (on_long(2147483648L) != 7 || on_long(-2147483647L - 1) != 8 || on_long(0) != 9) return 23;
    if (on_ulong(0xffffffffffffffffUL) != 1 || on_ulong(0x8000000000000000UL) != 2) return 24;
    if (on_ulong(4294967295u) != 3 || on_ulong(0xffffffff00000000UL) != 4 || on_ulong(0) != 5) return 25;
    if (nested(0x100000000L, 4294967295u) != 1 || nested(0x100000000L, 0) != 2) return 26;
    if (nested(0, 4294967295u) != 3 || nested(1, 0) != 4) return 27;
    return 0;
}

int main()
{
    int r;

    if ((r = check_narrow()) != 0) return r;
    if ((r = check_int()) != 0) return r;
    if ((r = check_long()) != 0) return r;
    return 200;
}
