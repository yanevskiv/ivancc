// (Test) Compiler error: [ERR_PAR_MEMBER_UNNAMED]
// A member whose declarator names nothing.

struct S { int (*)(void); int b; };

int main(void)
{
    return 0;
}
