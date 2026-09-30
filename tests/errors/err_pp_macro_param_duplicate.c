// (Test) Compiler error: [ERR_PP_MACRO_PARAM_DUPLICATE]
// Two parameters of one macro with the same name.

#define F(a, a) a

int main(void)
{
    return 0;
}
