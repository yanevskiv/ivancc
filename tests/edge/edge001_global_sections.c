// (Test) Status: 200
// Globals of every storage kind, in every section they land in: initialized
// ones in .data, zero ones in .bss, const ones and string literals read only.
// Every writable one is written and read back, and its neighbours checked.

int data_int = 11;
long data_long = -12;
char data_text[] = "data";
int bss_int;
long bss_long;
char bss_bytes[3];
short bss_shorts[1000];
static int static_bss;
static int static_data = 13;
const int const_int = 14;
const char *const const_text = "rodata";

struct Pair {
    int a;
    long b;
};

struct Pair bss_pair;
struct Pair data_pair = { 1, 2 };

int next(void)
{
    static int zero_start;
    static int one_start = 1;

    zero_start++;
    one_start++;
    return zero_start * 100 + one_start;
}

int check_bss(void)
{
    if (bss_int != 0 || bss_long != 0 || bss_pair.a != 0 || bss_pair.b != 0) return 1;
    for (int i = 0; i < 1000; i++) {
        if (bss_shorts[i] != 0) return 2;
    }

    bss_int = -1;
    bss_long = -2;
    bss_bytes[1] = 'x';
    bss_shorts[0] = 1;
    bss_shorts[999] = 2;
    static_bss = 3;
    bss_pair.a = 4;
    bss_pair.b = 5;

    if (bss_int != -1 || bss_long != -2) return 3;
    if (bss_bytes[0] != 0 || bss_bytes[1] != 'x' || bss_bytes[2] != 0) return 4;
    if (bss_shorts[0] != 1 || bss_shorts[1] != 0 || bss_shorts[998] != 0 || bss_shorts[999] != 2) return 5;
    if (static_bss != 3 || bss_pair.a != 4 || bss_pair.b != 5) return 6;
    return 0;
}

int check_data(void)
{
    if (data_int != 11 || data_long != -12 || static_data != 13) return 11;
    if (data_text[0] != 'd' || data_text[4] != 0) return 12;
    if (data_pair.a != 1 || data_pair.b != 2) return 13;

    data_int = 21;
    data_long = 22;
    data_text[0] = 'D';
    static_data = 23;
    data_pair.b = 24;

    if (data_int != 21 || data_long != 22 || static_data != 23) return 14;
    if (data_text[0] != 'D' || data_text[1] != 'a') return 15;
    if (data_pair.a != 1 || data_pair.b != 24) return 16;
    return 0;
}

int check_read_only(void)
{
    if (const_int != 14) return 21;
    if (const_text[0] != 'r' || const_text[6] != 0) return 22;
    if ("literal"[3] != 'e') return 23;
    return 0;
}

int main()
{
    int r;

    if ((r = check_bss()) != 0) return r;
    if ((r = check_data()) != 0) return r;
    if ((r = check_read_only()) != 0) return r;
    if (next() != 102 || next() != 203) return 31;
    return 200;
}
