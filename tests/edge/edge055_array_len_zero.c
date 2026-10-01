// (Test) Compiler error: [ERR_PAR_ARRAY_LEN_NOT_POSITIVE]
// C99 asks a constant array length to be greater than zero, so a zero-length
// array is refused too.

int zero[0];

int main()
{
    return 0;
}
