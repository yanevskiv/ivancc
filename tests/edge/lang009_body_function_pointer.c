// (Test) Compiler error: [ERR_PAR_BODY_NOT_FUNCTION]
// A pointer to a function is not a function, so its declarator takes no body.

int (*fp)(void) { return 1; }

int main()
{
    return 0;
}
