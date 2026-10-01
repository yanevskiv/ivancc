// (Test) Compiler error: [ERR_SEM_POINTER_OFFSET_NOT_INTEGER]
// Can't add a struct to a pointer.

struct S { int a; } s;

int *f(int *p)
{
    return p + s;
}
