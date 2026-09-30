// (Test) Status: 200
// bug006. A pointer converted to _Bool by initialization, assignment, return or
// argument passing kept its low bits rather than comparing with null.

int x;

_Bool ret(void) { return &x; }

int take(_Bool b) { return b; }

int main()
{
    int *p = &x;
    _Bool b = &x;
    _Bool c;

    c = p;
    if (b != 1) return 1;
    if (c != 1) return 2;
    if (ret() != 1) return 3;
    if (take(p) != 1) return 4;
    return 200;
}
