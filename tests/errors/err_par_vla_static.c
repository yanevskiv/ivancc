// (Test) Compiler error: [ERR_PAR_VLA_STATIC]
// Can't give a `static` local a variable-length array type.

void g(int n)
{
    static int a[n];
}
