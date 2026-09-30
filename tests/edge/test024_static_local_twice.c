// (Test) Compiler error: [ERR_PAR_REDECLARED]
// Two statics of one name in one block, with another local looked up after
// them, the input that hung the compiler before bug007.

int main()
{
    int r = 1;
    static int k = 5;
    static int k = 5;

    return r + k;
}
