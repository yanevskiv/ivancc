// (Test) Compiler error: [ERR_PP_STRINGIZE_NOT_PARAM]
// A `#` in a function-like macro followed by a name that is no parameter.

#define F(a) #b

int main(void)
{
    return 0;
}
