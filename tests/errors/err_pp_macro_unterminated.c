// (Test) Compiler error: [ERR_PP_MACRO_UNTERMINATED]
// Can't leave a macro call's argument list open.

#define F(x) x

int y = F(1;
