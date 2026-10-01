// (Test) Compiler error: [ERR_PP_MACRO_PARAM_VA_ARGS]
// Can't name a macro parameter `__VA_ARGS__`.
// Note: gcc only warns.

#define F(__VA_ARGS__) 1
