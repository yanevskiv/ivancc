// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_COMPATIBLE]
// bug024. Assignment converted between any two types without checking them, so
// an integer went into a pointer, and pointers to different types mixed.

int main()
{
    long x = 5;
    long *lp = &x;
    int *p;

    p = lp;
    return *p;
}
