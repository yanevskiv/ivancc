// (Test) Compiler error: [ERR_SEM_CALL_NOT_FUNCTION]
// Can't call an `int`.

int main(void)
{
    int x = 0;

    return x();
}
