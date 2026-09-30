// (Test) Compiler error: [ERR_SEM_CALL_NOT_FUNCTION]
// A call of an `int`.

int main(void)
{
    int x = 0;

    return x();
}
