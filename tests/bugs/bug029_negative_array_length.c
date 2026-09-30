// (Test) Compiler error: [ERR_PAR_ARRAY_LEN_NOT_POSITIVE]
// bug029. A negative array length compiled, so the C99 static assertion that
// declares an array of length -1 when its condition fails never fired.

typedef char check_int_size[sizeof(int) == 2 ? 1 : -1];

int main()
{
    return 0;
}
