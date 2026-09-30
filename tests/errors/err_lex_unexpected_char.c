// (Test) Compiler error: [ERR_LEX_UNEXPECTED_CHAR]
// An `@` outside a literal is no token of C.

int x = 1 @ 2;

int main(void)
{
    return 0;
}
