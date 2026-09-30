// (Test) Compiler error: [ERR_PAR_TAG_REDEFINED]
// A struct tag defined twice in one scope.

struct S { int a; };
struct S { int b; };

int main(void)
{
    return 0;
}
