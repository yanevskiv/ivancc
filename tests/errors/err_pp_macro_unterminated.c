// (Test) Compiler error: [ERR_PP_MACRO_UNTERMINATED]
// A macro call whose argument list never closes.

#define F(x) x

int y = F(1;

int main(void)
{
    return 0;
}
