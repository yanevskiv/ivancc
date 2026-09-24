// (Test) Return: 42
// The integer type zoo. Each width has its own size and its own range, and a
// value that leaves that range wraps into it rather than keeping the bits it
// had. A narrow type still promotes to int the moment it is used in arithmetic.

short half;
long word;
long long wide;

short takes(short a, long b)
{
    return a + b;
}

long widen(int n)
{
    return n;
}

int main()
{
    short s;
    long l;
    long long ll;

    if (sizeof(short) != 2) return 1;
    if (sizeof(long) != 8) return 2;
    if (sizeof(long long) != 8) return 3;
    if (sizeof(int) != 4) return 4;
    if (sizeof(char) != 1) return 5;

    // The long forms name the same types as the short ones.
    if (sizeof(short int) != 2) return 6;
    if (sizeof(long int) != 8) return 7;
    if (sizeof(long long int) != 8) return 8;

    s = 1000;
    l = 1000000;
    ll = 1000000;
    if (s != 1000) return 9;
    if (l != 1000000) return 10;
    if (ll != 1000000) return 11;

    // A short holds -32768 through 32767, and a store wraps into that range.
    s = 32767;
    if (s != 32767) return 12;
    s = 32768;
    if (s != -32768) return 13;
    s = -32769;
    if (s != 32767) return 14;

    // A long is wide enough for what an int cannot hold.
    l = 4000000000;
    if (l != 4000000000) return 15;
    if (l / 1000 != 4000000) return 16;

    ll = 8000000000;
    if (ll - 4000000000 != 4000000000) return 17;

    // Arithmetic on shorts happens in int, so it does not wrap on the way.
    s = 300;
    if (s * s != 90000) return 18;

    l = 1;
    l = l << 40;
    if (l != 1099511627776) return 19;
    if (l >> 40 != 1) return 20;

    // A negative long shifts arithmetically.
    l = -1024;
    if (l >> 4 != -64) return 21;
    if (l / 16 != -64) return 22;
    if (l % 7 != -2) return 23;

    half = 500;
    word = 5000000000;
    wide = -5000000000;
    if (half != 500) return 24;
    if (word != 5000000000) return 25;
    if (wide != -5000000000) return 26;

    if (takes(100, 200) != 300) return 27;
    if (widen(2000000) * 2000 != 4000000000) return 28;

    short arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 30000;
    arr[3] = -30000;
    if (sizeof(arr) != 8) return 29;
    if (arr[2] + arr[3] != 0) return 30;

    long *p = &l;
    *p = 77;
    if (l != 77) return 31;

    short *q = &s;
    *q = 88;
    if (s != 88) return 32;

    // A short steps by its own size, not by a word.
    short *r = arr;
    r = r + 2;
    if (*r != 30000) return 33;
    if (r - arr != 2) return 34;

    struct Pack {
        char c;
        short s;
        long l;
    };
    if (sizeof(struct Pack) != 16) return 35;

    struct Pack pk;
    pk.c = 1;
    pk.s = 2;
    pk.l = 3;
    if (pk.c + pk.s + pk.l != 6) return 36;

    l = 40;
    s = 2;
    return l + s;
}
