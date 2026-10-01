// (Test) Status: 200
// Operands each operator takes: pointers and functions tested for truth, casts
// of pointers and to void, pointer steps, and integer operators on characters
// and booleans.

struct S { int a; };

int one(void) { return 1; }

int main()
{
    int a[4] = { 1, 2, 3, 4 };
    int *p = a;
    int *q = a + 3;
    int *none = 0;
    struct S s = { 5 };
    char c = 6;
    _Bool b = 0;
    double d = 1.5;
    int n = 0;

    if (! p || none || ! one) return 1;
    while (p && p < q) { p++; n++; }
    if (n != 3 || (none ? 1 : 0)) return 2;
    (void) s;
    (void) one();
    p -= 2;
    p += 1;
    if (*p != 3 || q - p != 1 || (long) (q - a) != 3) return 3;
    if (~c != -7 || c % 4 != 2 || (c << 2) != 24 || (c & 3) != 2) return 4;
    b++;
    if (b != 1 || -d != -1.5 || ! d) return 5;
    if ((char *) a + 4 != (char *) (a + 1) || *(int *) (void *) q != 4) return 6;
    return 200;
}
