// (Test) Compiler error: [ERR_PP_VA_ARGS_MISPLACED]
// Can't use `__VA_ARGS__` in a macro that is not variadic.
// Note: gcc only warns.

#define F(a) a __VA_ARGS__
