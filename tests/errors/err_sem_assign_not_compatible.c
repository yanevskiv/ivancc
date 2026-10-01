// (Test) Compiler error: [ERR_SEM_ASSIGN_NOT_COMPATIBLE]
// Can't initialize an `int` with a struct.

struct S { int a; } s;

int main(void)
{
    int x = s;

    return x;
}
