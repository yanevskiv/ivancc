// (Test) Compiler error: [ERR_PP_MACRO_PARAMS_MALFORMED]
// A parameter list with a missing name after its comma.

#define F(a,) a

int main(void)
{
    return 0;
}
