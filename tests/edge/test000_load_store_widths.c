// (Test) Status: 200
// Loads and stores of every integer width, signed and unsigned, through
// globals, locals, pointers, struct members and array elements. Every value
// sits between neighbours with all bits set, so a load or store of the wrong
// width reads or clobbers a neighbour.

struct Mixed {
    signed char c;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
};

unsigned char bytes[3] = { 0xFF, 0x12, 0xFF };
unsigned short halves[3] = { 0xFFFF, 0x1234, 0xFFFF };
unsigned int words[3] = { 0xFFFFFFFF, 0x12345678, 0xFFFFFFFF };
signed char sbytes[3] = { -1, -2, -1 };
short shalves[3] = { -1, -300, -1 };
int swords[3] = { -1, -70000, -1 };

int check_loads(void)
{
    unsigned long u8 = bytes[1];
    unsigned long u16 = halves[1];
    unsigned long u32 = words[1];
    long s8 = sbytes[1];
    long s16 = shalves[1];
    long s32 = swords[1];

    if (u8 != 0x12) return 1;
    if (u16 != 0x1234) return 2;
    if (u32 != 0x12345678) return 3;
    if (s8 != -2) return 4;
    if (s16 != -300) return 5;
    if (s32 != -70000) return 6;
    return 0;
}

int check_stores(void)
{
    bytes[1] = 0x100 + 0x34;
    halves[1] = 0x10000 + 0x5678;
    words[1] = 0x9ABCDEF0;

    if (bytes[0] != 0xFF || bytes[1] != 0x34 || bytes[2] != 0xFF) return 11;
    if (halves[0] != 0xFFFF || halves[1] != 0x5678 || halves[2] != 0xFFFF) return 12;
    if (words[0] != 0xFFFFFFFF || words[1] != 0x9ABCDEF0 || words[2] != 0xFFFFFFFF) return 13;
    return 0;
}

int check_pointers(void)
{
    unsigned int local[3] = { 0xFFFFFFFF, 41, 0xFFFFFFFF };
    unsigned int *p = &local[1];
    unsigned short *h = &halves[1];
    signed char *c = &sbytes[1];

    *p = *p + 1;
    if (*p != 42) return 21;
    if (local[0] != 0xFFFFFFFF || local[2] != 0xFFFFFFFF) return 22;
    if ((unsigned long) *p + 1 != 43) return 23;
    if ((unsigned long) *h != 0x5678) return 24;
    if ((long) *c != -2) return 25;
    return 0;
}

int check_members(void)
{
    struct Mixed m = { -1, 0xFF, -1, 0xFFFF, -1, 0xFFFFFFFF, -1, 0xFFFFFFFFFFFFFFFF };
    struct Mixed *p = &m;

    p->uc = 7;
    p->us = 8;
    p->ui = 9;
    if (m.c != -1 || m.uc != 7 || m.s != -1) return 31;
    if (m.us != 8 || m.i != -1 || m.ui != 9) return 32;
    if (m.l != -1 || m.ul != 0xFFFFFFFFFFFFFFFF) return 33;
    if ((unsigned long) p->ui * 3 != 27) return 34;
    if ((long) p->c + (long) p->s + (long) p->i != -3) return 35;
    return 0;
}

int main()
{
    int r;

    if ((r = check_loads()) != 0) return r;
    if ((r = check_stores()) != 0) return r;
    if ((r = check_pointers()) != 0) return r;
    if ((r = check_members()) != 0) return r;
    return 200;
}
