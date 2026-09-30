// (Test) Compiler error: [ERR_PP_INCLUDE_MALFORMED]
// An #include whose operand is neither "FILE" nor <FILE>.

#include no_quotes.h

int main(void)
{
    return 0;
}
