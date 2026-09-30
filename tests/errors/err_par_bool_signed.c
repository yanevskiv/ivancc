// (Test) Compiler error: [ERR_PAR_BOOL_SIGNED]
// `unsigned` applied to `_Bool`.

unsigned _Bool b;

int main(void)
{
    return 0;
}
