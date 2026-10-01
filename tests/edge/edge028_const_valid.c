// (Test) Status: 200
// What `const` allows: initializing const objects, arrays, members and static
// locals, writing through a const pointer to a non-const object, and moving a
// pointer to const.

struct S { const int id; int value; };

int sum(const int n, const int *v, char *const names[])
{
    int total = names[0][0] - 'a';
    for (int i = 0; i < n; i++) total += v[i];
    return total;
}

int main()
{
    int x = 1;
    int y = 2;
    const int c = 3;
    const int list[3] = { 4, 5, 6 };
    int * const fixed = &x;
    const int *moving = &c;
    const int * const * pp = &moving;
    struct S s = { 7, 8 };
    const struct S cs = { 9, 10 };
    char a[] = "a";
    char *const names[1] = { a };
    static const char greeting[] = "hi";
    const int *lit = (const int []) { 11, 12 };

    *fixed = 20;
    moving = &y;
    s.value = 30;
    if (x != 20 || *moving != 2 || **pp != 2) return 1;
    if (c + list[2] != 9 || s.id + s.value != 37) return 2;
    if (cs.id + cs.value != 19 || greeting[1] != 'i' || lit[1] != 12) return 3;
    if (sum(3, list, names) != 15) return 4;
    return 200;
}
