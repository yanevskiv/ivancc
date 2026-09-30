// (Test) Compiler error: [ERR_PAR_CONFLICTING_TYPES]
// A file-scope object and a function cannot share a name.

int f;
int f(void);

int main()
{
    return 0;
}
