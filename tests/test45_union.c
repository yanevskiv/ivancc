// (Test) Return: 42
// A union is its widest member, and every member starts at offset zero. Writing
// one and reading another is how that overlap is visible.

union Word {
    int  whole;
    char bytes[4];
};

union Mixed {
    char  c;
    int   n;
    char *p;
};

int main()
{
    union Word w;
    union Mixed m;

    if (sizeof(union Word) != 4) return 1;
    if (sizeof(union Mixed) != 8) return 2;

    w.whole = 0;
    w.bytes[0] = 40;
    w.bytes[1] = 0;
    if (w.whole != 40) return 3;

    w.bytes[1] = 1;
    if (w.whole != 296) return 4;

    m.n = 2;
    if (m.c != 2) return 5;

    return w.bytes[0] + m.n;
}
