// (Test) Status: 200
// bug014. A static initializer's address constant had to be a bare symbol, so
// an element, a member, an offset and a function were refused.

struct S { int a; int m[3]; };

int twice(int x) { return 2 * x; }

int a[4] = { 10, 11, 12, 13 };
struct S s = { 1, { 2, 3, 4 } };
char str[] = "hello";

int *elem = &a[2];
int *past = a + 1;
int *member = &s.m[1];
char *tail = "str" + 1;
int (*fn)(int) = twice;
int (*table[2])(int) = { twice, &twice };

int main()
{
    if (*elem != 12 || *past != 11 || *member != 3) return 1;
    if (*tail != 't') return 2;
    if (fn(3) != 6 || table[0](4) != 8 || table[1](5) != 10) return 3;
    return 200;
}
