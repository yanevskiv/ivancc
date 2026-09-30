// (Test) Compiler error: [ERR_PAR_TAG_WRONG_KIND]
// A struct tag used again with `union`.

struct S { int a; };
union S u;

int main(void)
{
    return 0;
}
