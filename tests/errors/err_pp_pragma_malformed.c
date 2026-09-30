// (Test) Compiler error: [ERR_PP_PRAGMA_MALFORMED]
// _Pragma given a number rather than a string literal.

_Pragma(1)

int main(void)
{
    return 0;
}
