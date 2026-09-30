// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// In `int * const * pp`, the pointer pp points to is const, and pp itself is
// not, so only `*pp = ...` is refused.

int x;
int * const cp = &x;

int main()
{
    int * const * pp = &cp;
    int * const * other = pp;

    pp = other;
    *pp = &x;
    return 0;
}
