// (Test) Compiler error: [ERR_SEM_DEREF_INCOMPLETE]
// Can't dereference a pointer to an incomplete type.

struct T;

int main(void)
{
    struct T *p = 0;

    *p;
    return 0;
}
