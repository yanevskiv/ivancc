// (Test) Compiler error: [ERR_PAR_VOID_SIGNED]
// `unsigned` applied to `void`.

unsigned void f(void);

int main(void)
{
    return 0;
}
