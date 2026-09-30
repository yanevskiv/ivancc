// (Test) Status: 200
// Pointers, arrays, strings and functions converted to _Bool compare with null,
// on every path that converts: initialization, assignment, return, arguments,
// calls through a pointer, members, bit-fields and conditional operands.

struct S { _Bool m; _Bool bf : 1; int pad; };

int x;
int *np;
int arr[2];

_Bool from_ptr(int *p) { return p; }
_Bool from_arr(void) { return arr; }
_Bool from_str(void) { return "s"; }
_Bool from_fn(void) { return from_ptr; }
int take(_Bool b) { return b; }

int main()
{
    int *p = &x;
    int (*call)(_Bool) = take;
    _Bool b = p;
    _Bool n = np;
    _Bool a = arr;
    _Bool s = "abc";
    _Bool f = main;
    _Bool list[3] = { p, np, &x };
    struct S st = { p, p, 0 };
    _Bool t;

    if (b != 1 || n != 0 || a != 1 || s != 1 || f != 1) return 1;
    if (list[0] != 1 || list[1] != 0 || list[2] != 1) return 2;
    if (st.m != 1 || st.bf != 1) return 3;

    t = p;
    if (t != 1) return 4;
    t = np;
    if (t != 0) return 5;
    t = 1 ? p : np;
    if (t != 1) return 6;
    st.m = np;
    st.bf = p;
    if (st.m != 0 || st.bf != 1) return 7;

    if (from_ptr(p) != 1 || from_ptr(np) != 0) return 8;
    if (from_arr() != 1 || from_str() != 1 || from_fn() != 1) return 9;
    if (take(p) != 1 || take(np) != 0 || take(arr) != 1) return 10;
    if (call(p) != 1 || call(np) != 0) return 11;
    if ((_Bool) p + (_Bool) p != 2) return 12;
    return 200;
}
