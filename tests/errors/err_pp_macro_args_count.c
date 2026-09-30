// (Test) Compiler error: [ERR_PP_MACRO_ARGS_COUNT]
// Can't call a macro with fewer arguments than parameters.

#define F(a, b) a

int y = F(1);
