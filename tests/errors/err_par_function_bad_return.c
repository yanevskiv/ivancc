// (Test) Compiler error: [ERR_PAR_FUNCTION_BAD_RETURN]
// A function returning an array.

int f(void)[3];

int main(void)
{
    return 0;
}
