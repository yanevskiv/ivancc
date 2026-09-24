// (Test) Return: 42
// Type qualifiers. All three parse wherever a specifier may stand, in any
// order beside the type they qualify, and are recorded on the type they build.
// None of them changes the code generated for a read or a write.

const int limit = 100;
volatile int ticks;
const unsigned long mask = 0xFF;

int sum(const int *a, int n)
{
    int total = 0;
    for (int i = 0; i < n; i++) {
        total = total + a[i];
    }
    return total;
}

int first(const int a[const 4])
{
    return a[0];
}

void bump(volatile int *p)
{
    *p = *p + 1;
}

int copy(int *restrict to, const int *restrict from, int n)
{
    for (int i = 0; i < n; i++) {
        to[i] = from[i];
    }
    return n;
}

int main()
{
    const int c = 5;
    volatile int v = 6;
    const volatile int cv = 7;
    int *restrict r;

    if (c != 5) return 1;
    if (v != 6) return 2;
    if (cv != 7) return 3;
    if (limit != 100) return 4;
    if (mask != 255) return 5;

    // A qualifier may sit on either side of the type it qualifies.
    const long a = 1;
    long const b = 2;
    unsigned const int d = 3;
    const unsigned e = 4;
    if (a + b + d + e != 10) return 6;

    if (sizeof(const int) != 4) return 7;
    if (sizeof(volatile long) != 8) return 8;
    if (sizeof(const unsigned char) != 1) return 9;

    v = 10;
    if (v != 10) return 10;
    v = v + 1;
    if (v != 11) return 11;
    v++;
    if (v != 12) return 12;
    v += 3;
    if (v != 15) return 13;

    ticks = 0;
    bump(&ticks);
    bump(&ticks);
    if (ticks != 2) return 14;

    int arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    if (sum(arr, 4) != 10) return 15;
    if (first(arr) != 1) return 16;

    int dst[4];
    if (copy(dst, arr, 4) != 4) return 17;
    if (dst[0] + dst[3] != 5) return 18;

    r = arr;
    if (*r != 1) return 19;

    const char *msg = "hi";
    if (msg[0] != 'h') return 20;
    if (msg[1] != 'i') return 21;

    // A pointer to a qualified type is still an ordinary pointer.
    const int *p = &c;
    if (*p != 5) return 22;

    volatile int *q = &v;
    *q = 20;
    if (v != 20) return 23;

    struct Held {
        const int n;
        volatile char c;
    };
    if (sizeof(struct Held) != 8) return 24;

    struct Held h;
    if (sizeof(h.n) != 4) return 25;

    typedef const int Fixed;
    Fixed f = 30;
    if (f != 30) return 26;

    return limit - 58;
}
