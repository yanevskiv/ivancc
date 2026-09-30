// (Test) Compiler error: [ERR_PAR_CONFLICTING_TYPES]
// A file-scope object declared as `int` and then as `long`.

int x;
long x;

int main(void)
{
    return 0;
}
