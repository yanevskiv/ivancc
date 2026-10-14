// (Test) Compiler error: [ERR_PAR_DESIG_NOT_CONSTANT]
// Can't designate an index known only at run time.

void g(int n)
{
    int a[3] = { [n] = 1 };
}
