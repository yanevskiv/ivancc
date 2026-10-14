// (Test) Compiler error: [ERR_PAR_STATIC_ASSERT_NOT_CONSTANT]
// Can't assert statically what is known only at run time.

void g(int n)
{
    _Static_assert(n == 1, "known only at run time");
}
