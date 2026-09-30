// (Test) Compiler error: [ERR_PP_MACRO_ARGS_COUNT]
// A macro of two parameters called with one argument.

#define F(a, b) a

int y = F(1);

int main(void)
{
    return 0;
}
