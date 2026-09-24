// (Test) Return: 200
// Signed and unsigned types. The bits are the same; what differs is how a
// division, a shift, a comparison and a widening read them. Where operands of
// both kinds meet, the usual arithmetic conversions decide which one wins.

unsigned int gu;
unsigned char gc;

unsigned int half(unsigned int n)
{
    return n / 2;
}

int main()
{
    unsigned int u;
    unsigned char uc;
    unsigned short us;
    unsigned long ul;
    signed char sc;
    int i;

    if (sizeof(unsigned char) != 1) return 1;
    if (sizeof(unsigned short) != 2) return 2;
    if (sizeof(unsigned int) != 4) return 3;
    if (sizeof(unsigned long) != 8) return 4;
    if (sizeof(unsigned) != 4) return 5;
    if (sizeof(signed char) != 1) return 6;

    // The keywords may arrive in any order and still name one type.
    if (sizeof(long unsigned int) != 8) return 7;
    if (sizeof(unsigned long long) != 8) return 8;
    if (sizeof(int unsigned) != 4) return 9;

    // A narrow type widens by its own signedness, not by the bit pattern.
    uc = 200;
    sc = -56;
    if (uc != 200) return 10;
    if (sc != -56) return 11;
    if (uc + 0 != 200) return 12;

    us = 65535;
    if (us != 65535) return 13;
    if (us + 1 != 65536) return 14;

    // An unsigned int wraps at 2^32 rather than going negative.
    u = 4294967295;
    if (u != 4294967295) return 15;
    u = u + 1;
    if (u != 0) return 16;
    u = u - 1;
    if (u != 4294967295) return 17;

    // Division and shifting read the top bit as a value, not as a sign.
    u = 4294967295;
    if (u / 2 != 2147483647) return 18;
    if (u >> 1 != 2147483647) return 19;
    if (u % 7 != 3) return 20;

    i = -1;
    if (i / 2 != 0) return 21;
    if (i >> 1 != -1) return 22;

    // The same bits compare differently once the type differs.
    if (! (u > 5)) return 23;
    if (u < 5) return 24;
    if (! (i < 5)) return 25;

    ul = 18446744073709551615;
    if (ul <= 100) return 26;
    if (! (ul > 100)) return 27;
    if (ul / 2 != 9223372036854775807) return 28;

    // An unsigned operand drags a signed one of the same rank along with it.
    i = -1;
    u = 1;
    if (i < u) return 29;
    if (! (i > u)) return 30;

    // A long is wide enough to hold every unsigned int, so it wins instead.
    long l = -1;
    if (! (l < u)) return 31;

    unsigned int diff = 3;
    if (diff - 5 != 4294967294) return 32;

    gu = 4000000000;
    gc = 250;
    if (gu != 4000000000) return 33;
    if (gc != 250) return 34;
    if (gu / 4 != 1000000000) return 35;

    if (half(4294967294) != 2147483647) return 36;

    // A cast is what narrows, and it narrows by the target's signedness.
    i = 300;
    if ((unsigned char) i != 44) return 37;
    if ((signed char) i != 44) return 38;
    i = -1;
    if ((unsigned char) i != 255) return 39;
    if ((unsigned short) i != 65535) return 40;
    if ((unsigned int) i != 4294967295) return 41;
    if ((char) i != -1) return 42;

    unsigned char arr[3];
    arr[0] = 255;
    arr[1] = 128;
    arr[2] = 1;
    if (arr[0] != 255 || arr[1] != 128) return 43;
    if (arr[0] + arr[1] != 383) return 44;

    u = 0xFFFFFFFF;
    if (u != 4294967295) return 45;
    if (0xFFFFFFFFu / 5 != 858993459) return 46;

    struct Bits {
        unsigned int a : 4;
        int b : 4;
    };
    struct Bits bt;
    bt.a = 15;
    bt.b = 15;
    if (bt.a != 15) return 47;
    if (bt.b != -1) return 48;

    u = 21;
    if (u + 21 != 42) return 49;
    return 200;
}
