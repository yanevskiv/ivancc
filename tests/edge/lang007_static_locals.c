// (Test) Status: 200
// Block-scope statics of one name in sibling and nested blocks, in several
// functions and beside a global of that name, keep one object each, and every
// other name in their scopes stays visible.

int k = 100;

int count(void)
{
    static int k;
    return ++k;
}

int twice(void)
{
    static int k = 10;
    { static int k = 20; k++; }
    { static int k = 30; if (k != 30) return -1; }
    return ++k;
}

int nested(void)
{
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        int a = 1;
        static int k;
        {
            int b = 2;
            static int k = 50;
            static int *p = &k;
            *p += b;
            sum += k;
        }
        k += a;
        sum += k;
    }
    return sum;
}

int main()
{
    count();
    count();
    if (count() != 3) return 1;
    if (twice() != 11 || twice() != 12) return 2;
    if (nested() != 52 + 1 + 54 + 2 + 56 + 3) return 3;
    if (k != 100) return 4;
    {
        static int k = 7;
        {
            extern int k;
            if (k != 100) return 5;
        }
        if (k != 7) return 6;
    }
    return 200;
}
