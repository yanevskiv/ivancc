// (Test) Status: 200
// Functions declared in blocks, alone, beside objects in one declaration,
// under `extern`, nested, redeclaring one already defined, and returning
// floating values, are called directly and through pointers.

int twice(int x) { return 2 * x; }

int main()
{
    int twice(int), g(int), k = 4;
    extern int h(void);
    int (*ph)(void) = h;

    {
        double sq(double);
        if (sq(1.5) != 2.25) return 1;
        {
            long wide(long), (*pw)(long) = wide;
            if (pw(3) != 3000000000L) return 2;
        }
    }
    if (twice(k) != 8 || g(k) != 5 || k != 4) return 3;
    if (h() != 99 || ph() != 99) return 4;
    return 200;
}

int g(int x) { return x + 1; }
int h(void) { return 99; }
double sq(double d) { return d * d; }
long wide(long x) { return x * 1000000000L; }
