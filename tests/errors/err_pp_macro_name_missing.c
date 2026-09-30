// (Test) Compiler error: [ERR_PP_MACRO_NAME_MISSING]
// A #define whose name is a number.

#define 123 4

int main(void)
{
    return 0;
}
